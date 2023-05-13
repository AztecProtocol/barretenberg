import { parentPort } from 'worker_threads';
import { expose } from 'comlink';
import { BarretenbergWasm } from '../index.js';
import { nodeEndpoint } from './node_endpoint.js';

// async function main() {
//   if (!parentPort) {
//     throw new Error('No parentPort');
//   }

//   expose(await BarretenbergWasm.new(undefined, console.log), nodeEndpoint(parentPort));
//   parentPort.postMessage({ ready: true });
// }

// main().catch(console.error);

if (!parentPort) {
  throw new Error('No parentPort');
}

expose(new BarretenbergWasm(), nodeEndpoint(parentPort));
