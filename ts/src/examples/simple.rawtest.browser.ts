// import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
// import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
// import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { CachedNetCrs } from '../crs/cached_net_crs.js';
import { wrap, proxy } from 'comlink';
// import { Worker } from 'worker_threads';
// import { writeFileSync } from 'fs';
import createDebug from 'debug';
import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';

createDebug.enable('*');
// const debug = createDebug('wasm');

// function logger(message: string) {
//   debug(message);
//   // writeFileSync(`worker.log`, message + '\n', { flag: 'a' });
// }

async function main() {
  console.log('hello');
  // const BbApi = wrap<BarretenbergApi>(new Worker('./src/barretenberg_api/index.js'));
  const worker = new Worker('./barretenberg_wasm.js');
  const wasm = wrap<BarretenbergWasm>(worker);

  // Wait till worker is ready.
  await new Promise<void>(resolve => {
    const messageHandler = (event: any) => {
      if (event.data.ready) {
        // The worker is ready, resolve the promise
        worker.removeEventListener('message', messageHandler);
        resolve();
      }
    };

    // Listen for the ready message from the worker
    worker.addEventListener('message', messageHandler);
  });

  console.log('calling INIT!!!!!!!!!!!');
  // await wasm.on('log', proxy(console.log));
  await wasm.init();
  const api = new BarretenbergApi(new BarretenbergBinder(wasm));

  // Plus 1 needed!
  const crs = await CachedNetCrs.new(2 ** 19 + 1);
  const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);

  // while (true) {
  await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
  //   // logger(`valid: ${valid}`);
  // }

  worker.terminate();

  // await wasm.destroy();
  console.log('done');
}

void main();
