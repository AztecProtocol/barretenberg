import { DataStore } from '../data_store.js';
// import { default as levelup, LevelUp } from 'levelup';
// import { default as leveldown } from 'leveldown';
// import { default as memdown } from 'memdown';

export class NodeDataStore implements DataStore {
  // private db: LevelUp;
  private store: { [key: string]: Buffer } = {};

  // eslint-disable-next-line
  constructor(path?: string) {
    // LevelDown seemingly doesn't want to webpack... Fix.
    // this.db = levelup(path ? leveldown(path) : memdown());
    // this.db = levelup(memdown());
  }

  get(key: string): Promise<Buffer | undefined> {
    return Promise.resolve(this.store[key]);
    // return await this.db.get(key).catch(() => {});
    // throw new Error('Not implemented.');
  }

  set(key: string, value: Buffer): Promise<void> {
    this.store[key] = value;
    return Promise.resolve();
    // await this.db.put(key, value);
    // throw new Error('Not implemented.');
  }
}
