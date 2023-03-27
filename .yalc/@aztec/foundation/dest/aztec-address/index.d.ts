/// <reference types="node" resolution-mode="require"/>
export declare class AztecAddress {
    private buffer;
    static SIZE_IN_BYTES: number;
    static ZERO: AztecAddress;
    constructor(buffer: Buffer);
    static fromString(address: string): AztecAddress;
    static random(): AztecAddress;
    equals(rhs: AztecAddress): boolean;
    toBuffer(): Buffer;
    toString(): string;
    toShortString(): string;
}
//# sourceMappingURL=index.d.ts.map