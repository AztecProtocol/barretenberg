#!/usr/bin/env -S node --no-warnings
import { Crs } from './crs/index.js';
import createDebug from 'debug';
import { newBarretenbergApiAsync } from './factory/index.js';
import { readFileSync, writeFileSync } from 'fs';
import { gunzipSync } from 'zlib';
import { RawBuffer } from './types/index.js';
import { numToInt32BE } from './serialize/serialize.js';
import { Command } from 'commander';
import { info } from 'console';

createDebug.log = console.error.bind(console);
const debug = createDebug('bb.js');
createDebug.enable('*');

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
  return Buffer.concat([numToInt32BE(data.length / 32), data]);
}

async function init() {
  // Plus 1 needed!
  const crs = await Crs.new(CIRCUIT_SIZE + 1);

  const api = await newBarretenbergApiAsync();

  // Import to init slab allocator as first thing, to ensure maximum memory efficiency.
  await api.commonInitSlabAllocator(CIRCUIT_SIZE);

  const pippengerPtr = await api.eccNewPippenger(crs.getG1Data(), crs.numPoints);
  const acirComposer = await api.acirNewAcirComposer(pippengerPtr, crs.getG2Data());

  return { api, acirComposer };
}

export async function proveAndVerify(jsonPath: string, witnessPath: string, is_recursive: boolean) {
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
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness), false);

    const verified = await api.acirVerifyProof(acirComposer, proof, false);
    debug(`verified: ${verified}`);
    return verified;
  } finally {
    await api.destroy();
  }
}

export async function prove(jsonPath: string, witnessPath: string, is_recursive: boolean, outputPath: string) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    const exactCircuitSize = await api.acirGetExactCircuitSize(acirComposer);
    debug(`circuit size: ${exactCircuitSize}`);

    debug(`creating proof...`);
    const witness = getWitness(witnessPath);
    const proof = await api.acirCreateProof(acirComposer, new RawBuffer(bytecode), new RawBuffer(witness), is_recursive);

    writeFileSync(outputPath, proof);
    debug('done.');
  } finally {
    await api.destroy();
  }
}

export async function verify(jsonPath: string, proofPath: string, is_recursive: boolean) {
  const { api, acirComposer } = await init();
  try {
    debug('initing proving key...');
    const bytecode = getBytecode(jsonPath);
    await api.acirInitProvingKey(acirComposer, new RawBuffer(bytecode), CIRCUIT_SIZE);

    debug('initing verification key...');
    await api.acirInitVerificationKey(acirComposer);

    const verified = await api.acirVerifyProof(acirComposer, readFileSync(proofPath), is_recursive);
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

export async function proof_as_fields(proofPath: string, num_inner_public_inputs: number, outputPath: string) {
  const { api, acirComposer } = await init();

  try {
    debug('serializing proof byte array into field elements');
    const proof_as_fields = await api.acirSerializeProofIntoFields(acirComposer, readFileSync(proofPath), num_inner_public_inputs);

    writeFileSync(outputPath, proof_as_fields);
    debug('done.');
  } finally {
    await api.destroy();
  }
}

export async function vk_as_fields(vkey_oututPath: string, key_hash_outputPath: string) {
  const { api, acirComposer } = await init();

  // TODO: move to passing in the key so we don't have to recompute it
  // or just keep it as the writeVK method currently recomputes the pkey too
  api.acirInitVerificationKey(acirComposer);
  try {
    debug('serializing proof byte array into field elements');
    const [vk_as_fields, vk_hash_as_fields] = await api.acirSerializeVerificationKeyIntoFields(acirComposer);

    writeFileSync(vkey_oututPath, vk_as_fields);
    writeFileSync(key_hash_outputPath, vk_hash_as_fields);
    debug('done.');
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
  .option('-r, --recursive', 'prove and verify using recursive prover and verifier')
  .action(async ({ jsonPath, witnessPath, is_recursive }) => {
    const result = await proveAndVerify(jsonPath, witnessPath, false);
    process.exit(result ? 0 : 1);
  });

program
  .command('prove')
  .description('Generate a proof and write it to a file.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .option('-w, --witness-path <path>', 'Specify the witness path', './target/witness.tr')
  .option('-r, --recursive', 'prove using recursive prover')
  .option('-o, --output-dir <path>', 'Specify the proof output dir', './proofs')
  .requiredOption('-n, --name <filename>', 'Output file name.')
  .action(async ({ jsonPath, witnessPath, is_recursive, outputDir, name }) => {
    await prove(jsonPath, witnessPath, is_recursive, outputDir + '/' + name);
  });

program
  .command('verify')
  .description('Verify a proof. Process exists with success or failure code.')
  .option('-j, --json-path <path>', 'Specify the JSON path', './target/main.json')
  .requiredOption('-p, --proof-path <path>', 'Specify the path to the proof')
  .option('-r, --recursive', 'prove using recursive prover')
  .action(async ({ jsonPath, proofPath, is_recursive }) => {
    await verify(jsonPath, proofPath, is_recursive);
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
  .requiredOption('-n, --num-public-inputs', 'Specify the number of public inputs')
  .requiredOption('-o, --output-path <path>', 'Specify the path to write the proof fields')
  .action(async ({ proofPath, numPublicInputs, outputPath }) => {
    await proof_as_fields(proofPath, numPublicInputs, outputPath);
  })

program
  .command('vk_as_fields')
  .description('Return the verifiation key represented as fields elements. Also return the verification key hash.')
  .requiredOption('-v, --vkey-output-path <path>', 'Specify the path to write the verification key fields')
  .requiredOption('-h, --key-hash-output-path <path>', 'Specify the path to write the verification key hash')
  .action(async ({vkey_oututPath, key_hash_outputPath }) => {
    await vk_as_fields(vkey_oututPath, key_hash_outputPath);
  })

program.parse(process.argv);
