/// <reference types="node" resolution-mode="require"/>
declare abstract class Field {
    static SIZE_IN_BYTES: number;
    private buffer;
    constructor(input: Buffer | number);
    abstract maxValue(): bigint;
    toString(): string;
    toBuffer(): Buffer;
}
export declare class Fr extends Field {
    /**
     * Maximum represntable value in a field is the curve prime minus one.
     * @returns 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000n
     */
    maxValue(): bigint;
    static fromBuffer(buffer: Buffer): Fr;
}
export declare class Fq extends Field {
    /**
     * Maximum represntable vaue in a field is the curve prime minus one.
     * TODO: Find out actual max value for Fq.
     * @returns 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000n
     */
    maxValue(): bigint;
    static fromBuffer(buffer: Buffer): Fq;
}
export {};
//# sourceMappingURL=fields.d.ts.map