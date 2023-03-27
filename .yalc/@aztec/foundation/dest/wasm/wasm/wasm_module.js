import { createDebugLogger } from '@aztec/foundation';
import { Buffer } from 'buffer';
import { MemoryFifo } from '../memory_fifo.js';
import { getEmptyWasiSdk } from './empty_wasi_sdk.js';
import { randomBytes } from 'crypto';
/**
 * WasmModule:
 *  Helper over a webassembly module.
 *  Assumes a few quirks.
 *  1) the module expects wasi_snapshot_preview1 with the methods from getEmptyWasiSdk
 *  2) of which the webassembly
 *  we instantiate only uses random_get (update this if more WASI sdk methods are needed).
 */
export class WasmModule {
    /**
     * Create a wasm module. Should be followed by await init();.
     * @param module - The module as a WebAssembly.Module or a Buffer.
     * @param importFn - Imports expected by the WASM.
     * @param loggerName - Optional, for debug logging.
     */
    constructor(module, importFn, loggerName = 'wasm') {
        this.module = module;
        this.importFn = importFn;
        this.mutexQ = new MemoryFifo();
        this.debug = createDebugLogger(loggerName);
        this.mutexQ.put(true);
    }
    /**
     * Return the wasm source.
     * @returns The source.
     */
    getModule() {
        return this.module;
    }
    /**
     * Initialize this wasm module.
     * @param wasmImportEnv - Linked to a module called "env". Functions implementations referenced from e.g. C++.
     * @param initial - 20 pages by default. 20*2**16 \> 1mb stack size plus other overheads.
     * @param maximum - 8192 maximum by default. 512mb.
     */
    async init(initial = 20, maximum = 8192) {
        this.debug(`initial mem: ${initial} pages, ${(initial * 2 ** 16) / (1024 * 1024)}mb. max mem: ${maximum} pages, ${(maximum * 2 ** 16) / (1024 * 1024)}mb`);
        this.memory = new WebAssembly.Memory({ initial, maximum });
        // Create a view over the memory buffer.
        // We do this once here, as webkit *seems* bugged out and actually shows this as new memory,
        // thus displaying double. It's only worse if we create views on demand. I haven't established yet if
        // the bug is also exasperating the termination on mobile due to "excessive memory usage". It could be
        // that the OS is actually getting an incorrect reading in the same way the memory profiler does...
        // The view will have to be recreated if the memory is grown. See getMemory().
        this.heap = new Uint8Array(this.memory.buffer);
        // We support the wasi 12 SDK, but only implement random_get
        /* eslint-disable camelcase */
        const importObj = {
            wasi_snapshot_preview1: {
                ...getEmptyWasiSdk(this.debug),
                random_get: (arr, length) => {
                    arr = arr >>> 0;
                    const heap = this.getMemory();
                    const randomData = randomBytes(length);
                    for (let i = arr; i < arr + length; ++i) {
                        heap[i] = randomData[i - arr];
                    }
                },
            },
            env: this.importFn(this),
        };
        if (this.module instanceof WebAssembly.Module) {
            this.instance = await WebAssembly.instantiate(this.module, importObj);
        }
        else {
            const { instance } = await WebAssembly.instantiate(this.module, importObj);
            this.instance = instance;
        }
    }
    /**
     * The methods or objects exported by the WASM module.
     * @returns An indexable object.
     */
    exports() {
        if (!this.instance) {
            throw new Error('WasmModule: not initialized!');
        }
        return this.instance.exports;
    }
    /**
     * Get the current logger.
     * @returns Logging function.
     */
    getLogger() {
        return this.debug;
    }
    /**
     * Add a logger.
     * @param logger - Function to call when logging.
     */
    addLogger(logger) {
        const oldDebug = this.debug;
        this.debug = (...args) => {
            logger(...args);
            oldDebug(...args);
        };
    }
    /**
     * Calls into the WebAssembly.
     * @param name - The method name.
     * @param args - The arguments to the method.
     * @returns The numeric method result.
     */
    call(name, ...args) {
        if (!this.exports()[name]) {
            throw new Error(`WASM function ${name} not found.`);
        }
        try {
            // When returning values from the WASM, use >>> operator to convert
            // signed representation to unsigned representation.
            return this.exports()[name](...args) >>> 0;
        }
        catch (err) {
            const message = `WASM function ${name} aborted, error: ${err}`;
            this.debug(message);
            this.debug(err.stack);
            throw new Error(message);
        }
    }
    /**
     * Get the memory used by the WASM module.
     * @returns A WebAssembly memory object.
     */
    getRawMemory() {
        return this.memory;
    }
    /**
     * Get the memory used by the WASM module, as a byte array.
     * @returns A Uint8Array view of the WASM module memory.
     */
    getMemory() {
        // If the memory is grown, our view over it will be lost. Recreate the view.
        if (this.heap.length === 0) {
            this.heap = new Uint8Array(this.memory.buffer);
        }
        return this.heap;
    }
    /**
     * The memory size in bytes.
     * @returns Number of bytes.
     */
    memSize() {
        return this.getMemory().length;
    }
    /**
     * Get a slice of memory between two addresses.
     * @param start - The start address.
     * @param end - The end address.
     * @returns A Uint8Array view of memory.
     */
    getMemorySlice(start, end) {
        return this.getMemory().slice(start, end);
    }
    /**
     * Write data into the heap.
     * @param offset - The address to write data at.
     * @param arr - The data to write.
     */
    writeMemory(offset, arr) {
        const mem = this.getMemory();
        for (let i = 0; i < arr.length; i++) {
            mem[i + offset] = arr[i];
        }
    }
    /**
     * Read WASM memory as a JS string.
     * @param addr - The memory address.
     * @returns A JS string.
     */
    getMemoryAsString(addr) {
        addr = addr >>> 0;
        const m = this.getMemory();
        let i = addr;
        for (; m[i] !== 0; ++i)
            ;
        return Buffer.from(m.slice(addr, i)).toString('ascii');
    }
    /**
     * When calling the wasm, sometimes a caller will require exclusive access over a series of calls.
     * E.g. When a result is written to address 0, one cannot have another caller writing to the same address via
     * writeMemory before the result is read via sliceMemory.
     * Acquire() gets a single token from a fifo. The caller must call release() to add the token back.
     */
    async acquire() {
        await this.mutexQ.get();
    }
    /**
     * Release the mutex, letting another promise call acquire().
     */
    release() {
        if (this.mutexQ.length() !== 0) {
            throw new Error('Release called but not acquired.');
        }
        this.mutexQ.put(true);
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoid2FzbV9tb2R1bGUuanMiLCJzb3VyY2VSb290IjoiIiwic291cmNlcyI6WyIuLi8uLi8uLi9zcmMvd2FzbS93YXNtL3dhc21fbW9kdWxlLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sRUFBRSxpQkFBaUIsRUFBZSxNQUFNLG1CQUFtQixDQUFDO0FBQ25FLE9BQU8sRUFBRSxNQUFNLEVBQUUsTUFBTSxRQUFRLENBQUM7QUFDaEMsT0FBTyxFQUFFLFVBQVUsRUFBRSxNQUFNLG1CQUFtQixDQUFDO0FBQy9DLE9BQU8sRUFBRSxlQUFlLEVBQUUsTUFBTSxxQkFBcUIsQ0FBQztBQUN0RCxPQUFPLEVBQUUsV0FBVyxFQUFFLE1BQU0sUUFBUSxDQUFDO0FBRXJDOzs7Ozs7O0dBT0c7QUFDSCxNQUFNLE9BQU8sVUFBVTtJQU9yQjs7Ozs7T0FLRztJQUNILFlBQ1UsTUFBbUMsRUFDbkMsUUFBcUMsRUFDN0MsVUFBVSxHQUFHLE1BQU07UUFGWCxXQUFNLEdBQU4sTUFBTSxDQUE2QjtRQUNuQyxhQUFRLEdBQVIsUUFBUSxDQUE2QjtRQVh2QyxXQUFNLEdBQUcsSUFBSSxVQUFVLEVBQVcsQ0FBQztRQWN6QyxJQUFJLENBQUMsS0FBSyxHQUFHLGlCQUFpQixDQUFDLFVBQVUsQ0FBQyxDQUFDO1FBQzNDLElBQUksQ0FBQyxNQUFNLENBQUMsR0FBRyxDQUFDLElBQUksQ0FBQyxDQUFDO0lBQ3hCLENBQUM7SUFFRDs7O09BR0c7SUFDSSxTQUFTO1FBQ2QsT0FBTyxJQUFJLENBQUMsTUFBTSxDQUFDO0lBQ3JCLENBQUM7SUFDRDs7Ozs7T0FLRztJQUNJLEtBQUssQ0FBQyxJQUFJLENBQUMsT0FBTyxHQUFHLEVBQUUsRUFBRSxPQUFPLEdBQUcsSUFBSTtRQUM1QyxJQUFJLENBQUMsS0FBSyxDQUNSLGdCQUFnQixPQUFPLFdBQVcsQ0FBQyxPQUFPLEdBQUcsQ0FBQyxJQUFJLEVBQUUsQ0FBQyxHQUFHLENBQUMsSUFBSSxHQUFHLElBQUksQ0FBQyxnQkFBZ0IsT0FB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