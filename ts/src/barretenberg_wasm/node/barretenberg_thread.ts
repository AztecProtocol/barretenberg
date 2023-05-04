import { parentPort, threadId } from 'worker_threads';
import { BarretenbergWasm } from '../barretenberg_wasm.js';
import { writeFileSync } from 'fs';
import createDebug from 'debug';

// import inspector from 'node:inspector';
// if (threadId === 1) {
//   inspector.open();
//   inspector.waitForDebugger();
// }

if (!parentPort) {
  throw new Error('InvalidWorker');
}

const barretenbergWasm = new BarretenbergWasm();

const debug = createDebug(`wasm:worker:${threadId}`);

function logger(message: string) {
  debug(message);
  writeFileSync(`worker.log`, message + '\n', { flag: 'a' });
  // parentPort?.postMessage(message);
  // console.log(message);
}

parentPort.on('message', ({ msg, data }) => {
  // TODO: Potential race between 'start' and 'thread'. Process msgs on serial queue?
  switch (msg) {
    case 'start':
      void start(data);
      break;
    case 'thread':
      runThread(data);
      break;
    case 'exit':
      process.exit(0);
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
  parentPort?.postMessage(threadId);
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
