import { Crs } from '../crs/index.js';
import createDebug from 'debug';
import { newBarretenbergApiAsync } from '../factory/index.js';
import { RawBuffer } from '../types/index.js';

createDebug.enable('*');
const debug = createDebug('simple_test');

async function main() {
  const CIRCUIT_SIZE = 2 ** 18;

  debug('starting test...');
  const api = await newBarretenbergApiAsync();

  // Important to init slab allocator as first thing, to ensure maximum memory efficiency.
  await api.commonInitSlabAllocator(CIRCUIT_SIZE);

  // Plus 1 needed!
  const crs = await Crs.new(CIRCUIT_SIZE + 1);
  await api.srsInitSrs(new RawBuffer(crs.getG1Data()), crs.numPoints, new RawBuffer(crs.getG2Data()));

  for (let i = 0; i < 10; ++i) {
    debug(`iteration ${i} starting...`);
    await api.examplesSimpleCreateAndVerifyProof();
  }

  await api.destroy();

  debug('test complete.');
}

void main();
