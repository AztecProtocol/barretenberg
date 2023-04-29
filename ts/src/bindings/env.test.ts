import { wasm } from '../call_wasm_export/index.js';
import { envGetData, envSetData, envTestThreads } from './index.js';

describe('env', () => {
  it('should thread test', async () => {
    const result = envTestThreads(10);
    expect(result).toBe(10);
    await wasm.destroy();
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
