import { BufferReader } from './buffer_reader.js';

export interface OutputType<T = any> {
  SIZE_IN_BYTES?: number;
  fromBuffer: (b: Uint8Array | BufferReader) => T;
}

export function NumberDeserializer(): OutputType {
  return {
    fromBuffer: (buf: Uint8Array | BufferReader) => {
      const reader = BufferReader.asReader(buf);
      return reader.readNumber();
    },
  };
}

export function VectorDeserializer<T>(t: OutputType<T>): OutputType {
  return {
    fromBuffer: (buf: Uint8Array | BufferReader) => {
      const reader = BufferReader.asReader(buf);
      return reader.readVector(t);
    },
  };
}

export function BufferDeserializer(): OutputType {
  return {
    fromBuffer: (buf: Uint8Array | BufferReader) => {
      const reader = BufferReader.asReader(buf);
      return reader.readBuffer();
    },
  };
}
