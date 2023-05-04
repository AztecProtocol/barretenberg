import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergApi } from './index.js';
import debug from 'debug';

debug.enable('*');

describe('env', () => {
  let api: BarretenbergApi;

  beforeAll(async () => {
    api = new BarretenbergApi(new BarretenbergBinder(await BarretenbergWasm.new(2)));
  }, 60000);

  afterAll(async () => {
    await api.destroy();
  });

  it('thread test', () => {
    const threads = api.binder.wasm.getNumWorkers();
    const iterations = 1000000 / 4;
    const result = api.envTestThreads(threads, iterations);
    expect(result).toBe(iterations);
  });

  // it('async call test', () => {
  //   wasm.call('condvar_test');
  // });

  // it('should set and get data via async call', async () => {
  //   const data = Buffer.from('This is a buffer containing some data to save.');
  //   await envSetData('key', data);
  //   const result = await envGetData('key');
  //   expect(result).toEqual(data);
  // });
});
