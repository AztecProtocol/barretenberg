import { BarretenbergBinder } from '../barretenberg_binder/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm/index.js';
import { BarretenbergApi } from './index.js';

describe('env', () => {
  let api: BarretenbergApi;

  beforeAll(async () => {
    api = new BarretenbergApi(new BarretenbergBinder(await BarretenbergWasm.new()));
    // api.binder.wasm.on('log', console.log);
  });

  afterAll(async () => {
    await api.destroy();
  });

  it('should thread test', () => {
    const result = api.envTestThreads(10);
    expect(result).toBe(10);
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
