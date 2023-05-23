import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { Crs } from '../crs/index.js';
import createDebug from 'debug';
import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';

createDebug.enable('*');
const debug = createDebug('simple_test');

async function main() {
  const CIRCUIT_SIZE = 2 ** 19;

  debug('starting test...');
  const { wasm, worker } = await BarretenbergWasm.newWorker();
  const api = new BarretenbergApi(new BarretenbergBinder(wasm));

  // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
  await api.commonInitSlabAllocator(CIRCUIT_SIZE);

  // Plus 1 needed!
  const crs = await Crs.new(CIRCUIT_SIZE + 1);
  const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
  debug(Buffer.from(crs.getG2Data()).toString('hex'));

  for (let i = 0; i < 10; ++i) {
    debug(`iteration ${i} starting...`);
    await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
  }

  await wasm.destroy();
  await worker.terminate();

  debug('test complete.');
}

void main();
