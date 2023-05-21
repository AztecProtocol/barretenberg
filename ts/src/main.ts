#!/usr/bin/env node
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newMultiThreaded } from './factory/index.js';
import { readFileSync } from 'fs';
import { gunzipSync } from 'zlib';

const debug = createDebug('bb.js');
createDebug.enable('*');

function getBytecode() {
  const json = readFileSync('/mnt/user-data/charlie/noir-projects/512k/target/main.json', 'utf-8');
  console.log(json.length);
  const parsed = JSON.parse(json);

  const buffer = Buffer.from(parsed.bytecode, 'base64');
  console.log(buffer.length);

  const decompressed = gunzipSync(buffer);
  console.log(decompressed.length);

  return decompressed;
}

export async function main() {
  debug('starting test...');
  const api = await newMultiThreaded();
  try {
    // Plus 1 needed!
    const crs = await Crs.new(2 ** 19 + 1);
    const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);

    const acirComposer = await api.acirNewAcirComposer(pippengerPtr, crs.getG2Data());
    const bytecode = getBytecode();
    await api.acirInitProvingKey(acirComposer, bytecode);
    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(exactCircuitSize);

    // for (let i = 0; i < 10; ++i) {
    //   debug(`iteration ${i} starting...`);
    //   await api.examplesSimpleCreateAndVerifyProof(pippengerPtr, crs.getG2Data());
    // }

    debug('test complete.');
  } finally {
    await api.destroy();
  }
}

await main().catch(console.error);
