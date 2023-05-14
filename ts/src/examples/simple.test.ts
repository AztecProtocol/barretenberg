import { BarretenbergApiSync } from '../barretenberg_api/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergBinderSync } from '../barretenberg_binder/index.js';
import { Crs } from '../index.js';

describe('simple', () => {
  let api: BarretenbergApiSync;

  beforeAll(async () => {
    api = new BarretenbergApiSync(new BarretenbergBinderSync(await BarretenbergWasm.new()));
  }, 20000);

  afterAll(async () => {
    await api.destroy();
  });

  it('should construct 512k gate proof', async () => {
    const crs = await Crs.new(2 ** 19 + 1);
    const pippengerPtr = api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
    const valid = api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
    expect(valid).toBe(true);
  }, 60000);
});
