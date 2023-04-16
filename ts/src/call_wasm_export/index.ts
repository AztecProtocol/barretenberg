import { BarretenbergWasm } from '../barretenberg_wasm/barretenberg_wasm.js';
import { HeapAllocator } from './heap_allocator.js';
import { Bufferable, OutputType } from '../serialize/index.js';

export const wasm = await BarretenbergWasm.new();

/**
 * Calls a WASM export function, handles allocating/freeing of memory, and serializing/deserializing to types.
 *
 * Notes on function binding ABI:
 * All functions can have an arbitrary number of input and output args.
 * All arguments must be pointers.
 * Input args are determined by being const or pointer to const.
 * Output args must come after input args.
 * All input data is big-endian.
 * All output data is big-endian, except output heap alloc pointers.
 * As integer types are converted to/from big-endian form, we shouldn't have to worry about memory alignment. (SURE?)
 * All functions should return void.
 * This binding function is responsible for allocating argument memory (including output memory).
 * Variable length output args are allocated on the heap, and the resulting pointer is written to the output arg ptr,
 * hence the above statement remains true.
 * Binding will free any variable length output args that were allocated on the heap.
 */
export function callWasmExport(funcName: string, inArgs: Bufferable[], outTypes: OutputType[]) {
  const alloc = new HeapAllocator(wasm);
  const inPtrs = alloc.copyToMemory(inArgs);
  const outPtrs = alloc.getOutputPtrs(outTypes);
  wasm.call(funcName, ...inPtrs, ...outPtrs);
  const outArgs = deserializeOutputArgs(outTypes, outPtrs, alloc);
  alloc.freeAll();
  return outArgs;
}

export async function asyncCallWasmExport(funcName: string, inArgs: Bufferable[], outTypes: OutputType[]) {
  const alloc = new HeapAllocator(wasm);
  const inPtrs = alloc.copyToMemory(inArgs);
  const outPtrs = alloc.getOutputPtrs(outTypes);
  await wasm.asyncCall(funcName, ...inPtrs, ...outPtrs);
  const outArgs = deserializeOutputArgs(outTypes, outPtrs, alloc);
  alloc.freeAll();
  return outArgs;
}

function deserializeOutputArgs(outTypes: OutputType[], outPtrs: number[], alloc: HeapAllocator) {
  return outTypes.map((t, i) => {
    if (t.SIZE_IN_BYTES) {
      return t.fromBuffer(wasm.getMemorySlice(outPtrs[i]));
    }
    const ptr = Buffer.from(wasm.getMemorySlice(outPtrs[i], outPtrs[i] + 4)).readUInt32LE();
    alloc.addOutputPtr(ptr);
    return t.fromBuffer(wasm.getMemorySlice(ptr));
  });
}
