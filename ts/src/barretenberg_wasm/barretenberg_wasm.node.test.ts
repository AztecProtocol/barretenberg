import { expect } from 'chai';
import { type Worker } from 'worker_threads';
import { BarretenbergWasm, BarretenbergWasmWorker } from './barretenberg_wasm.js';

describe('barretenberg wasm', () => {
  let wasm!: BarretenbergWasm;

  before(async () => {
    // Note: beforeAll is changed to before in Mocha.
    wasm = await BarretenbergWasm.new();
  });

  after(async () => {
    // Note: afterAll is changed to after in Mocha.
    await wasm.destroy();
  });

  it('should new malloc, transfer and slice mem', () => {
    const length = 1024;
    const ptr = wasm.call('bbmalloc', length);
    const buf = Buffer.alloc(length, 128);
    wasm.writeMemory(ptr, buf);
    const result = Buffer.from(wasm.getMemorySlice(ptr, ptr + length));
    wasm.call('bbfree', ptr);
    expect(result).to.deep.equal(buf); // Using Chai's deep.equal instead of Jest's toStrictEqual
  });

  it('test abort', () => {
    expect(() => wasm.call('test_abort')).to.throw(); // Using Chai's throw() assertion
  });

  it('test c/c++ stdout/stderr', () => {
    // We're checking we don't crash, but you can manually confirm you see log lines handled by logstr.
    wasm.call('test_stdout_stderr');
  });
});

describe('barretenberg wasm worker', () => {
  let worker!: Worker;
  let wasm!: BarretenbergWasmWorker;

  before(async function () {
    this.timeout(20000);
    ({ wasm, worker } = (await BarretenbergWasm.newWorker(2)) as any);
  });

  after(async () => {
    await wasm.destroy();
    await worker.terminate();
  });

  it('should new malloc, transfer and slice mem', async () => {
    const length = 1024;
    const ptr = await wasm.call('bbmalloc', length);
    const buf = Buffer.alloc(length, 128);
    await wasm.writeMemory(ptr, buf);
    const result = Buffer.from(await wasm.getMemorySlice(ptr, ptr + length));
    await wasm.call('bbfree', ptr);
    expect(result).to.deep.equal(buf);
  });
});
