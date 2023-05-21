import { BarretenbergApi, BarretenbergApiSync } from '../barretenberg_api/index.js';
import { BarretenbergBinder, BarretenbergBinderSync } from '../barretenberg_binder/index.js';
import { BarretenbergWasm, BarretenbergWasmWorker } from '../barretenberg_wasm/index.js';

export async function newSingleThreaded() {
  return new BarretenbergApiSync(new BarretenbergBinderSync(await BarretenbergWasm.new()));
}

class BarretenbergApiAsync extends BarretenbergApi {
  constructor(private worker: any, private wasm: BarretenbergWasmWorker) {
    super(new BarretenbergBinder(wasm));
  }

  async destroy() {
    await this.wasm.destroy();
    await this.worker.terminate();
  }
}

export async function newMultiThreaded() {
  const { wasm, worker } = await BarretenbergWasm.newWorker();
  return new BarretenbergApiAsync(worker, wasm);
}
