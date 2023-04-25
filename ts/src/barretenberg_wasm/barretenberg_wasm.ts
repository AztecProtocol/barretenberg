import { readFile } from 'fs/promises';
import isNode from 'detect-node';
import { EventEmitter } from 'events';
import { randomBytes } from 'crypto';
import { fetch } from 'cross-fetch';
import { dirname } from 'path';
import { fileURLToPath } from 'url';
import { AsyncCallState, AsyncFnState } from './async_call_state.js';
import { Crs } from '../crs/index.js';
import { NodeDataStore } from './node/node_data_store.js';
import { WebDataStore } from './browser/web_data_store.js';
import { createHash } from 'crypto';
import { writeFileSync } from 'fs';

EventEmitter.defaultMaxListeners = 30;

const sha256 = (data: Uint8Array) => createHash('sha256').update(data).digest('hex');

export async function fetchCode() {
  if (isNode) {
    const __dirname = dirname(fileURLToPath(import.meta.url));
    return await readFile(__dirname + '/barretenberg.wasm');
  } else {
    const res = await fetch('/barretenberg.wasm');
    return Buffer.from(await res.arrayBuffer());
  }
}

export class BarretenbergWasm extends EventEmitter {
  private store = isNode ? new NodeDataStore() : new WebDataStore();
  private memory!: WebAssembly.Memory;
  private heap!: Uint8Array;
  private instance!: WebAssembly.Instance;
  private asyncCallState = new AsyncCallState();
  public module!: WebAssembly.Module;
  private memStore: { [key: string]: Uint8Array } = {};

  public static async new(initial?: number) {
    const barretenberg = new BarretenbergWasm();
    await barretenberg.init(undefined, initial);
    return barretenberg;
  }

  constructor() {
    super();
  }

  /**
   * 20 pages by default. 20*2**16 > 1mb stack size plus other overheads.
   * 8192 maximum by default. 512mb.
   */
  public async init(module?: WebAssembly.Module, initial = 25, maximum = 2 ** 16) {
    this.debug(
      `initial mem: ${initial} pages, ${(initial * 2 ** 16) / (1024 * 1024)}mb. max mem: ${maximum} pages, ${
        (maximum * 2 ** 16) / (1024 * 1024)
      }mb`,
    );
    this.memory = new WebAssembly.Memory({ initial, maximum });
    // Create a view over the memory buffer.
    // We do this once here, as webkit *seems* bugged out and actually shows this as new memory,
    // thus displaying double. It's only worse if we create views on demand. I haven't established yet if
    // the bug is also exasperating the termination on mobile due to "excessive memory usage". It could be
    // that the OS is actually getting an incorrect reading in the same way the memory profiler does...
    // The view will have to be recreated if the memory is grown. See getMemory().
    this.heap = new Uint8Array(this.memory.buffer);

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
      },

      // These are functions implementations for imports we've defined are needed.
      // The native C++ build defines these in a module called "env". We must implement TypeScript versions here.
      env: {
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
          this.debug(`set_data: ${key} length: ${dataLength} hash: ${sha256(this.memStore[key])}`);
          // writeFileSync('/mnt/user-data/charlie/debugging/ts_' + key, this.memStore[key]);
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
        memory: this.memory,
      },
    };
    /* eslint-enable camelcase */

    if (module) {
      this.instance = await WebAssembly.instantiate(module, importObj);
      this.module = module;
    } else {
      const { instance, module } = await WebAssembly.instantiate(await fetchCode(), importObj);
      this.instance = instance;
      this.module = module;
    }

    // Init all global/static data.
    this.call('_initialize');

    this.asyncCallState.init(this.memory, this.call.bind(this), this.debug.bind(this));
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
    for (let i = 0; i < arr.length; i++) {
      mem[i + offset] = arr[i];
    }
  }

  // PRIVATE METHODS

  private getMemory() {
    // If the memory is grown, our view over it will be lost. Recreate the view.
    if (this.heap.length === 0) {
      this.heap = new Uint8Array(this.memory.buffer);
    }
    return this.heap;
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
