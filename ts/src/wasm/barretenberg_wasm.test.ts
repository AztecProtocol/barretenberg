import { FileCrs, SRS_DEV_PATH } from '../crs/index.js';
import { BarretenbergWasm } from './barretenberg_wasm.js';

describe('basic barretenberg smoke test', () => {
  const wasm: BarretenbergWasm = new BarretenbergWasm();

  beforeAll(async () => {
    await wasm.init();
  });

  it('should correctly pass CRS data through env_load_verifier_crs', async () => {
    const crs = new FileCrs(0, SRS_DEV_PATH);
    await crs.init();
    const g2DataPtr = wasm.call('test_env_load_verifier_crs');
    const g2Data = wasm.getMemorySlice(g2DataPtr, g2DataPtr + 128);
    expect(Buffer.from(g2Data)).toStrictEqual(crs.getG2Data());
    wasm.call('bbfree', g2DataPtr);
  });

  it('should correctly pass CRS data through env_load_prover_crs', async () => {
    const numPoints = 1024;
    const crs = new FileCrs(numPoints, SRS_DEV_PATH);
    await crs.init();
    const g1DataPtr = await wasm.call('test_env_load_prover_crs', numPoints);
    const g1Data = wasm.getMemorySlice(g1DataPtr, g1DataPtr + numPoints * 64);
    expect(Buffer.from(g1Data)).toStrictEqual(crs.getG1Data());
    wasm.call('bbfree', g1DataPtr);
  });
});
