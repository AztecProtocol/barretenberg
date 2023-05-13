import { BarretenbergBinderSync } from '../barretenberg_binder/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergApiSync } from './index.js';
import debug from 'debug';

debug.enable('*');

describe('env', () => {
  let api: BarretenbergApiSync;

  beforeAll(async () => {
    api = new BarretenbergApiSync(new BarretenbergBinderSync(await BarretenbergWasm.new(3)));
  });

  afterAll(async () => {
    await api.destroy();
  });

  it('thread test', () => {
    // Main thread doesn't do anything in this test, so -1.
    const threads = api.binder.wasm.getNumThreads() - 1;
    const iterations = 100000;
    const result = api.testThreads(threads, iterations);
    expect(result).toBe(iterations);
  });
});
