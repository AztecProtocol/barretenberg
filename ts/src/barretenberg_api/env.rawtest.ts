import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergApi } from './index.js';
import debug from 'debug';

debug.enable('*');

const api = new BarretenbergApi(new BarretenbergBinder(await BarretenbergWasm.new(2)));

// afterAll(async () => {
//   await api.destroy();
// });

while (true) {
  const threads = api.binder.wasm.getNumWorkers();
  const iterations = 10000000 / 4;
  api.envTestThreads(threads, iterations);
  await new Promise(resolve => setTimeout(resolve, 0));
}
