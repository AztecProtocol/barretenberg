/// <reference types="node" resolution-mode="require"/>
export declare class EthAddress {
    private buffer;
    static SIZE_IN_BYTES: number;
    static ZERO: EthAddress;
    constructor(buffer: Buffer);
    static fromString(address: string): EthAddress;
    static random(): EthAddress;
    static isAddress(address: string): boolean;
    isZero(): boolean;
    static checkAddressChecksum(address: string): boolean;
    static toChecksumAddress(address: string): string;
    equals(rhs: EthAddress): boolean;
    toString(): string;
    toChecksumString(): string;
    toBuffer(): Buffer;
    toBuffer32(): Buffer;
}
//# sourceMappingURL=index.d.ts.map