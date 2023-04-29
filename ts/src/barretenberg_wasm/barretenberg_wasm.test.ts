import { writeFileSync } from 'fs';
import { BarretenbergWasm } from './barretenberg_wasm.js';

function debug(message: string) {
  // const logFilePath = `worker.log`;
  // writeFileSync(logFilePath, message + '\n', { flag: 'a' });
  console.log(message);
}

describe('barretenberg wasm', () => {
  let wasm!: BarretenbergWasm;

  beforeAll(async () => {
    wasm = await BarretenbergWasm.new();
    wasm.on('log', debug);
  });

  afterAll(async () => {
    await wasm.destroy();
  });

  // it('should new malloc, transfer and slice mem', () => {
  //   const length = 1024;
  //   const ptr = wasm.call('bbmalloc', length);
  //   const buf = Buffer.alloc(length, 128);
  //   wasm.writeMemory(ptr, buf);
  //   const result = Buffer.from(wasm.getMemorySlice(ptr, ptr + length));
  //   wasm.call('bbfree', ptr);
  //   expect(result).toStrictEqual(buf);
  // });

});
