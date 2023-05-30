#!/usr/bin/env -S node --no-warnings
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newBarretenbergApiAsync } from './factory/index.js';
import { readFileSync, writeFileSync } from 'fs';
import { gunzipSync } from 'zlib';
import { RawBuffer } from './types/index.js';
import { numToInt32BE } from './serialize/serialize.js';
import { Command } from 'commander';
import { exit } from 'process';

createDebug.log = console.error.bind(console);
const debug = createDebug('bb.js');
createDebug.enable('*');

// Maximum we support.
const CIRCUIT_SIZE = 2 ** 18;

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

async function init() {
  // Plus 1 needed!
  const crs = await Crs.new(2 ** 19 + 1);

  const api = await newBarretenbergApiAsync();

  // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
  await api.commonInitSlabAllocator(CIRCUIT_SIZE);

  const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
  const acirComposer = await api.acirNewAcirComposer(pippengerPtr, crs.getG2Data());

  return { api, acirComposer };
}

export async function proveAndVerify(jsonPath: string, witnessPath: string, isRecursive: boolean) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);
    debug(`got proving key`);

    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(`circuit size: ${exactCircuitSize}`);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    debug(`creating proof...`);
    const witness = getWitness(witnessPath);
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness), isRecursive);

    const verified = await api.acirVerifyProof(acirComposer, proof, isRecursive);
    debug(`verified: ${verified}`);
    return verified;
  } finally {
    await api.destroy();
  }
}

export async function prove(jsonPath: string, witnessPath: string, isRecursive: boolean, outputPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(`circuit size: ${exactCircuitSize}`);

    debug(`creating proof...`);
    const witness = getWitness(witnessPath);
    debug({ bytecode, witness });
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness), isRecursive);
    debug(`got proof`);
    writeFileSync(outputPath, Buffer.from(proof));
    debug('done.');
  } finally {
    await api.destroy();
  }
}

export async function verify(jsonPath: string, proofPath: string, isRecursive: boolean) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    const verified = await api.acirVerifyProof(acirComposer, readFileSync(proofPath), isRecursive);
    debug(`verified: ${verified}`);
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
      debug(`contract written to: ${outputPath}`);
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
      debug(`vk written to: ${outputPath}`);
    }
  } finally {
    await api.destroy();
  }
}

export async function proofAsFields(proofPath: string, numInnerPublicInputs: number, outputPath: string) {
  const { api, acirComposer } = await init();

  try {
    debug('serializing proof byte array into field elements');
    const proofAsFieldsBuffer = await api.acirSerializeProofIntoFields(
      acirComposer,
      readFileSync(proofPath),
      numInnerPublicInputs,
    );
    const proofAsFields = bufferAsFieldHex(Buffer.from(proofAsFieldsBuffer));

    writeFileSync(outputPath, proofAsFields);
    debug('done.');
  } finally {
    await api.destroy();
  }
}

export async function vkAsFields(jsonPath: string, vkeyOutputPath: string) {
  const { api, acirComposer } = await init();

  // TODO: consider moving to passing in the key so we don't have to recompute it
  // or just keep it as the writeVK method currently recomputes the pkey too ¯\_(ツ)_/¯
  debug('initing proving key...');
  const bytecode = getBytecode(jsonPath);
  await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

  await api.acirInitVerificationKey(acirComposer);
  try {
    debug('serializing vk byte array into field elements');
    const [vkAsFieldsBuffer, vkHashBuffer] = await api.acirSerializeVerificationKeyIntoFields(acirComposer);
    const vkFields = bufferAsFieldHex(Buffer.concat([Buffer.from(vkHashBuffer), Buffer.from(vkAsFieldsBuffer)]));

    writeFileSync(vkeyOutputPath, vkFields);
    debug('done.');
  } finally {
    await api.destroy();
  }
}

function bufferAsFieldHex(buffer: Buffer): string {
  const hex = buffer.toString('hex');
  const splitHex = hex.match(/.{1,64}/g);
  if (splitHex == null) {
    exit();
  } else {
    for (let i = 0; i < splitHex.length; i++) {
      splitHex[i] = '0x'.concat(splitHex[i]);
    }
    const separateFields = JSON.stringify(splitHex);
    return separateFields;
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
  .option('-r, --recursive', 'prove and verify using recursive prover and verifier', false)
  .action(async ({ jsonPath, witnessPath, recursive }) => {
    const result = await proveAndVerify(jsonPath, witnessPath, recursive);
    process.exit(result ? 0 : 1);
  });

program
  .command('prove')
  .description('Generate a proof and write it to a file.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .option('-w, --witness-path <path>', 'Specify the witness path', './target/witness.tr')
  .option('-r, --recursive', 'prove using recursive prover', false)
  .option('-o, --output-dir <path>', 'Specify the proof output dir', './proofs')
  .requiredOption('-n, --name <filename>', 'Output file name.')
  .action(async ({ jsonPath, witnessPath, recursive, outputDir, name }) => {
    await prove(jsonPath, witnessPath, recursive, outputDir + '/' + name);
  });

program
  .command('verify')
  .description('Verify a proof. Process exists with success or failure code.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption('-p, --proof-path <path>', 'Specify the path to the proof')
  .option('-r, --recursive', 'prove using recursive prover', false)
  .action(async ({ jsonPath, proofPath, recursive }) => {
    await verify(jsonPath, proofPath, recursive);
  });

program
  .command('contract')
  .description('Output solidity verification key contract.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption('-o, --output-path <path>', 'Specify the path to write the contract')
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

program
  .command('proof_as_fields')
  .description('Return the proof as fields elements')
  .requiredOption('-p, --proof-path <path>', 'Specify the proof path')
  .requiredOption('-n, --num-public-inputs <number>', 'Specify the number of public inputs')
  .requiredOption('-o, --output-path <path>', 'Specify the JSON path to write the proof fields')
  .action(async ({ proofPath, numPublicInputs, outputPath }) => {
    await proofAsFields(proofPath, numPublicInputs, outputPath);
  });

program
  .command('vk_as_fields')
  .description('Return the verifiation key represented as fields elements. Also return the verification key hash.')
  .requiredOption('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption(
    '-v, --vkey-output-path <path>',
    'Specify the JSON path to write the verification key fields and key hash',
  )
  .action(async ({ jsonPath, vkeyOutputPath }) => {
    await vkAsFields(jsonPath, vkeyOutputPath);
  });

program.parse(process.argv);
