import { numToUInt32BE } from '../serialize/index.js';
class Field {
    constructor(input) {
        if (Buffer.isBuffer(input)) {
            if (input.length != Field.SIZE_IN_BYTES) {
                throw new Error(`Unexpected buffer size ${input.length} (expected ${Field.SIZE_IN_BYTES} bytes)`);
            }
            this.buffer = input;
        }
        else {
            if (BigInt(input) > this.maxValue()) {
                throw new Error(`Input value ${input} too large (expected ${this.maxValue()})`);
            }
            this.buffer = numToUInt32BE(input, 32);
        }
    }
    toString() {
        return '0x' + this.buffer.toString('hex');
    }
    toBuffer() {
        return this.buffer;
    }
}
Field.SIZE_IN_BYTES = 32;
export class Fr extends Field {
    /**
     * Maximum represntable value in a field is the curve prime minus one.
     * @returns 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000n
     */
    maxValue() {
        return 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001n - 1n;
    }
    static fromBuffer(buffer) {
        return new this(buffer);
    }
}
export class Fq extends Field {
    /**
     * Maximum represntable vaue in a field is the curve prime minus one.
     * TODO: Find out actual max value for Fq.
     * @returns 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000n
     */
    maxValue() {
        return 0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000001n - 1n;
    }
    static fromBuffer(buffer) {
        return new this(buffer);
    }
}
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiZmllbGRzLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vLi4vc3JjL3ByaW1pdGl2ZXMvZmllbGRzLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiJBQUFBLE9BQU8sRUFBRSxhQUFhLEVBQUUsTUFBTSx1QkFBdUIsQ0FBQztBQUV0RCxNQUFlLEtBQUs7SUFLbEIsWUFBWSxLQUFzQjtRQUNoQyxJQUFJLE1BQU0sQ0FBQyxRQUFRLENBQUMsS0FBSyxDQUFDLEVBQUU7WUFDMUIsSUFBSSxLQUFLLENBQUMsTUFBTSxJQUFJLEtBQUssQ0FBQyxhQUFhLEVBQUU7Z0JBQ3ZDLE1BQU0sSUFBSSxLQUFLLENBQUMsMEJBQTBCLEtBQUssQ0FBQyxNQUFNLGNBQWMsS0FBSyxDQUFDLGFBQWEsU0FBUyxDQUFDLENBQUM7YUFDbkc7WUFDRCxJQUFJLENBQUMsTUFBTSxHQUFHLEtBQUssQ0FBQztTQUNyQjthQUFNO1lBQ0wsSUFBSSxNQUFNLENBQUMsS0FBSyxDQUFDLEdBQUcsSUFBSSxDQUFDLFFBQVEsRUFBRSxFQUFFO2dCQUNuQyxNQUFNLElBQUksS0FBSyxDQUFDLGVBQWUsS0FBSyx3QkFBd0IsSUFBSSxDQUFDLFFBQVEsRUFBRSxHQUFHLENBQUMsQ0FBQzthQUNqRjtZQUNELElBQUksQ0FBQyxNQUFNLEdBQUcsYUFBYSxDQUFDLEtBQUssRUFBRSxFQUFFLENBQUMsQ0FBQztTQUN4QztJQUNILENBQUM7SUFJRCxRQUFRO1FBQ04sT0FBTyxJQUFJLEdBQUcsSUFBSSxDQUFDLE1BQU0sQ0FBQyxRQUFRLENBQUMsS0FBSyxDQUFDLENBQUM7SUFDNUMsQ0FBQztJQUVELFFBQVE7UUFDTixPQUFPLElBQUksQ0FBQyxNQUFNLENBQUM7SUFDckIsQ0FBQzs7QUExQmEsbUJBQWEsR0FBRyxFQUFFLENBQUM7QUE2Qm5DLE1BQU0sT0FBTyxFQUFHLFNBQVEsS0FBSztJQUMzQjs7O09BR0c7SUFDSCxRQUFRO1FBQ04sT0FBTyxtRUFBbUUsR0FBRyxFQUFFLENBQUM7SUFDbEYsQ0FBQztJQUVELE1BQU0sQ0FBQyxVQUFVLENBQUMsTUFBYztRQUM5QixPQUFPLElBQUksSUFBSSxDQUFDLE1BQU0sQ0FBQyxDQUFDO0lBQzFCLENBQUM7Q0FDRjtBQUVELE1BQU0sT0FBTyxFQUFHLFNBQVEsS0FBSztJQUMzQjs7OztPQUlHO0lBQ0gsUUFBUTtRQUNOLE9BQU8sbUVBQW1FLEdBQUcsRUFBRSxDQUFDO0lBQ2xGLENBQUM7SUFFRCxNQUFNLENBQUMsVUFBVSxDQUFDLE1BQWM7UUFDOUIsT0FBTyxJQUFJLElBQUksQ0FBQyxNQUFNLENBQUMsQ0FBQztJQUMxQixDQUFDO0NBQ0YifQ==