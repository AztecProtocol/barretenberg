import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { Crs } from '../index.js';
import debug from 'debug';

debug.enable('wasm');

describe('simple', () => {
  let api: BarretenbergApi;

  beforeAll(async () => {
    api = new BarretenbergApi(new BarretenbergBinder(await BarretenbergWasm.new(8, debug('wasm'))));
  });

  afterAll(async () => {
    await api.destroy();
  });

  it('should construct proof', async () => {
    // Plus 1 need or ASSERT gets triggered. It was fine in release. Is the assertion wrong?
    const crs = new Crs(2 ** 19 + 1);
    await crs.init();
    const pippengerPtr = api.eccNewPippenger(Buffer.from(crs.getG1Data()), crs.numPoints);
    const valid = api.examplesSimpleCreateAndVerifyProof(pippengerPtr, Buffer.from(crs.getG2Data()));
    expect(valid).toBe(true);
  });
});
