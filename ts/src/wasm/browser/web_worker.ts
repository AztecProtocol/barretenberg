import { DispatchMsg, TransportServer, WorkerListener } from '../../transport/index.js';
import { BarretenbergWasm } from '../barretenberg_wasm.js';

async function dispatch({ fn, args }: DispatchMsg) {
  if (fn === '__destroyWorker__') {
    transportServer.stop();
    return;
  }
  if (!(barretenbergWasm as any)[fn]) {
    throw new Error(`dispatch error, function not found: ${fn}`);
  }
  return await (barretenbergWasm as any)[fn](...args);
}

const barretenbergWasm = new BarretenbergWasm();
const transportListener = new WorkerListener(self);
const transportServer = new TransportServer<DispatchMsg>(transportListener, dispatch);
barretenbergWasm.on('log', (...args: any[]) => transportServer.broadcast({ fn: 'emit', args: ['log', ...args] }));
transportServer.start();
