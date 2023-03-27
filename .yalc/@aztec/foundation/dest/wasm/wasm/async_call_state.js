/**
 * To enable asynchronous callbacks from wasm to js, we leverage asyncify.
 * Https://kripken.github.io/blog/wasm/2019/07/16/asyncify.html.
 *
 * This class holds state and logic specific to handling async calls from wasm to js.
 * A single instance of this class is instantiated as part of BarretenbergWasm.
 * It allocates some memory for the asyncify stack data and initialises it.
 *
 * To make an async call into the wasm, just call `call` the same as in BarretenbergWasm, only it returns a promise.
 *
 * To make an async import that will be called from the wasm, wrap a function with the signature:
 *   my_func(state: AsyncFnState, ...args)
 * with a call to `wrapImportFn`.
 * The arguments are whatever the original call arguments were. The addition of AsyncFnState as the first argument
 * allows for the detection of wether the function is continuing after the the async call has completed.
 * If `state.continuation` is false, the function should start its async operation and return the promise.
 * If `state.continuation` is true, the function can get the result from `state.result` perform any finalisation,
 * and return an (optional) value to the wasm.
 */
export class AsyncCallState {
    constructor() {
        this.ASYNCIFY_DATA_SIZE = 16 * 1024;
    }
    /**
     * Initialize the call hooks with a WasmModule.
     * @param wasm - The module.
     */
    init(wasm) {
        this.wasm = wasm;
        this.callExport = (name, ...args) => wasm.call(name, ...args);
        // Allocate memory for asyncify stack data.
        this.asyncifyDataAddr = this.callExport('bbmalloc', this.ASYNCIFY_DATA_SIZE);
        // TODO: is this view construction problematic like in WasmModule?
        const view = new Uint32Array(wasm.getRawMemory().buffer);
        // First two integers of asyncify data, are the start and end of the stack region.
        view[this.asyncifyDataAddr >> 2] = this.asyncifyDataAddr + 8;
        view[(this.asyncifyDataAddr + 4) >> 2] = this.asyncifyDataAddr + this.ASYNCIFY_DATA_SIZE;
    }
    /**
     * Log a message.
     * @param args - The message arguments.
     */
    debug(...args) {
        this.wasm.getLogger()(...args);
    }
    /**
     * Free the data associated with async call states.
     */
    destroy() {
        // Free call stack data.
        this.callExport('bbfree', this.asyncifyDataAddr);
    }
    /**
     * We call the wasm function, that will in turn call back into js via callImport and set this.asyncPromise and
     * enable the instrumented "record stack unwinding" code path.
     * Once the stack has unwound out of the wasm call, we enter into a loop of resolving the promise set in the call
     * to callImport, and calling back into the wasm to rewind the stack and continue execution.
     * @param name - The function name.
     * @param args - The function args.
     * @returns The function result.
     */
    async call(name, ...args) {
        if (this.state) {
            throw new Error(`Can only handle one async call at a time: ${name}(${args})`);
        }
        this.state = { continuation: false };
        let result = this.callExport(name, ...args);
        while (this.asyncPromise) {
            // Disable the instrumented "record stack unwinding" code path.
            this.callExport('asyncify_stop_unwind');
            this.debug('stack unwound.');
            // Wait for the async work to complete.
            this.state.result = await this.asyncPromise;
            this.state.continuation = true;
            this.debug('result set starting rewind.');
            // Enable "stack rewinding" code path.
            this.callExport('asyncify_start_rewind', this.asyncifyDataAddr);
            // Call function again to rebuild the stack, and continue where we left off.
            result = this.callExport(name, ...args);
        }
        // Cleanup
        this.state = undefined;
        return result;
    }
    /**
     * Wrap a WASM import function.
     * @param fn - The function.
     * @returns A wrapped version with asyncify calls.
     */
    wrapImportFn(fn) {
        return (...args) => {
            if (!this.asyncPromise) {
                // We are in the normal code path. Start the async fetch of data.
                this.asyncPromise = fn(this.state, ...args);
                // Enable "record stack unwinding" code path and return.
                this.callExport('asyncify_start_unwind', this.asyncifyDataAddr);
            }
            else {
                // We are in the stack rewind code path, called once the promise is resolved.
                // Save the result data back to the wasm, disable stack rewind code paths, and return.
                this.callExport('asyncify_stop_rewind');
                const result = fn(this.state, ...args);
                // Cleanup.
                this.asyncPromise = undefined;
                this.state = { continuation: false };
                return result;
            }
        };
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiYXN5bmNfY2FsbF9zdGF0ZS5qcyIsInNvdXJjZVJvb3QiOiIiLCJzb3VyY2VzIjpbIi4uLy4uLy4uL3NyYy93YXNtL3dhc20vYXN5bmNfY2FsbF9zdGF0ZS50cyJdLCJuYW1lcyI6W10sIm1hcHBpbmdzIjoiQUFnQkE7Ozs7Ozs7Ozs7Ozs7Ozs7OztHQWtCRztBQUNILE1BQU0sT0FBTyxjQUFjO0lBQTNCO1FBQ1UsdUJBQWtCLEdBQUcsRUFBRSxHQUFHLElBQUksQ0FBQztJQW1HekMsQ0FBQztJQTVGQzs7O09BR0c7SUFDSSxJQUFJLENBQUMsSUFBZ0I7UUFDMUIsSUFBSSxDQUFDLElBQUksR0FBRyxJQUFJLENBQUM7UUFDakIsSUFBSSxDQUFDLFVBQVUsR0FBRyxDQUFDLElBQVksRUFBRSxHQUFHLElBQVcsRUFBRSxFQUFFLENBQUMsSUFBSSxDQUFDLElBQUksQ0FBQyxJQUFJLEVBQUUsR0FBRyxJQUFJLENBQUMsQ0FBQztRQUM3RSwyQ0FBMkM7UUFDM0MsSUFBSSxDQUFDLGdCQUFnQixHQUFHLElBQUksQ0FBQyxVQUFVLENBQUMsVUFBVSxFQUFFLElBQUksQ0FBQyxrQkFBa0IsQ0FBQyxDQUFDO1FBQzdFLGtFQUFrRTtRQUNsRSxNQUFNLElBQUksR0FBRyxJQUFJLFdBQVcsQ0FBQyxJQUFJLENBQUMsWUFBWSxFQUFFLENBQUMsTUFBTSxDQUFDLENBQUM7UUFDekQsa0ZBQWtGO1FBQ2xGLElBQUksQ0FBQyxJQUFJLENBQUMsZ0JBQWdCLElBQUksQ0FBQyxDQUFDLEdBQUcsSUFBSSxDQUFDLGdCQUFnQixHQUFHLENBQUMsQ0FBQztRQUM3RCxJQUFJLENBQUMsQ0FBQyxJQUFJLENBQUMsZ0JBQWdCLEdBQUcsQ0FBQyxDQUFDLElBQUksQ0FBQyxDQUFDLEdBQUcsSUFBSSxDQUFDLGdCQUFnQixHQUFHLElBQUksQ0FBQyxrQkFBa0IsQ0FBQztJQUMzRixDQUFDO0lBRUQ7OztPQUdHO0lBQ0ssS0FBSyxDQUFDLEdBQUcsSUFBVztRQUMxQixJQUFJLENBQUMsSUFBSSxD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