import { type Worker } from 'worker_threads';
import { EventEmitter } from 'events';
import createDebug from 'debug';
import { Remote } from 'comlink';
// Webpack config swaps this import with ./browser/index.js
// You can toggle between these two imports to sanity check the type-safety.
import { fetchCode, getNumCpu, createWorker, randomBytes, getRemoteBarretenbergWasm } from './node/index.js';
// import { fetchCode, getNumCpu, createWorker, randomBytes } from './browser/index.js';

EventEmitter.defaultMaxListeners = 30;

export class BarretenbergWasm extends EventEmitter {
  private memory!: WebAssembly.Memory;
  private instance!: WebAssembly.Instance;
  private workers: Worker[] = [];
  private remoteWasms: RemoteBarretenbergWasm[] = [];
  private nextWorker = 0;
  private nextThreadId = 1;

  public static async new(
    threads = Math.min(getNumCpu(), 8),
    logHandler: (msg: string) => void = createDebug('wasm'),
    initial?: number,
  ) {
    const barretenberg = new BarretenbergWasm();
    barretenberg.on('log', logHandler);
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
  public async init(threads = Math.min(getNumCpu(), 8), initial = 25, maximum = 2 ** 16) {
    const initialMb = (initial * 2 ** 16) / (1024 * 1024);
    const maxMb = (maximum * 2 ** 16) / (1024 * 1024);
    this.debug(
      `initial mem: ${initial} pages, ${initialMb}MiB. ` +
        `max mem: ${maximum} pages, ${maxMb}MiB. ` +
        `threads: ${threads}`,
    );

    this.memory = new WebAssembly.Memory({ initial, maximum, shared: true });

    const { instance, module } = await WebAssembly.instantiate(await fetchCode(), this.getImportObj(this.memory));

    this.instance = instance;

    // Init all global/static data.
    this.call('_initialize');

    // Create worker threads. Create 1 less than requested, as main thread counts as a thread.
    this.debug('creating worker threads...');
    this.workers = await Promise.all(Array.from({ length: threads - 1 }).map(createWorker));
    this.remoteWasms = await Promise.all(this.workers.map(getRemoteBarretenbergWasm));
    await Promise.all(this.remoteWasms.map(w => w.initThread(module, this.memory)));
    this.debug('init complete.');
  }

  /**
   * Init as spawned thread.
   */
  public async initThread(module: WebAssembly.Module, memory: WebAssembly.Memory) {
    this.memory = memory;
    this.instance = await WebAssembly.instantiate(module, this.getImportObj(this.memory));
  }

  /**
   * Called on main thread. Signals child threads to gracefully exit.
   */
  public destroy() {
    this.workers.forEach(w => w.terminate());
    return Promise.resolve();
  }

  private getImportObj(memory: WebAssembly.Memory) {
    /* eslint-disable camelcase */
    const importObj = {
      // We need to implement a part of the wasi api:
      // https://github.com/WebAssembly/WASI/blob/main/phases/snapshot/docs.md
      // We literally only need to support random_get, everything else is noop implementated in barretenberg.wasm.
      wasi_snapshot_preview1: {
        random_get: (out: any, length: number) => {
          out = out >>> 0;
          const randomData = randomBytes(length);
          const mem = this.getMemory();
          mem.set(randomData, out);
        },
        clock_time_get: (a1: number, a2: number, out: number) => {
          out = out >>> 0;
          const ts = BigInt(new Date().getTime()) * 1000000n;
          const view = new DataView(this.getMemory().buffer);
          view.setBigUint64(out, ts, true);
        },
      },
      wasi: {
        'thread-spawn': (arg: number) => {
          arg = arg >>> 0;
          const id = this.nextThreadId++;
          const worker = this.nextWorker++ % this.remoteWasms.length;
          // this.debug(`spawning thread ${id} on worker ${worker} with arg ${arg >>> 0}`);
          void this.remoteWasms[worker].call('wasi_thread_start', id, arg);
          // this.remoteWasms[worker].postMessage({ msg: 'thread', data: { id, arg } });
          return id;
        },
      },

      // These are functions implementations for imports we've defined are needed.
      // The native C++ build defines these in a module called "env". We must implement TypeScript versions here.
      env: {
        env_hardware_concurrency: () => {
          // If there are no workers (we're already running as a worker, or the main thread requested no workers)
          // then we return 1, which should cause any algos using threading to just not create a thread.
          return this.remoteWasms.length + 1;
        },
        /**
         * The 'info' call we use for logging in C++, calls this under the hood.
         * The native code will just print to std:err (to avoid std::cout which is used for IPC).
         * Here we just emit the log line for the client to decide what to do with.
         */
        logstr: (addr: number) => {
          const str = this.stringFromAddress(addr);
          const m = this.getMemory();
          const str2 = `${str} (mem: ${(m.length / (1024 * 1024)).toFixed(2)}MiB)`;
          this.debug(str2);
        },

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

  public memSize() {
    return this.getMemory().length;
  }

  public getMemorySlice(start: number, end?: number) {
    return this.getMemory().subarray(start, end);
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
    const textDecoder = new TextDecoder('ascii');
    return textDecoder.decode(m.slice(addr, i));
  }

  private debug(str: string) {
    console.log(str);
    this.emit('log', str);
  }
}

export type RemoteBarretenbergWasm = Remote<BarretenbergWasm>;
