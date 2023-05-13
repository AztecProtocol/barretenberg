import { Worker } from 'worker_threads';
import { dirname } from 'path';
import { fileURLToPath } from 'url';
import { readFile } from 'fs/promises';
import os from 'os';
import { randomBytes as cryptoRandomBytes } from 'crypto';
import { type BarretenbergWasm, type RemoteBarretenbergWasm } from '../barretenberg_wasm.js';
import { wrap } from 'comlink';
import { nodeEndpoint } from './node_endpoint.js';

export async function fetchCode() {
  const __dirname = dirname(fileURLToPath(import.meta.url));
  return await readFile(__dirname + '/../barretenberg.wasm');
}

// export function createWorker(i: number, module: WebAssembly.Module, memory: WebAssembly.Memory) {
//   return new Promise<Worker>(resolve => {
//     const __dirname = dirname(fileURLToPath(import.meta.url));
//     const worker = new Worker(
//       __dirname + `/barretenberg_thread.ts`,
//       // i === 0 ? { execArgv: ['--inspect-brk=0.0.0.0'] } : {},
//     );
//     // const debug = createDebug(`wasm:worker:${i}`);
//     // Worker posts a message when it's ready.
//     worker.once('message', () => resolve(worker));
//     worker.postMessage({ msg: 'start', data: { module, memory } });
//   });
// }
export function createWorker() {
  // return new Promise<Worker>(resolve => {
  const __dirname = dirname(fileURLToPath(import.meta.url));
  const worker = new Worker(
    __dirname + `/worker.ts`,
    // i === 0 ? { execArgv: ['--inspect-brk=0.0.0.0'] } : {},
  );
  return worker;

  // Wait till worker is ready.
  // const messageHandler = (event: any) => {
  //   if (event.data.ready) {
  //     // The worker is ready, resolve the promise
  //     worker.on('message', messageHandler);
  //     resolve(worker);
  //   }
  // };

  // // Listen for the ready message from the worker
  // worker.on('message', messageHandler);

  // const debug = createDebug(`wasm:worker:${i}`);
  // Worker posts a message when it's ready.
  // worker.addEventListener('message', () => resolve(worker));
  // worker.postMessage({ msg: 'start', data: { module, memory } });
  // });
}

export function getRemoteBarretenbergWasm(worker: Worker): RemoteBarretenbergWasm {
  return wrap<BarretenbergWasm>(nodeEndpoint(worker));
}

export function getNumCpu() {
  return os.cpus().length;
}

export function randomBytes(len: number) {
  return new Uint8Array(cryptoRandomBytes(len));
}
