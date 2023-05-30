#!/usr/bin/env -S node --no-warnings
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newBarretenbergApiAsync } from './factory/index.js';
import { readFileSync, writeFileSync } from 'fs';
import { gunzipSync } from 'zlib';
import { RawBuffer } from './types/index.js';
import { numToUInt32BE } from './serialize/serialize.js';
import { Command } from 'commander';

createDebug.log = console.error.bind(console);
const debug = createDebug('bb.js');
// createDebug.enable('*');

// Maximum we support.
const CIRCUIT_SIZE = 2 ** 19;

function getBytecode(jsonPath: string) {
  const json = readFileSync(jsonPath, 'utf-8');
  const parsed = JSON.parse(json);
  const buffer = Buffer.from(parsed.bytecode, 'base64');
  const decompressed = gunzipSync(buffer);
  return decompressed;
}

function getWitness(witnessPath: string) {
  const data = readFileSync(witnessPath);
  return Buffer.concat([numToUInt32BE(data.length / 32), data]);
}

async function init() {
  // Plus 1 needed!
  const crs = await Crs.new(CIRCUIT_SIZE + 1);

  const api = await newBarretenbergApiAsync();

  // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
  await api.commonInitSlabAllocator(CIRCUIT_SIZE);

  // Load CRS into wasm global CRS state.
  // TODO: Make RawBuffer be default behaviour, and have a specific Vector type for when wanting length prefixed.
  await api.srsInitSrs(new RawBuffer(crs.getG1Data()), crs.numPoints, new RawBuffer(crs.getG2Data()));

  const acirComposer = await api.acirNewAcirComposer();

  return { api, acirComposer };
}

export async function proveAndVerify(jsonPath: string, witnessPath: string) {
  const { api, acirComposer } = await init();
  try {
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
    debug(`done.`);

    const verified = await api.acirVerifyProof(acirComposer, proof);
    console.log(`verified: ${verified}`);
    return verified;
  } finally {
    await api.destroy();
  }
}

export async function prove(jsonPath: string, witnessPath: string, outputPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(`circuit size: ${exactCircuitSize}`);

    debug(`creating proof...`);
    const witness = getWitness(witnessPath);
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness));
    debug(`done.`);

    writeFileSync(outputPath, proof);
    console.log(`proof written to: ${outputPath}`);
  } finally {
    await api.destroy();
  }
}

export async function gateCount(jsonPath: string) {
  const { api, acirComposer } = await init();
  try {
    const bytecode = getBytecode(jsonPath);
    await api.acirCreateCircuit(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    const gates = await api.acirGetTotalCircuitSize(acirComposer);
    console.log(`${gates}`);
  } finally {
    await api.acirDeleteAcirComposer(acirComposer);
    await api.destroy();
  }
}

export async function verify(jsonPath: string, proofPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    const verified = await api.acirVerifyProof(acirComposer, readFileSync(proofPath));
    console.log(`verified: ${verified}`);
    return verified;
  } finally {
    await api.destroy();
  }
}

export async function contract(jsonPath: string, outputPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    const contract = await api.acirGetSolidityVerifier(acirComposer);
    if (outputPath === '-') {
      console.log(contract);
    } else {
      writeFileSync(outputPath, contract);
      console.log(`contract written to: ${outputPath}`);
    }
  } finally {
    await api.destroy();
  }
}

export async function writeVk(jsonPath: string, outputPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    debug('initing verification key...');
    const vk = await api.acirGetVerificationKey(acirComposer);
    if (outputPath === '-') {
      process.stdout.write(vk);
    } else {
      writeFileSync(outputPath, vk);
      console.log(`vk written to: ${outputPath}`);
    }
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

program
  .command('prove')
  .description('Generate a proof and write it to a file.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .option('-w, --witness-path <path>', 'Specify the witness path', './target/witness.tr')
  .option('-o, --output-dir <path>', 'Specify the proof output dir', './proofs')
  .requiredOption('-n, --name <filename>', 'Output file name.')
  .action(async ({ jsonPath, witnessPath, outputDir, name }) => {
    await prove(jsonPath, witnessPath, outputDir + '/' + name);
  });

program
  .command('gates')
  .description('Print gate count to standard output.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .action(async ({ jsonPath }) => {
    await gateCount(jsonPath);
  });

program
  .command('verify')
  .description('Verify a proof. Process exists with success or failure code.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption('-p, --proof-path <path>', 'Specify the path to the proof')
  .action(async ({ jsonPath, proofPath }) => {
    await verify(jsonPath, proofPath);
  });

program
  .command('contract')
  .description('Output solidity verification key contract.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .option('-o, --output-path <path>', 'Specify the path to write the contract', '-')
  .action(async ({ jsonPath, outputPath }) => {
    await contract(jsonPath, outputPath);
  });

program
  .command('write_vk')
  .description('Output verification key.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption('-o, --output-path <path>', 'Specify the path to write the key')
  .action(async ({ jsonPath, outputPath }) => {
    await writeVk(jsonPath, outputPath);
  });

program.name('bb.js').parse(process.argv);
