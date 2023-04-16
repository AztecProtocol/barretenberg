import { envGetData, envSetData } from './index.js';

describe('env', () => {
  it('should set and get data via async call', async () => {
    const data = Buffer.from('This is a buffer containing some data to save.');
    await envSetData('key', data);
    const result = await envGetData('key');
    expect(result).toEqual(data);
  });
});
