import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { writeFileSync } from 'fs';

function logger(message: string) {
  writeFileSync(`worker.log`, message + '\n', { flag: 'a' });
}

const wasm = await BarretenbergWasm.new(4, logger);

// afterAll(async () => {
//   await api.destroy();
// });

// wasm.call('start_spinner');
while (true) {
  wasm.call('thread_test');
}
