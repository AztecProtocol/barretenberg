import { expect } from 'chai';
import { BarretenbergApiAsync, newBarretenbergApiAsync } from '../factory/index.js';

describe('env', () => {
  let api: BarretenbergApiAsync;

  before(async function () {
    this.timeout(15000); // Set timeout for this hook
    api = await newBarretenbergApiAsync(3);
  });

  after(async () => {
    await api.destroy();
  });

  it('thread test', async () => {
    // Main thread doesn't do anything in this test, so -1.
    const threads = (await api.getNumThreads()) - 1;
    const iterations = 100000;
    const result = await api.testThreads(threads, iterations);
    expect(result).to.equal(iterations);
  });
});
