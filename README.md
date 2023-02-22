## Barretenberg, an optimized elliptic curve library for the bn128 curve, and PLONK SNARK prover

**This code is highly experimental, use at your own risk!**

### Dependencies

- cmake >= 3.24
- clang >= 10 or gcc >= 10
- clang-format
- libomp (if multithreading is required. Multithreading can be disabled using the compiler flag `-DMULTITHREADING 0`)
- wasm-opt (part of the [Binaryen](https://github.com/WebAssembly/binaryen) toolkit)

### Installing openMP (Linux)

```
RUN git clone -b release/10.x --depth 1 https://github.com/llvm/llvm-project.git \
  && cd llvm-project && mkdir build-openmp && cd build-openmp \
  && cmake ../openmp -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLIBOMP_ENABLE_SHARED=OFF \
  && cmake --build . --parallel \
  && cmake --build . --parallel --target install \
  && cd ../.. && rm -rf llvm-project
```

### Getting started

Run the bootstrap script. (The bootstrap script will build both the native and wasm versions of barretenberg)

```
cd cpp
./bootstrap.sh
```

### Parallelise the build

Use the `--parallel` option to `cmake --build <path>` to parallelize builds. This is roughly equivalent to `make -j$(nproc)` but is more portable.

### Formatting

Code is formatted using `clang-format` and the `./cpp/format.sh` script which is called via a git pre-commit hook.
If you've installed the C++ Vscode extension you should configure it to format on save.

### Testing

Each module has its own tests. e.g. To build and run `ecc` tests:

```
cmake --build . --parallel --target ecc_tests
./bin/ecc_tests
```

A shorthand for the above is:

```
cmake --build . --parallel --target run_ecc_tests
```

Running the entire suite of tests using `ctest`:

```
cmake --build . --parallel --target test
```

You can run specific tests, e.g.

```
./bin/ecc_tests --gtest_filter=scalar_multiplication.*
```

### Benchmarks

Some modules have benchmarks. The build targets are named `<module_name>_bench`. To build and run, for example `ecc` benchmarks.

```
cmake --build . --parallel --target ecc_bench
./src/aztec/ecc/ecc_bench
```

A shorthand for the above is:

```
cmake --build . --parallel --target run_ecc_bench
```

### CMake Build Options

CMake can be passed various build options on its command line:

- `-DCMAKE_BUILD_TYPE=Debug | Release | RelWithAssert`: Build types.
- `-DDISABLE_ASM=ON | OFF`: Enable/disable x86 assembly.
- `-DDISABLE_ADX=ON | OFF`: Enable/disable ADX assembly instructions (for older cpu support).
- `-DMULTITHREADING=ON | OFF`: Enable/disable multithreading using OpenMP.
- `-DTESTING=ON | OFF`: Enable/disable building of tests.
- `-DBENCHMARK=ON | OFF`: Enable/disable building of benchmarks.
- `-DTOOLCHAIN=<filename in ./cmake/toolchains>`: Use one of the preconfigured toolchains.
- `-DFUZZING=ON | OFF`: Enable building various fuzzers.

### WASM build

To build:

```
mkdir build-wasm && cd build-wasm
cmake -DTOOLCHAIN=wasm-linux-clang ..
cmake --build . --parallel --target barretenberg.wasm
```

The resulting wasm binary will be at `./build-wasm/bin/barretenberg.wasm`.

To run the tests, you'll need to install `wasmtime`.

```
curl https://wasmtime.dev/install.sh -sSf | bash
```

Tests can be built and run like:

```
cmake --build . --parallel --target ecc_tests
wasmtime --dir=.. ./bin/ecc_tests
```

### Fuzzing build

For detailed instructions look in cpp/docs/Fuzzing.md

To build:
```
mkdir build-fuzzing && cd build-fuzzing
cmake -DTOOLCHAIN=x86_64-linux-clang -DFUZZING=ON ..
cmake --build . --parallel
```
Fuzzing build turns off building tests and benchmarks, since they are incompatible with libfuzzer interface.

To turn on address sanitizer add `-DADDRESS_SANITIZER=ON`. Note that address sanitizer can be used to explore crashes.
Sometimes you might have to specify the address of llvm-symbolizer. You have to do it with `export ASAN_SYMBOLIZER_PATH=<PATH_TO_SYMBOLIZER>`.
For undefined behaviour sanitizer `-DUNDEFINED_BEHAVIOUR_SANITIZER=ON`.
Note that the fuzzer can be orders of magnitude slower with ASan (2-3x slower) or UBSan on, so it is best to run a non-sanitized build first, minimize the testcase and then run it for a bit of time with sanitizers.

### Test coverage build

To build:
```
mkdir build-coverage && cd build-coverage
cmake -DTOOLCHAIN=x86_64-linux-clang -DCOVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel
```

Then run tests (on the mainframe always use taskset and nice to limit your influence on the server. Profiling instrumentation is very heavy):
```
taskset 0xffffff nice -n10 make test
```

And generate report:
```
make create_full_coverage_report
```

The report will land in the build directory in the all_test_coverage_report directory.

Alternatively you can build separate test binaries, e.g. honk_tests or numeric_tests and run **make test** just for them or even just for a single test. Then the report will just show coverage for those binaries

### VS Code configuration
A default configuration for VS Code is provided by the file [`barretenberg.code-workspace`](barretenberg.code-workspace). These settings can be overridden by placing configuration files in `.vscode/`.
