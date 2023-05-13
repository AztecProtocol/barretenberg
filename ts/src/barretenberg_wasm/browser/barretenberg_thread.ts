import { BarretenbergWasm } from '../barretenberg_wasm.js';
import createDebug from 'debug';

const barretenbergWasm = new BarretenbergWasm();
const threadId = Math.random().toString(16).substring(0, 8);

const debug = createDebug(`wasm:worker:${threadId}`);

function logger(message: string) {
  debug(message);
  // console.log(message);
}

self.addEventListener('message', e => {
  const { msg, data } = e.data;

  switch (msg) {
    case 'start':
      void start(data);
      break;
    case 'thread':
      runThread(data);
      break;
    case 'exit':
      self.postMessage('exit');
      self.close();
  }
});

async function start({ module, memory }: any) {
  if (!module || !memory) {
    logger('Missing module or memory.');
    return;
  }
  barretenbergWasm.on('log', logger);
  await barretenbergWasm.initThread(module, memory);
  // logger(`worker ${threadId} started.`);
  // Signal ready.
  self.postMessage(threadId);
}

function runThread({ id, arg }: any) {
  while (true) {
    try {
      // logger(`worker ${threadId} processing thread ${id} with arg ${arg}...`);
      barretenbergWasm.call('wasi_thread_start', id, arg);
      // logger(`worker ${threadId} completed thread ${id}.`);
      break;
    } catch (err: any) {
      logger(`exception ${err.message}`);
    }
  }
}
