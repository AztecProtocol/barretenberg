#!/usr/bin/env -S node --no-warnings
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newBarretenbergApiAsync } from './factory/index.js';
import { readFileSync } from 'fs';
import { gunzipSync } from 'zlib';
import { RawBuffer } from './types/index.js';
import { numToInt32BE } from './serialize/serialize.js';
import { Command } from 'commander';

const debug = createDebug('bb.js');
createDebug.enable('*');

function getBytecode(jsonPath: string) {
  const json = readFileSync(jsonPath, 'utf-8');
  const parsed = JSON.parse(json);
  const buffer = Buffer.from(parsed.bytecode, 'base64');
  const decompressed = gunzipSync(buffer);
  return decompressed;
}

function getWitness(witnessPath: string) {
  const data = readFileSync(witnessPath);
  return Buffer.concat([numToInt32BE(data.length / 32), data]);
}

function getProof(proofPath: string) {
  const data = readFileSync(proofPath);
  return data;
}

export async function proveAndVerify(jsonPath: string, witnessPath: string) {
  const api = await newBarretenbergApiAsync();
  try {
    const CIRCUIT_SIZE = 2 ** 19;

    // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
    await api.commonInitSlabAllocator(CIRCUIT_SIZE);

    // Plus 1 needed!
    const crs = await Crs.new(2 ** 19 + 1);
    const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);

    const acirComposer = await api.acirNewAcirComposer(pippengerPtr, crs.getG2Data());

    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(`circuit size: ${exactCircuitSize}`);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    debug(`creating proof...`);
    const witness = getWitness(witnessPath);
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness));

    const verified = await api.acirVerifyProof(acirComposer, proof);
    debug(`verified: ${verified}`);

    return verified;
  } finally {
    await api.destroy();
  }
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

const program = new Command();

program
  .command('prove_and_verify')
  .description('Generate a proof and verify it. Process exits with success or failure code.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .option('-w, --witness-path <path>', 'Specify the witness path', './target/witness.tr')
  .action(async ({ jsonPath, witnessPath }) => {
    const result = await proveAndVerify(jsonPath, witnessPath);
    process.exit(result ? 0 : 1);
  });

program.parse(process.argv);
