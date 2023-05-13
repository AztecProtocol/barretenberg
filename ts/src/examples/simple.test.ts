import { BarretenbergApi } from '../barretenberg_api/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { Crs } from '../index.js';

describe('simple', () => {
  let api: BarretenbergApi;

  beforeAll(async () => {
    api = new BarretenbergApi(new BarretenbergBinder(await BarretenbergWasm.new()));
  }, 20000);

  afterAll(async () => {
    await api.destroy();
  });

  it('should construct proof', async () => {
    const crs = new Crs(2 ** 19 + 1);
    await crs.init();
    const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
    const valid = await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
    expect(valid).toBe(true);
  });
});
