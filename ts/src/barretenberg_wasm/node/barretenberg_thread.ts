import { parentPort, threadId } from 'worker_threads';
import { BarretenbergWasm } from '../barretenberg_wasm.js';
// import { writeFileSync } from 'fs';

// import inspector from 'node:inspector';
// if (threadId === 1) {
//   inspector.open();
//   inspector.waitForDebugger();
// }

if (!parentPort) {
  debug('InvalidWorker');
  throw new Error('InvalidWorker');
}

const barretenbergWasm = new BarretenbergWasm();

// debug('worker launched!');

function debug(message: string) {
  // const logFilePath = `worker.log`;
  // writeFileSync(logFilePath, message + '\n', { flag: 'a' });
  // parentPort?.postMessage(message);
  // console.log(message);
}

parentPort.on('message', ({ msg, data }) => {
  // TODO: Process msgs on serial queue.
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
    debug('Missing module or memory.');
    return;
  }
  barretenbergWasm.on('log', debug);
  await barretenbergWasm.initThread(module, memory);
  debug(`worker ${threadId} started.`);
  // Signal ready.
  parentPort?.postMessage(threadId);
}

function runThread({ id, arg }: any) {
  debug(`worker ${threadId} processing thread ${id}...`);
  barretenbergWasm.call('wasi_thread_start', id, arg);
  debug(`worker ${threadId} completed thread ${id}.`);
}
