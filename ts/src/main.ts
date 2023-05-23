#!/usr/bin/env node
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newMultiThreaded } from './factory/index.js';
import { readFileSync } from 'fs';
import { gunzipSync } from 'zlib';
import { RawBuffer } from './types/index.js';
import { numToInt32BE } from './serialize/serialize.js';

const debug = createDebug('bb.js');
createDebug.enable('*');

function getBytecode() {
  const json = readFileSync('/mnt/user-data/charlie/noir-projects/512k/target/main.json', 'utf-8');
  const parsed = JSON.parse(json);
  const buffer = Buffer.from(parsed.bytecode, 'base64');
  const decompressed = gunzipSync(buffer);
  return decompressed;
}

function getWitness() {
  const data = readFileSync('/mnt/user-data/charlie/noir-projects/512k/target/witness.tr');
  return Buffer.concat([numToInt32BE(data.length / 32), data]);
}

// nargo use bb.js: backend -> bb.js
// backend prove --data-dir data --witness /foo/bar/witness.tr --json /foo/bar/main.json
// backend verify ...
// backend get_total_num_gates --data-dir data --json /foo/bar/main.json
// backend get_sol_contract --data-dir data --json /foo/bar/main.json --output
// backend get_features
// OPTIONAL stateful backend:
// backend start
// backend stop
export async function main() {
  debug('starting test...');
  const api = await newMultiThreaded();
  try {
    const CIRCUIT_SIZE = 2 ** 19;

    // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
    await api.commonInitSlabAllocator(CIRCUIT_SIZE);

    // Plus 1 needed!
    const crs = await Crs.new(2 ** 19 + 1);
    const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);

    const acirComposer = await api.acirNewAcirComposer(pippengerPtr, crs.getG2Data());

    // debug('initing proving key...');
    const bytecode = getBytecode();
    // await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    // const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    // debug(`circuit size: ${exactCircuitSize}`);

    debug('creating proof...');
    const witness = getWitness();
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness));

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    debug('verifying...');
    const verified = await api.acirVerifyProof(acirComposer, proof);
    debug(`verified: ${verified}`);

    debug('test complete.');
  } finally {
    await api.destroy();
  }
}

await main().catch(console.error);
