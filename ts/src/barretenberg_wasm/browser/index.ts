import { wrap } from 'comlink';
import { RemoteBarretenbergWasm, type BarretenbergWasm } from '../barretenberg_wasm.js';

export async function fetchCode() {
  const res = await fetch('/barretenberg.wasm');
  return await res.arrayBuffer();
}

export function createWorker() {
  return new Worker('barretenberg_wasm.js');
  // return new Promise<RemoteBarretenbergWasm>(resolve => {
  // const worker = new Worker('barretenberg_wasm.js');
  // const wasm = wrap<BarretenbergWasm>(worker);

  // // Wait till worker is ready.
  // const messageHandler = (event: any) => {
  //   if (event.data.ready) {
  //     // The worker is ready, resolve the promise
  //     worker.removeEventListener('message', messageHandler);
  //     resolve(wasm);
  //   }
  // };

  // // Listen for the ready message from the worker
  // worker.addEventListener('message', messageHandler);

  // const debug = createDebug(`wasm:worker:${i}`);
  // Worker posts a message when it's ready.
  // worker.addEventListener('message', () => resolve(worker));
  // worker.postMessage({ msg: 'start', data: { module, memory } });
  // });
}

export function getRemoteBarretenbergWasm(worker: Worker): RemoteBarretenbergWasm {
  return wrap<BarretenbergWasm>(worker);
}

export function getNumCpu() {
  return navigator.hardwareConcurrency;
}

export const randomBytes = (len: number) => {
  const getWebCrypto = () => {
    if (typeof window !== 'undefined' && window.crypto) return window.crypto;
    if (typeof self !== 'undefined' && self.crypto) return self.crypto;
    return undefined;
  };

  const crypto = getWebCrypto();
  if (!crypto) {
    throw new Error('randomBytes UnsupportedEnvironment');
  }

  const buf = new Uint8Array(len);

  // limit of Crypto.getRandomValues()
  // https://developer.mozilla.org/en-US/docs/Web/API/Crypto/getRandomValues
  const MAX_BYTES = 65536;

  if (len > MAX_BYTES) {
    // this is the max bytes crypto.getRandomValues
    // can do at once see https://developer.mozilla.org/en-US/docs/Web/API/window.crypto.getRandomValues
    for (let generated = 0; generated < len; generated += MAX_BYTES) {
      // buffer.slice automatically checks if the end is past the end of
      // the buffer so we don't have to here
      crypto.getRandomValues(buf.subarray(generated, generated + MAX_BYTES));
    }
  } else {
    crypto.getRandomValues(buf);
  }

  return buf;
};
