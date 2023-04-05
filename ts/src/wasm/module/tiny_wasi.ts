// Copyright (c) 2022 QR Date

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

import * as crypto from 'crypto';

export class TinyWASI {
  private instance?: WebAssembly.Instance = undefined;

  private WASI_ERRNO_SUCCESS = 0;
  private WASI_ERRNO_BADF = 8;
  private WASI_ERRNO_NOSYS = 52;
  private WASI_ERRNO_INVAL = 28;

  private WASI_FILETYPE_CHARACTER_DEVICE = 2;

  private WASI_RIGHTS_FD_SYNC = 1 << 4;
  private WASI_RIGHTS_FD_WRITE = 1 << 6;
  private WASI_RIGHTS_FD_FILESTAT_GET = 1 << 21;

  private WASI_FDFLAGS_APPEND = 1 << 0;

  private nameSpaces: { [key: string]: { [key: string]: CallableFunction | undefined } } = {
    wasi_snapshot_preview1: {
      args_get: undefined, // ((param i32 i32) (result i32))
      args_sizes_get: undefined, // ((param i32 i32) (result i32))

      clock_res_get: this.clock_res_get, // ((param i32 i32) (result i32))
      clock_time_get: this.clock_time_get, // ((param i32 i64 i32) (result i32))

      environ_get: undefined, // ((param i32 i32) (result i32))
      environ_sizes_get: undefined, // ((param i32 i32) (result i32))

      fd_advise: undefined, // ((param i32 i64 i64 i32) (result i32))
      fd_allocate: undefined, // ((param i32 i64 i64) (result i32))
      fd_close: undefined, // ((param i32) (result i32))
      fd_datasync: undefined, // ((param i32) (result i32))
      fd_fdstat_get: this.fd_fdstat_get, // ((param i32 i32) (result i32))
      fd_fdstat_set_flags: undefined, // ((param i32 i32) (result i32))
      fd_fdstat_set_rights: undefined, // ((param i32 i64 i64) (result i32))
      fd_filestat_get: undefined, // ((param i32 i32) (result i32))
      fd_filestat_set_size: undefined, // ((param i32 i64) (result i32))
      fd_filestat_set_times: undefined, // ((param i32 i64 i64 i32) (result i32))
      fd_pread: undefined, // ((param i32 i32 i32 i64 i32) (result i32))
      fd_prestat_dir_name: undefined, // ((param i32 i32 i32) (result i32))
      fd_prestat_get: undefined, // ((param i32 i32) (result i32))
      fd_pwrite: undefined, // ((param i32 i32 i32 i64 i32) (result i32))
      fd_read: undefined, // ((param i32 i32 i32 i32) (result i32))
      fd_readdir: undefined, // ((param i32 i32 i32 i64 i32) (result i32))
      fd_renumber: undefined, // ((param i32 i32) (result i32))
      fd_seek: undefined, // ((param i32 i64 i32 i32) (result i32))
      fd_sync: undefined, // ((param i32) (result i32))
      fd_tell: undefined, // ((param i32 i32) (result i32))
      fd_write: this.fd_write, // ((param i32 i32 i32 i32) (result i32))

      path_create_directory: undefined, // ((param i32 i32 i32) (result i32))
      path_filestat_get: undefined, // ((param i32 i32 i32 i32 i32) (result i32))
      path_filestat_set_times: undefined, // ((param i32 i32 i32 i32 i64 i64 i32) (result i32))
      path_link: undefined, // ((param i32 i32 i32 i32 i32 i32 i32) (result i32))
      path_open: undefined, // ((param i32 i32 i32 i32 i32 i64 i64 i32 i32) (result i32))
      path_readlink: undefined, // ((param i32 i32 i32 i32 i32 i32) (result i32))
      path_remove_directory: undefined, // ((param i32 i32 i32) (result i32))
      path_rename: undefined, // ((param i32 i32 i32 i32 i32 i32) (result i32))
      path_symlink: undefined, // ((param i32 i32 i32 i32 i32) (result i32))
      path_unlink_file: undefined, // ((param i32 i32 i32) (result i32))

      poll_oneoff: undefined, // ((param i32 i32 i32 i32) (result i32))

      proc_exit: undefined, // ((param i32))
      proc_raise: undefined, // ((param i32) (result i32))

      random_get: this.random_get, // ((param i32 i32) (result i32))

      sched_yield: undefined, // ((result i32))

      sock_recv: undefined, // ((param i32 i32 i32 i32 i32 i32) (result i32))
      sock_send: undefined, // ((param i32 i32 i32 i32 i32) (result i32))
      sock_shutdown: undefined, // ((param i32 i32) (result i32))
    },
  };

  constructor(trace?: boolean) {
    for (const ns of Object.keys(this.nameSpaces)) {
      const nameSpace = this.nameSpaces[ns];

      for (const fn of Object.keys(nameSpace)) {
        let func = nameSpace[fn] || this.nosys(fn);

        func = func.bind(this);

        if (trace) func = this.trace(fn, func).bind(this);

        nameSpace[fn] = func;
      }
    }
  }

  initialize(instance: WebAssembly.Instance) {
    this.instance = instance;

    const initialize = instance.exports._initialize as CallableFunction;
    initialize();
  }

  get imports(): WebAssembly.Imports {
    return this.nameSpaces as WebAssembly.Imports;
  }

  private getMemory(): WebAssembly.Memory {
    if (this.instance) return this.instance.exports.memory as WebAssembly.Memory;
    else throw new Error('Attempt to access instance before initialisation!');
  }

  private getDataView(): DataView {
    if (this.instance) return new DataView((this.instance.exports.memory as WebAssembly.Memory).buffer);
    else throw new Error('Attempt to access instance before initialisation!');
  }

  private trace(name: string, origFunc: CallableFunction): CallableFunction {
    return (...args: number[]): number => {
      const result = origFunc(...args);
      console.log(`Trace: ${name}(${args.toString()}) -> ${result}`);
      return result;
    };
  }

  private nosys(name: string): CallableFunction {
    return (...args: number[]): number => {
      console.error(`Unimplemented call to ${name}(${args.toString()})`);
      return this.WASI_ERRNO_NOSYS;
    };
  }

  private clock_res_get(id: number, resOut: number): number {
    if (id !== 0) return this.WASI_ERRNO_INVAL;

    const view = this.getDataView();

    view.setUint32(resOut, 1000000.0 % 0x100000000, true);
    view.setUint32(resOut + 4, 1000000.0 / 0x100000000, true);

    return this.WASI_ERRNO_SUCCESS;
  }

  private clock_time_get(id: number, precision: number, timeOut: number): number {
    if (id !== 0) return this.WASI_ERRNO_INVAL;

    const view = this.getDataView();

    const now = new Date().getTime();

    view.setUint32(timeOut, (now * 1000000.0) % 0x100000000, true);
    view.setUint32(timeOut + 4, (now * 1000000.0) / 0x100000000, true);

    return this.WASI_ERRNO_SUCCESS;
  }

  private fd_fdstat_get(fd: number, fdstat: number): number {
    if (fd > 2) return this.WASI_ERRNO_BADF;

    const view = this.getDataView();

    view.setUint8(fdstat, this.WASI_FILETYPE_CHARACTER_DEVICE);
    view.setUint16(fdstat + 2, this.WASI_FDFLAGS_APPEND, true);
    view.setUint16(
      fdstat + 8,
      this.WASI_RIGHTS_FD_SYNC | this.WASI_RIGHTS_FD_WRITE | this.WASI_RIGHTS_FD_FILESTAT_GET,
      true,
    );
    view.setUint16(fdstat + 16, 0, true);

    return this.WASI_ERRNO_SUCCESS;
  }

  private fd_write(fd: number, iovs: number, iovsLen: number, nwritten: number): number {
    if (fd > 2) return this.WASI_ERRNO_BADF;

    const view = this.getDataView();
    const memory = this.getMemory();

    const buffers: Uint8Array[] = [];

    for (let i = 0; i < iovsLen; i++) {
      const iov = iovs + i * 8;
      const offset = view.getUint32(iov, true);
      const len = view.getUint32(iov + 4, true);

      buffers.push(new Uint8Array(memory.buffer, offset, len));
    }

    const length = buffers.reduce((s, b) => s + b.length, 0);

    const buffer = new Uint8Array(length);
    let offset = 0;

    buffers.forEach(b => {
      buffer.set(b, offset);
      offset += b.length;
    });

    const string = new TextDecoder('utf-8').decode(buffer).replace(/\n$/, '');

    if (fd === 1) console.log(string);
    else console.error(string);

    view.setUint32(nwritten, buffer.length, true);

    return this.WASI_ERRNO_SUCCESS;
  }

  private random_get(pointer: number, size: number): number {
    const memory = this.getMemory();

    const buffer = new Uint8Array(memory.buffer, pointer, size);

    crypto.randomFillSync(buffer);

    return this.WASI_ERRNO_SUCCESS;
  }
}
