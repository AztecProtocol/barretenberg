import { Crs } from '../index.js';
import { BarretenbergApiAsync, newBarretenbergApiAsync } from '../factory/index.js';

describe('simple', () => {
  let api: BarretenbergApiAsync;

  beforeAll(async () => {
    api = await newBarretenbergApiAsync();

    // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
    const CIRCUIT_SIZE = 2 ** 19;
    await api.commonInitSlabAllocator(CIRCUIT_SIZE);
  }, 20000);

  afterAll(async () => {
    await api.destroy();
  });

  it('should construct 512k gate proof', async () => {
    const crs = await Crs.new(2 ** 19 + 1);
    const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
    const valid = await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
    expect(valid).toBe(true);
  }, 60000);
});
