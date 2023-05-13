import { expose } from 'comlink';
import { BarretenbergWasm } from '../index.js';

expose(new BarretenbergWasm());

self.postMessage({ ready: true });
