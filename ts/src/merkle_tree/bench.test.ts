import { BarretenbergWasm, WorkerPool } from '../wasm/index.js';
import { PooledPedersen } from '../crypto/index.js';
import { MerkleTree } from './index.js';
import { default as levelup } from 'levelup';
import { default as memdown } from 'memdown';
import { jest } from '@jest/globals';

jest.setTimeout(10000);

describe.skip('merkle_tree_benchmark', () => {
  let barretenberg!: BarretenbergWasm;
  const values: Buffer[] = [];

  beforeAll(async () => {
    barretenberg = new BarretenbergWasm();
    await barretenberg.init();

    for (let i = 0; i < 2 ** 12 - 2; ++i) {
      const v = Buffer.alloc(64, 0);
      v.writeUInt32LE(i, 0);
      values[i] = v;
    }
  });

  it('benchmark_tree', async () => {
    const pool = await WorkerPool.new(barretenberg, 4);
    const pedersen = new PooledPedersen(barretenberg, pool);
    const db = levelup(memdown());
    const tree = await MerkleTree.new(db, pedersen, 'test', 32);

    const start = new Date().getTime();
    await tree.updateElements(0, values);
    const end = new Date().getTime() - start;
    console.log(`Tree: ~${end / values.length}ms / insert`);

    await pool.destroy();
  });
});
