import { wrap } from 'comlink';
import { BarretenbergWasmWorker, type BarretenbergWasm } from '../barretenberg_wasm.js';

export async function fetchCode() {
  const res = await fetch('/barretenberg.wasm');
  return await res.arrayBuffer();
}

export function createWorker() {
  return new Worker('barretenberg_wasm.js');
}

export function getRemoteBarretenbergWasm(worker: Worker): BarretenbergWasmWorker {
  return wrap<BarretenbergWasm>(worker);
}

export function getNumCpu() {
  return navigator.hardwareConcurrency;
}
