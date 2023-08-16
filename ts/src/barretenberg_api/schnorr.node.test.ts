import { expect } from 'chai';
import { TextEncoder } from 'util';
import { Buffer128, Buffer32, Fr, Point } from '../types/index.js';
import { BarretenbergApiSync } from './index.js';
import { newBarretenbergApiSync } from '../factory/index.js';

describe('schnorr', () => {
  const msg = Buffer.from(new TextEncoder().encode('The quick brown dog jumped over the lazy fox.'));
  let api: BarretenbergApiSync;

  before(async () => {
    api = await newBarretenbergApiSync();
    api.pedersenInit();
  });

  after(async () => {
    await api.destroy();
  });

  it('should verify signature', () => {
    const pk = Fr.fromBuffer(
      new Uint8Array([
        0x0b, 0x9b, 0x3a, 0xde, 0xe6, 0xb3, 0xd8, 0x1b, 0x28, 0xa0, 0x88, 0x6b, 0x2a, 0x84, 0x15, 0xc7, 0xda, 0x31,
        0x29, 0x1a, 0x5e, 0x96, 0xbb, 0x7a, 0x56, 0x63, 0x9e, 0x17, 0x7d, 0x30, 0x1b, 0xeb,
      ]),
    );
    const pubKey = api.schnorrComputePublicKey(pk);
    const [s, e] = api.schnorrConstructSignature(msg, pk);
    const verified = api.schnorrVerifySignature(msg, pubKey, s, e);

    expect(verified).to.be.true;
  });

  it('public key negation should work', () => {
    const publicKeyStr =
      '0x164f01b1011a1b292217acf53eef4d74f625f6e9bd5edfdb74c56fd81aafeebb21912735f9266a3719f61c1eb747ddee0cac9917f5c807485d356709b529b62c';
    const publicKey = Point.fromString(publicKeyStr);
    const expectedInvertedStr =
      '0x164f01b1011a1b292217acf53eef4d74f625f6e9bd5edfdb74c56fd81aafeebb0ed3273ce80b35f29e5a2997ca397a6f1b874f3083f16948e6ac8e8a3ad649d5';
    const expectedInverted = Point.fromString(expectedInvertedStr);
    const negatedPublicKey = api.schnorrNegatePublicKey(publicKey);
    expect(negatedPublicKey.equals(expectedInverted)).to.be.true;
    expect(api.schnorrNegatePublicKey(negatedPublicKey).equals(publicKey)).to.be.true;
  });

  it('should create + verify multi signature', () => {
    const numSigners = 7;
    const pks = [...Array(numSigners)].map(() => Fr.random());
    const pubKeys = pks.map(pk => api.schnorrMultisigCreateMultisigPublicKey(pk));
    const roundOnePublicOutputs: Buffer128[] = [];
    const roundOnePrivateOutputs: Buffer128[] = [];
    for (let i = 0; i < numSigners; ++i) {
      const [publicOutput, privateOutput] = api.schnorrMultisigConstructSignatureRound1();
      roundOnePublicOutputs.push(publicOutput);
      roundOnePrivateOutputs.push(privateOutput);
    }
    const roundTwoOutputs = pks.map(
      (pk, i) =>
        api.schnorrMultisigConstructSignatureRound2(
          msg,
          pk,
          roundOnePrivateOutputs[i],
          pubKeys,
          roundOnePublicOutputs,
        )[0],
    );
    const [s, e] = api.schnorrMultisigCombineSignatures(msg, pubKeys, roundOnePublicOutputs, roundTwoOutputs)!;
    const [combinedKey] = api.schnorrMultisigValidateAndCombineSignerPubkeys(pubKeys);
    expect(combinedKey).not.to.equal(Buffer.alloc(64));
    const verified = api.schnorrVerifySignature(msg, combinedKey, s, e);
    expect(verified).to.be.true;
  });

  it('should identify invalid multi signature', () => {
    const pks = [...Array(3)].map(() => Fr.random());
    const pubKeys = pks.map(pk => api.schnorrMultisigCreateMultisigPublicKey(pk));
    const [combinedKey] = api.schnorrMultisigValidateAndCombineSignerPubkeys(pubKeys);
    const verified = api.schnorrVerifySignature(msg, combinedKey, Buffer32.random(), Buffer32.random());
    expect(verified).to.be.false;
  });

  it('should not construct invalid multi signature', () => {
    const numSigners = 7;
    const pks = [...Array(numSigners)].map(() => Fr.random());
    const pubKeys = pks.map(pk => api.schnorrMultisigCreateMultisigPublicKey(pk));
    const roundOnePublicOutputs: Buffer128[] = [];
    const roundOnePrivateOutputs: Buffer128[] = [];
    for (let i = 0; i < numSigners; ++i) {
      const [publicOutput, privateOutput] = api.schnorrMultisigConstructSignatureRound1();
      roundOnePublicOutputs.push(publicOutput);
      roundOnePrivateOutputs.push(privateOutput);
    }
    const roundTwoOutputs = pks.map(
      (pk, i) =>
        api.schnorrMultisigConstructSignatureRound2(
          msg,
          pk,
          roundOnePrivateOutputs[i],
          pubKeys,
          roundOnePublicOutputs,
        )[0],
    );
    expect(
      api.schnorrMultisigCombineSignatures(
        msg,
        pubKeys.slice(0, -1),
        roundOnePublicOutputs.slice(0, -1),
        roundTwoOutputs.slice(0, -1),
      )[2],
    ).to.be.false;
    const invalidOutputs = [...roundTwoOutputs];
    invalidOutputs[1] = api.schnorrMultisigConstructSignatureRound2(
      msg,
      pks[2],
      roundOnePrivateOutputs[1],
      pubKeys,
      roundOnePublicOutputs,
    )[0];
    expect(api.schnorrMultisigCombineSignatures(msg, pubKeys, roundOnePublicOutputs, invalidOutputs)[2]).to.be.false;
    const duplicatedOutputs = [...roundTwoOutputs];
    duplicatedOutputs[1] = roundTwoOutputs[2];
    expect(api.schnorrMultisigCombineSignatures(msg, pubKeys, roundOnePublicOutputs, duplicatedOutputs)[2]).to.be.false;
  });

  it('should not create combined key from public keys containing invalid key', () => {
    const pks = [...Array(5)].map(() => Fr.random());
    const pubKeys = pks.map(pk => api.schnorrMultisigCreateMultisigPublicKey(pk));

    // not a valid point
    {
      pubKeys[1] = new Buffer128(Buffer.alloc(128));
      expect(api.schnorrMultisigValidateAndCombineSignerPubkeys(pubKeys)[1]).to.be.false;
    }

    // contains duplicates
    {
      pubKeys[1] = pubKeys[2];
      expect(api.schnorrMultisigValidateAndCombineSignerPubkeys(pubKeys)[1]).to.be.false;
    }
  });
});
