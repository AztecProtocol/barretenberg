import { expect } from 'chai';
import { Crs } from '../index.js';
import { BarretenbergApiAsync, newBarretenbergApiAsync } from '../factory/index.js';
import { RawBuffer } from '../types/index.js';

describe('simple', () => {
  let api: BarretenbergApiAsync;

  before(async function () {
    this.timeout(30000);

    api = await newBarretenbergApiAsync();

    // Important to init slab allocator as first thing, to ensure maximum memory efficiency.
    const CIRCUIT_SIZE = 2 ** 19;
    await api.commonInitSlabAllocator(CIRCUIT_SIZE);

    const crs = await Crs.new(2 ** 19 + 1);
    await api.srsInitSrs(new RawBuffer(crs.getG1Data()), crs.numPoints, new RawBuffer(crs.getG2Data()));
  });

  after(async () => {
    await api.destroy();
  });

  it('should construct 512k gate proof', async function () {
    this.timeout(90000);

    const valid = await api.examplesSimpleCreateAndVerifyProof();
    expect(valid).to.equal(true);
  });
});
