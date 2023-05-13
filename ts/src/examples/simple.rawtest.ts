import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { Crs } from '../crs/index.js';
import createDebug from 'debug';
import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';

createDebug.enable('*');
const debug = createDebug('simple_test');

async function main() {
  debug('starting test...');
  const { wasm, worker } = await BarretenbergWasm.newWorker();
  const api = new BarretenbergApi(new BarretenbergBinder(wasm));

  // Plus 1 needed!
  const crs = await Crs.new(2 ** 19 + 1);
  const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);

  // while (true) {
  await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
  //   // logger(`valid: ${valid}`);
  // }

  await wasm.destroy();
  await worker.terminate();

  debug('test complete.');
}

void main();
