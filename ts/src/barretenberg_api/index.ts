// WARNING: FILE CODE GENERATED BY BINDGEN UTILITY. DO NOT EDIT!
/* eslint-disable @typescript-eslint/no-unused-vars */
import { BarretenbergWasmMain } from '../barretenberg_wasm/barretenberg_wasm_main/index.js';
import { BarretenbergWasmWorker, BarretenbergWasm } from '../barretenberg_wasm/index.js';
import {
  BufferDeserializer,
  NumberDeserializer,
  VectorDeserializer,
  BoolDeserializer,
  StringDeserializer,
  serializeBufferable,
  OutputType,
} from '../serialize/index.js';
import { Fr, Fq, Point, Buffer32, Buffer128, Ptr } from '../types/index.js';

export class BarretenbergApi {
  constructor(protected wasm: BarretenbergWasmWorker | BarretenbergWasmMain) {}

  async pedersenCommit(inputsBuffer: Fr[], ctxIndex: number): Promise<Point> {
    const inArgs = [inputsBuffer, ctxIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Point];
    const result = await this.wasm.callWasmExport(
      'pedersen_commit',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async pedersenHash(inputsBuffer: Fr[], hashIndex: number): Promise<Fr> {
    const inArgs = [inputsBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'pedersen_hash',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async pedersenHashes(inputsBuffer: Fr[], hashIndex: number): Promise<Fr> {
    const inArgs = [inputsBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'pedersen_hashes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async pedersenHashBuffer(inputBuffer: Uint8Array, hashIndex: number): Promise<Fr> {
    const inArgs = [inputBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'pedersen_hash_buffer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async poseidon2Hash(inputsBuffer: Fr[]): Promise<Fr> {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'poseidon2_hash',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async poseidon2Hashes(inputsBuffer: Fr[]): Promise<Fr> {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'poseidon2_hashes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async poseidon2Permutation(inputsBuffer: Fr[]): Promise<Fr[]> {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = await this.wasm.callWasmExport(
      'poseidon2_permutation',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async poseidon2HashAccumulate(inputsBuffer: Fr[]): Promise<Fr> {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'poseidon2_hash_accumulate',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async blake2s(data: Uint8Array): Promise<Buffer32> {
    const inArgs = [data].map(serializeBufferable);
    const outTypes: OutputType[] = [Buffer32];
    const result = await this.wasm.callWasmExport(
      'blake2s',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async blake2sToField(data: Uint8Array): Promise<Fr> {
    const inArgs = [data].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = await this.wasm.callWasmExport(
      'blake2s_to_field_',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async aesEncryptBufferCbc(input: Uint8Array, iv: Uint8Array, key: Uint8Array, length: number): Promise<Uint8Array> {
    const inArgs = [input, iv, key, length].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'aes_encrypt_buffer_cbc',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async aesDecryptBufferCbc(input: Uint8Array, iv: Uint8Array, key: Uint8Array, length: number): Promise<Uint8Array> {
    const inArgs = [input, iv, key, length].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'aes_decrypt_buffer_cbc',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async srsInitSrs(pointsBuf: Uint8Array, numPoints: number, g2PointBuf: Uint8Array): Promise<void> {
    const inArgs = [pointsBuf, numPoints, g2PointBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'srs_init_srs',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async srsInitGrumpkinSrs(pointsBuf: Uint8Array, numPoints: number): Promise<void> {
    const inArgs = [pointsBuf, numPoints].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'srs_init_grumpkin_srs',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async examplesSimpleCreateAndVerifyProof(): Promise<boolean> {
    const inArgs = [].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'examples_simple_create_and_verify_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async testThreads(threads: number, iterations: number): Promise<number> {
    const inArgs = [threads, iterations].map(serializeBufferable);
    const outTypes: OutputType[] = [NumberDeserializer()];
    const result = await this.wasm.callWasmExport(
      'test_threads',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async commonInitSlabAllocator(circuitSize: number): Promise<void> {
    const inArgs = [circuitSize].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'common_init_slab_allocator',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async acirGetCircuitSizes(
    constraintSystemBuf: Uint8Array,
    recursive: boolean,
    honkRecursion: boolean,
  ): Promise<[number, number]> {
    const inArgs = [constraintSystemBuf, recursive, honkRecursion].map(serializeBufferable);
    const outTypes: OutputType[] = [NumberDeserializer(), NumberDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_get_circuit_sizes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  async acirNewAcirComposer(sizeHint: number): Promise<Ptr> {
    const inArgs = [sizeHint].map(serializeBufferable);
    const outTypes: OutputType[] = [Ptr];
    const result = await this.wasm.callWasmExport(
      'acir_new_acir_composer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirDeleteAcirComposer(acirComposerPtr: Ptr): Promise<void> {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'acir_delete_acir_composer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async acirInitProvingKey(acirComposerPtr: Ptr, constraintSystemBuf: Uint8Array, recursive: boolean): Promise<void> {
    const inArgs = [acirComposerPtr, constraintSystemBuf, recursive].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'acir_init_proving_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async acirCreateProof(
    acirComposerPtr: Ptr,
    constraintSystemBuf: Uint8Array,
    recursive: boolean,
    witnessBuf: Uint8Array,
  ): Promise<Uint8Array> {
    const inArgs = [acirComposerPtr, constraintSystemBuf, recursive, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_create_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirProveAndVerifyUltraHonk(constraintSystemBuf: Uint8Array, witnessBuf: Uint8Array): Promise<boolean> {
    const inArgs = [constraintSystemBuf, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_prove_and_verify_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirProveAndVerifyMegaHonk(constraintSystemBuf: Uint8Array, witnessBuf: Uint8Array): Promise<boolean> {
    const inArgs = [constraintSystemBuf, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_prove_and_verify_mega_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirProveAztecClient(ivcInputsBuf: Uint8Array): Promise<[Uint8Array, Uint8Array]> {
    const inArgs = [ivcInputsBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer(), BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_prove_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  async acirVerifyAztecClient(proofBuf: Uint8Array, vkBuf: Uint8Array): Promise<boolean> {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_verify_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirLoadVerificationKey(acirComposerPtr: Ptr, vkBuf: Uint8Array): Promise<void> {
    const inArgs = [acirComposerPtr, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'acir_load_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async acirInitVerificationKey(acirComposerPtr: Ptr): Promise<void> {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = await this.wasm.callWasmExport(
      'acir_init_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  async acirGetVerificationKey(acirComposerPtr: Ptr): Promise<Uint8Array> {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_get_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirGetProvingKey(acirComposerPtr: Ptr, acirVec: Uint8Array, recursive: boolean): Promise<Uint8Array> {
    const inArgs = [acirComposerPtr, acirVec, recursive].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_get_proving_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirVerifyProof(acirComposerPtr: Ptr, proofBuf: Uint8Array): Promise<boolean> {
    const inArgs = [acirComposerPtr, proofBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_verify_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirGetSolidityVerifier(acirComposerPtr: Ptr): Promise<string> {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [StringDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_get_solidity_verifier',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirHonkSolidityVerifier(proofBuf: Uint8Array, vkBuf: Uint8Array): Promise<string> {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [StringDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_honk_solidity_verifier',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirSerializeProofIntoFields(
    acirComposerPtr: Ptr,
    proofBuf: Uint8Array,
    numInnerPublicInputs: number,
  ): Promise<Fr[]> {
    const inArgs = [acirComposerPtr, proofBuf, numInnerPublicInputs].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = await this.wasm.callWasmExport(
      'acir_serialize_proof_into_fields',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirSerializeVerificationKeyIntoFields(acirComposerPtr: Ptr): Promise<[Fr[], Fr]> {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr), Fr];
    const result = await this.wasm.callWasmExport(
      'acir_serialize_verification_key_into_fields',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  async acirProveUltraHonk(acirVec: Uint8Array, witnessVec: Uint8Array): Promise<Uint8Array> {
    const inArgs = [acirVec, witnessVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_prove_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirProveUltraKeccakHonk(acirVec: Uint8Array, witnessVec: Uint8Array): Promise<Uint8Array> {
    const inArgs = [acirVec, witnessVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_prove_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirVerifyUltraHonk(proofBuf: Uint8Array, vkBuf: Uint8Array): Promise<boolean> {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_verify_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirVerifyUltraKeccakHonk(proofBuf: Uint8Array, vkBuf: Uint8Array): Promise<boolean> {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_verify_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirWriteVkUltraHonk(acirVec: Uint8Array): Promise<Uint8Array> {
    const inArgs = [acirVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_write_vk_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirWriteVkUltraKeccakHonk(acirVec: Uint8Array): Promise<Uint8Array> {
    const inArgs = [acirVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_write_vk_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirProofAsFieldsUltraHonk(proofBuf: Uint8Array): Promise<Fr[]> {
    const inArgs = [proofBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = await this.wasm.callWasmExport(
      'acir_proof_as_fields_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirVkAsFieldsUltraHonk(vkBuf: Uint8Array): Promise<Fr[]> {
    const inArgs = [vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = await this.wasm.callWasmExport(
      'acir_vk_as_fields_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirVkAsFieldsMegaHonk(vkBuf: Uint8Array): Promise<Fr[]> {
    const inArgs = [vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = await this.wasm.callWasmExport(
      'acir_vk_as_fields_mega_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  async acirGatesAztecClient(ivcInputsBuf: Uint8Array): Promise<Uint8Array> {
    const inArgs = [ivcInputsBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = await this.wasm.callWasmExport(
      'acir_gates_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }
}
export class BarretenbergApiSync {
  constructor(protected wasm: BarretenbergWasm) {}

  pedersenCommit(inputsBuffer: Fr[], ctxIndex: number): Point {
    const inArgs = [inputsBuffer, ctxIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Point];
    const result = this.wasm.callWasmExport(
      'pedersen_commit',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  pedersenHash(inputsBuffer: Fr[], hashIndex: number): Fr {
    const inArgs = [inputsBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'pedersen_hash',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  pedersenHashes(inputsBuffer: Fr[], hashIndex: number): Fr {
    const inArgs = [inputsBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'pedersen_hashes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  pedersenHashBuffer(inputBuffer: Uint8Array, hashIndex: number): Fr {
    const inArgs = [inputBuffer, hashIndex].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'pedersen_hash_buffer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  poseidon2Hash(inputsBuffer: Fr[]): Fr {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'poseidon2_hash',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  poseidon2Hashes(inputsBuffer: Fr[]): Fr {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'poseidon2_hashes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  poseidon2Permutation(inputsBuffer: Fr[]): Fr[] {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = this.wasm.callWasmExport(
      'poseidon2_permutation',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  poseidon2HashAccumulate(inputsBuffer: Fr[]): Fr {
    const inArgs = [inputsBuffer].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'poseidon2_hash_accumulate',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  blake2s(data: Uint8Array): Buffer32 {
    const inArgs = [data].map(serializeBufferable);
    const outTypes: OutputType[] = [Buffer32];
    const result = this.wasm.callWasmExport(
      'blake2s',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  blake2sToField(data: Uint8Array): Fr {
    const inArgs = [data].map(serializeBufferable);
    const outTypes: OutputType[] = [Fr];
    const result = this.wasm.callWasmExport(
      'blake2s_to_field_',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  aesEncryptBufferCbc(input: Uint8Array, iv: Uint8Array, key: Uint8Array, length: number): Uint8Array {
    const inArgs = [input, iv, key, length].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'aes_encrypt_buffer_cbc',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  aesDecryptBufferCbc(input: Uint8Array, iv: Uint8Array, key: Uint8Array, length: number): Uint8Array {
    const inArgs = [input, iv, key, length].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'aes_decrypt_buffer_cbc',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  srsInitSrs(pointsBuf: Uint8Array, numPoints: number, g2PointBuf: Uint8Array): void {
    const inArgs = [pointsBuf, numPoints, g2PointBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'srs_init_srs',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  srsInitGrumpkinSrs(pointsBuf: Uint8Array, numPoints: number): void {
    const inArgs = [pointsBuf, numPoints].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'srs_init_grumpkin_srs',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  examplesSimpleCreateAndVerifyProof(): boolean {
    const inArgs = [].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'examples_simple_create_and_verify_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  testThreads(threads: number, iterations: number): number {
    const inArgs = [threads, iterations].map(serializeBufferable);
    const outTypes: OutputType[] = [NumberDeserializer()];
    const result = this.wasm.callWasmExport(
      'test_threads',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  commonInitSlabAllocator(circuitSize: number): void {
    const inArgs = [circuitSize].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'common_init_slab_allocator',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  acirGetCircuitSizes(constraintSystemBuf: Uint8Array, recursive: boolean, honkRecursion: boolean): [number, number] {
    const inArgs = [constraintSystemBuf, recursive, honkRecursion].map(serializeBufferable);
    const outTypes: OutputType[] = [NumberDeserializer(), NumberDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_get_circuit_sizes',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  acirNewAcirComposer(sizeHint: number): Ptr {
    const inArgs = [sizeHint].map(serializeBufferable);
    const outTypes: OutputType[] = [Ptr];
    const result = this.wasm.callWasmExport(
      'acir_new_acir_composer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirDeleteAcirComposer(acirComposerPtr: Ptr): void {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'acir_delete_acir_composer',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  acirInitProvingKey(acirComposerPtr: Ptr, constraintSystemBuf: Uint8Array, recursive: boolean): void {
    const inArgs = [acirComposerPtr, constraintSystemBuf, recursive].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'acir_init_proving_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  acirCreateProof(
    acirComposerPtr: Ptr,
    constraintSystemBuf: Uint8Array,
    recursive: boolean,
    witnessBuf: Uint8Array,
  ): Uint8Array {
    const inArgs = [acirComposerPtr, constraintSystemBuf, recursive, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_create_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirProveAndVerifyUltraHonk(constraintSystemBuf: Uint8Array, witnessBuf: Uint8Array): boolean {
    const inArgs = [constraintSystemBuf, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_prove_and_verify_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirProveAndVerifyMegaHonk(constraintSystemBuf: Uint8Array, witnessBuf: Uint8Array): boolean {
    const inArgs = [constraintSystemBuf, witnessBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_prove_and_verify_mega_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirProveAztecClient(ivcInputsBuf: Uint8Array): [Uint8Array, Uint8Array] {
    const inArgs = [ivcInputsBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer(), BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_prove_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  acirVerifyAztecClient(proofBuf: Uint8Array, vkBuf: Uint8Array): boolean {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_verify_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirLoadVerificationKey(acirComposerPtr: Ptr, vkBuf: Uint8Array): void {
    const inArgs = [acirComposerPtr, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'acir_load_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  acirInitVerificationKey(acirComposerPtr: Ptr): void {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [];
    const result = this.wasm.callWasmExport(
      'acir_init_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return;
  }

  acirGetVerificationKey(acirComposerPtr: Ptr): Uint8Array {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_get_verification_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirGetProvingKey(acirComposerPtr: Ptr, acirVec: Uint8Array, recursive: boolean): Uint8Array {
    const inArgs = [acirComposerPtr, acirVec, recursive].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_get_proving_key',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirVerifyProof(acirComposerPtr: Ptr, proofBuf: Uint8Array): boolean {
    const inArgs = [acirComposerPtr, proofBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_verify_proof',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirGetSolidityVerifier(acirComposerPtr: Ptr): string {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [StringDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_get_solidity_verifier',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirHonkSolidityVerifier(proofBuf: Uint8Array, vkBuf: Uint8Array): string {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [StringDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_honk_solidity_verifier',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirSerializeProofIntoFields(acirComposerPtr: Ptr, proofBuf: Uint8Array, numInnerPublicInputs: number): Fr[] {
    const inArgs = [acirComposerPtr, proofBuf, numInnerPublicInputs].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = this.wasm.callWasmExport(
      'acir_serialize_proof_into_fields',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirSerializeVerificationKeyIntoFields(acirComposerPtr: Ptr): [Fr[], Fr] {
    const inArgs = [acirComposerPtr].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr), Fr];
    const result = this.wasm.callWasmExport(
      'acir_serialize_verification_key_into_fields',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out as any;
  }

  acirProveUltraHonk(acirVec: Uint8Array, witnessVec: Uint8Array): Uint8Array {
    const inArgs = [acirVec, witnessVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_prove_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirProveUltraKeccakHonk(acirVec: Uint8Array, witnessVec: Uint8Array): Uint8Array {
    const inArgs = [acirVec, witnessVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_prove_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirVerifyUltraHonk(proofBuf: Uint8Array, vkBuf: Uint8Array): boolean {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_verify_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirVerifyUltraKeccakHonk(proofBuf: Uint8Array, vkBuf: Uint8Array): boolean {
    const inArgs = [proofBuf, vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BoolDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_verify_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirWriteVkUltraHonk(acirVec: Uint8Array): Uint8Array {
    const inArgs = [acirVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_write_vk_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirWriteVkUltraKeccakHonk(acirVec: Uint8Array): Uint8Array {
    const inArgs = [acirVec].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_write_vk_ultra_keccak_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirProofAsFieldsUltraHonk(proofBuf: Uint8Array): Fr[] {
    const inArgs = [proofBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = this.wasm.callWasmExport(
      'acir_proof_as_fields_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirVkAsFieldsUltraHonk(vkBuf: Uint8Array): Fr[] {
    const inArgs = [vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = this.wasm.callWasmExport(
      'acir_vk_as_fields_ultra_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirVkAsFieldsMegaHonk(vkBuf: Uint8Array): Fr[] {
    const inArgs = [vkBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [VectorDeserializer(Fr)];
    const result = this.wasm.callWasmExport(
      'acir_vk_as_fields_mega_honk',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }

  acirGatesAztecClient(ivcInputsBuf: Uint8Array): Uint8Array {
    const inArgs = [ivcInputsBuf].map(serializeBufferable);
    const outTypes: OutputType[] = [BufferDeserializer()];
    const result = this.wasm.callWasmExport(
      'acir_gates_aztec_client',
      inArgs,
      outTypes.map(t => t.SIZE_IN_BYTES),
    );
    const out = result.map((r, i) => outTypes[i].fromBuffer(r));
    return out[0];
  }
}
