import { readFile } from 'fs/promises';
import isNode from 'detect-node';
import { EventEmitter } from 'events';
import { randomBytes } from 'crypto';
import { fetch } from 'cross-fetch';
import { dirname } from 'path';
import { fileURLToPath } from 'url';
import { AsyncCallState, AsyncFnState } from './async_call_state.js';
import { Crs } from '../crs/index.js';
// import { NodeDataStore } from './node/node_data_store.js';
// import { WebDataStore } from './browser/web_data_store.js';
import { createHash } from 'crypto';
import { Worker } from 'worker_threads';
import os from 'os';

EventEmitter.defaultMaxListeners = 30;

const sha256 = (data: Uint8Array) => createHash('sha256').update(data).digest('hex');

function getNumCpu() {
  return Math.min(isNode ? os.cpus().length : navigator.hardwareConcurrency, 8);
}

export async function fetchCode() {
  if (isNode) {
    const __dirname = dirname(fileURLToPath(import.meta.url));
    return await readFile(__dirname + '/barretenberg.wasm');
    // return await readFile('/mnt/user-data/charlie/min_wasm/maincpp.wasm');
  } else {
    const res = await fetch('/barretenberg.wasm');
    return Buffer.from(await res.arrayBuffer());
  }
}

export class BarretenbergWasm extends EventEmitter {
  // private store = isNode ? new NodeDataStore() : new WebDataStore();
  private memory!: WebAssembly.Memory;
  private instance!: WebAssembly.Instance;
  private asyncCallState = new AsyncCallState();
  private memStore: { [key: string]: Uint8Array } = {};
  private workers: Worker[] = [];
  private nextWorker = 0;
  private nextThreadId = 1;

  public static async new(threads = getNumCpu(), logHandler?: (m: string) => void, initial?: number) {
    const barretenberg = new BarretenbergWasm();
    if (logHandler) {
      barretenberg.on('log', logHandler);
    }
    await barretenberg.init(threads, initial);
    return barretenberg;
  }

  constructor() {
    super();
  }

  public getNumWorkers() {
    return this.workers.length;
  }

  /**
   * Init as main thread. Spawn child threads.
   */
  public async init(threads: number, initial = 25, maximum = 2 ** 16) {
    const initialMb = (initial * 2 ** 16) / (1024 * 1024);
    const maxMb = (maximum * 2 ** 16) / (1024 * 1024);
    this.debug(`main thread initial mem: ${initial} pages, ${initialMb}mb. max mem: ${maximum} pages, ${maxMb}mb`);

    this.memory = new WebAssembly.Memory({ initial, maximum, shared: true });

    const { instance, module } = await WebAssembly.instantiate(await fetchCode(), this.getImportObj(this.memory));

    this.instance = instance;

    // Init all global/static data.
    this.call('_initialize');

    // this.asyncCallState.init(this.memory, this.call.bind(this), this.debug.bind(this));

    // Create worker threads. Returns once all workers have signalled they're ready.
    this.workers = await Promise.all(
      Array.from({ length: threads }).map((_, i) => {
        return new Promise<Worker>(resolve => {
          this.debug(`creating worker ${i}`);
          const __dirname = dirname(fileURLToPath(import.meta.url));
          const worker = new Worker(
            __dirname + `/node/barretenberg_thread.ts`,
            i === 0 ? { execArgv: ['--inspect-brk=0.0.0.0'] } : {},
          );
          worker.on('message', msg => {
            if (typeof msg === 'number') {
              // Worker is ready.
              this.debug(`worker ready ${i}`);
              resolve(worker);
              return;
            }
            this.debug(msg);
          });
          // worker.on('exit', () => this.debug(`worker ${i} exited!`));
          worker.postMessage({ msg: 'start', data: { module, memory: this.memory } });
        });
      }),
    );
  }

  /**
   * Init as spawned thread.
   */
  public async initThread(module: WebAssembly.Module, memory: WebAssembly.Memory) {
    this.memory = memory;
    this.instance = await WebAssembly.instantiate(module, this.getImportObj(this.memory));

    // this.asyncCallState.init(this.memory, this.call.bind(this), this.debug.bind(this));
  }

  /**
   * Called on main thread. Signals child threads to gracefully exit.
   */
  public async destroy() {
    this.workers.forEach(t => t.postMessage({ msg: 'exit' }));
    await Promise.all(this.workers.map(worker => new Promise(resolve => worker.once('exit', resolve))));
  }

  private getImportObj(memory: WebAssembly.Memory) {
    /* eslint-disable camelcase */
    const importObj = {
      // We need to implement a part of the wasi api:
      // https://github.com/WebAssembly/WASI/blob/main/phases/snapshot/docs.md
      // We literally only need to support random_get, everything else is noop implementated in barretenberg.wasm.
      wasi_snapshot_preview1: {
        random_get: (arr: any, length: number) => {
          arr = arr >>> 0;
          const heap = this.getMemory();
          const randomData = randomBytes(length);
          for (let i = arr; i < arr + length; ++i) {
            heap[i] = randomData[i - arr];
          }
        },
        clock_time_get: (a1: number, a2: number, out: number) => {
          out = out >>> 0;
          const ts = BigInt(new Date().getTime()) * 1000000n;
          const heap = this.getMemory();
          const buf = Buffer.alloc(8);
          buf.writeBigUInt64LE(ts);
          buf.copy(heap, out);
        },
        // sched_yield: () => {
        // this.debug('sched_yield');
        // process.nextTick(() => {});
        // return 0;
        // },
      },
      wasi: {
        'thread-spawn': (arg: number) => {
          const id = this.nextThreadId++;
          const worker = this.nextWorker++ % this.workers.length;
          // this.debug(`spawning thread ${id} on worker ${worker} with arg ${arg >>> 0}`);
          this.workers[worker].postMessage({ msg: 'thread', data: { id, arg } });
          return id;
        },
      },

      // These are functions implementations for imports we've defined are needed.
      // The native C++ build defines these in a module called "env". We must implement TypeScript versions here.
      env: {
        env_hardware_concurrency: () => {
          // If there are no workers (we're already running as a worker, or the main thread requested no workers)
          // then we return 1, which should cause any algos using threading to just not create a thread.
          return this.workers.length || 1;
        },
        /**
         * The 'info' call we use for logging in C++, calls this under the hood.
         * The native code will just print to std:err (to avoid std::cout which is used for IPC).
         * Here we just emit the log line for the client to decide what to do with.
         */
        logstr: (addr: number) => {
          const str = this.stringFromAddress(addr);
          const m = this.getMemory();
          const str2 = `${str} (mem: ${(m.length / (1024 * 1024)).toFixed(2)}MB)`;
          this.debug(str2);
        },

        get_data: (keyAddr: number, outBufAddr: number) => {
          const key = this.stringFromAddress(keyAddr);
          outBufAddr = outBufAddr >>> 0;
          const data = this.memStore[key];
          if (!data) {
            this.debug(`get_data miss ${key}`);
            return;
          }
          // this.debug(`get_data hit ${key} ${data.length}`);
          this.writeMemory(outBufAddr, data);
        },

        set_data: (keyAddr: number, dataAddr: number, dataLength: number) => {
          const key = this.stringFromAddress(keyAddr);
          dataAddr = dataAddr >>> 0;
          this.memStore[key] = this.getMemorySlice(dataAddr, dataAddr + dataLength);
          // this.debug(`set_data: ${key} length: ${dataLength} hash: ${sha256(this.memStore[key])}`);
        },

        my_async_func: (acAddr: number) => {
          this.debug('my_async_func entry');
          setTimeout(() => {
            this.debug('my_async_func notifying');
            this.call('async_complete', acAddr);
          }, 2000);
        },

        /**
         * Read the data associated with the key located at keyAddr.
         * Malloc data within the WASM, copy the data into the WASM, and return the address to the caller.
         * The caller is responsible for taking ownership of (and freeing) the memory at the returned address.
         */
        // get_data: this.wrapAsyncImportFn(async (keyAddr: number, lengthOutAddr: number) => {
        //   const key = this.stringFromAddress(keyAddr);
        //   const data = await this.store.get(key);
        //   if (!data) {
        //     this.writeMemory(lengthOutAddr, numToUInt32LE(0));
        //     return 0;
        //   }
        //   const dataAddr = this.call('bbmalloc', data.length);
        //   this.writeMemory(lengthOutAddr, numToUInt32LE(data.length));
        //   this.writeMemory(dataAddr, data);
        //   return dataAddr;
        // }),
        // set_data: this.wrapAsyncImportFn(async (keyAddr: number, dataAddr: number, dataLength: number) => {
        //   const key = this.stringFromAddress(keyAddr);
        //   await this.store.set(key, Buffer.from(this.getMemorySlice(dataAddr, dataAddr + dataLength)));
        // }),
        env_load_verifier_crs: this.wrapAsyncImportFn(async () => {
          // TODO optimize
          const crs = new Crs(0);
          await crs.init();
          const crsPtr = this.call('bbmalloc', crs.getG2Data().length);
          this.writeMemory(crsPtr, crs.getG2Data());
          return crsPtr;
        }),
        env_load_prover_crs: this.wrapAsyncImportFn(async (numPoints: number) => {
          const crs = new Crs(numPoints);
          await crs.init();
          const crsPtr = this.call('bbmalloc', crs.getG1Data().length);
          this.writeMemory(crsPtr, crs.getG1Data());
          return crsPtr;
        }),
        memory,
      },
    };
    /* eslint-enable camelcase */

    return importObj;
  }

  public exports(): any {
    return this.instance.exports;
  }

  /**
   * When returning values from the WASM, use >>> operator to convert signed representation to unsigned representation.
   */
  public call(name: string, ...args: any) {
    if (!this.exports()[name]) {
      throw new Error(`WASM function ${name} not found.`);
    }
    try {
      return this.exports()[name](...args) >>> 0;
    } catch (err: any) {
      const message = `WASM function ${name} aborted, error: ${err}`;
      this.debug(message);
      this.debug(err.stack);
      throw new Error(message);
    }
  }

  /**
   * Uses asyncify to enable async callbacks into js.
   * https://kripken.github.io/blog/wasm/2019/07/16/asyncify.html
   */
  public async asyncCall(name: string, ...args: any) {
    if (this.asyncCallState.state) {
      throw new Error(`Can only handle one async call at a time: ${name}(${args})`);
    }
    return await this.asyncCallState.call(name, ...args);
  }

  public memSize() {
    return this.getMemory().length;
  }

  public getMemorySlice(start: number, end?: number) {
    return this.getMemory().slice(start, end);
  }

  public writeMemory(offset: number, arr: Uint8Array) {
    const mem = this.getMemory();
    mem.set(arr, offset);
  }

  // PRIVATE METHODS

  private getMemory() {
    return new Uint8Array(this.memory.buffer);
  }

  private stringFromAddress(addr: number) {
    addr = addr >>> 0;
    const m = this.getMemory();
    let i = addr;
    for (; m[i] !== 0; ++i);
    return Buffer.from(m.slice(addr, i)).toString('ascii');
  }

  private wrapAsyncImportFn(fn: (...args: number[]) => Promise<number | void>) {
    // TODO upstream this utility to asyncCallState?
    return this.asyncCallState.wrapImportFn((state: AsyncFnState, ...args: number[]) => {
      if (!state.continuation) {
        return fn(...args);
      }
      return state.result;
    });
  }

  private debug(str: string) {
    this.emit('log', str);
  }
}
