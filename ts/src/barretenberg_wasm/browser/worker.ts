import { expose } from 'comlink';
import { BarretenbergWasm } from '../index.js';

// async function main() {
//   expose(await BarretenbergWasm.new(undefined, console.log));
//   self.postMessage({ ready: true });
// }

// main().catch(console.error);

expose(new BarretenbergWasm());

self.postMessage({ ready: true });
