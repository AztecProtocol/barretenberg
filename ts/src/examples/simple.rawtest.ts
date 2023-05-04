import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { Crs } from '../index.js';
import { writeFileSync } from 'fs';
import createDebug from 'debug';

createDebug.enable('*');
const debug = createDebug('wasm');

function logger(message: string) {
  debug(message);
  writeFileSync(`worker.log`, message + '\n', { flag: 'a' });
}

const wasm = await BarretenbergWasm.new(16, logger);
const api = new BarretenbergApi(new BarretenbergBinder(wasm));

// Plus 1 need or ASSERT gets triggered. It was fine in release. Is the assertion wrong?
const crs = new Crs(2 ** 19 + 1);
await crs.init();
const pippengerPtr = api.eccNewPippenger(Buffer.from(crs.getG1Data()), crs.numPoints);

while (true) {
  api.examplesSimpleCreateAndVerifyProof(pippengerPtr, Buffer.from(crs.getG2Data()));
  // logger(`valid: ${valid}`);
}

await wasm.destroy();
