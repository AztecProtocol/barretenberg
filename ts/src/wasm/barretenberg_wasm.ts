import { WasmModule } from '@aztec/foundation/wasm';

import isNode from 'detect-node';
import { readFile } from 'fs/promises';
import { dirname } from 'path';
import { fileURLToPath } from 'url';

import { Crs } from '../crs/index.js';

const INITIAL_CRS_POINTS = 16384;
/**
 * Get the WASM binary for barretenberg.
 * @returns The binary buffer.
 */
export async function fetchCode() {
  if (isNode) {
    const __dirname = dirname(fileURLToPath(import.meta.url));
    return await readFile(__dirname + '/barretenberg.wasm');
  } else {
    const res = await fetch('/barretenberg.wasm');
    return Buffer.from(await res.arrayBuffer());
  }
}

/**
 * A low-level wrapper for an instance of Barretenberg WASM.
 */
export class BarretenbergWasm {
  private wasm!: WasmModule;

  /**
   * Create and initialize a BarretenbergWasm module.
   * @param initial - Initial memory pages.
   * @returns The module.
   */
  public static async new(initial?: number, initialCrsPoints: number = INITIAL_CRS_POINTS) {
    const crs = new Crs(initialCrsPoints);
    await crs.init();
    const barretenberg = new BarretenbergWasm(crs);
    await barretenberg.init(initial);
    return barretenberg;
  }
  constructor(private crs: Crs, private loggerName?: string) {}

  /**
   * We need to let bb.js know about the CRS.
   * To do this, we preload the points for the maximum circuit size we expect.
   */
  public async ensureEnoughCrsPointsForCircuitSize(circuitSize: number) {
    const numPoints = circuitSize + 1;
    if (numPoints < this.crs.numPoints) {
      this.crs = new Crs(numPoints);
    }
  }

  /**
   * 20 pages by default. 20*2**16 \> 1mb stack size plus other overheads.
   * 8192 maximum by default. 512mb.
   * @param initial - Initial memory pages.
   * @param maximum - Max memory pages.
   */
  public async init(initial = 22, maximum = 8192) {
    let wasm: WasmModule;
    this.wasm = wasm = new WasmModule(
      await fetchCode(),
      module => ({
        // These are functions implementations for imports we've defined are needed.
        // The native C++ build defines these in a module called "env". We must implement TypeScript versions here.
        /**
         * Log a string from barretenberg.
         * @param addr - The string address to log.
         */
        logstr(addr: number) {
          const str = wasm.getMemoryAsString(addr);
          const m = wasm.getMemory();
          const str2 = `${str} (mem: ${(m.length / (1024 * 1024)).toFixed(2)}MB)`;
          wasm.getLogger()(str2);
        },
        // eslint-disable-next-line camelcase
        env_load_verifier_crs() {
          const crsPtr = wasm.call('bbmalloc', this.crs.getG2Data().length);
          wasm.writeMemory(crsPtr, this.crs.getG2Data());
          return crsPtr;
        },
        // eslint-disable-next-line camelcase
        env_load_prover_crs(numPoints: number) {
          if (this.crs.numPoints < numPoints) {
            throw new Error(
              `ABORT: A circuit asked for ${numPoints} points but only ${this.crs.numPoints} are available.
               You must either initialize BarretenbergWasm.new() with a second argument with enough points you ever expect to encounter, or to call ensureEnoughCrsPointsForCircuitSize() with (an upper bound of your) circuit size as you determine it.`,
            );
          }
          // TODO optimization, only write the number of requested points
          const crsPtr = wasm.call('bbmalloc', this.crs.getG1Data().length);
          wasm.writeMemory(crsPtr, this.crs.getG1Data());
          return crsPtr;
        },
        memory: module.getRawMemory(),
      }),
      this.loggerName,
    );
    await wasm.init(initial, maximum);
  }

  /**
   * Get a slice of memory between two addresses.
   * @param start - The start address.
   * @param end - The end address.
   * @returns A Uint8Array view of memory.
   */
  public getMemorySlice(start: number, end: number) {
    return this.wasm.getMemorySlice(start, end);
  }

  /**
   * Write data into the heap.
   * @param arr - The data to write.
   * @param offset - The address to write data at.
   */
  public writeMemory(offset: number, arr: Uint8Array) {
    this.wasm.writeMemory(offset, arr);
  }

  /**
   * Calls into the WebAssembly.
   * @param name - The method name.
   * @param args - The arguments to the method.
   * @returns The numeric integer or address result.
   */
  public call(name: string, ...args: any): number {
    return this.wasm.call(name, ...args);
  }
}
