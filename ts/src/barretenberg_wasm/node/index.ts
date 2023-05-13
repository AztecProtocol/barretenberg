import { Worker } from 'worker_threads';
import { dirname } from 'path';
import { fileURLToPath } from 'url';
import { readFile } from 'fs/promises';
import os from 'os';
import { randomBytes as cryptoRandomBytes } from 'crypto';
import { type BarretenbergWasm, type BarretenbergWasmWorker } from '../barretenberg_wasm.js';
import { wrap } from 'comlink';
import { nodeEndpoint } from './node_endpoint.js';

export async function fetchCode() {
  const __dirname = dirname(fileURLToPath(import.meta.url));
  return await readFile(__dirname + '/../barretenberg.wasm');
}

export function createWorker() {
  const __dirname = dirname(fileURLToPath(import.meta.url));
  return new Worker(__dirname + `/worker.ts`);
}

export function getRemoteBarretenbergWasm(worker: Worker): BarretenbergWasmWorker {
  return wrap<BarretenbergWasm>(nodeEndpoint(worker)) as BarretenbergWasmWorker;
}

export function getNumCpu() {
  return os.cpus().length;
}

export function randomBytes(len: number) {
  return new Uint8Array(cryptoRandomBytes(len));
}
