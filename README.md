\page README README
## Barretenberg, an optimized elliptic curve library for the bn128 curve, and PLONK SNARK prover

**This code is highly experimental, use at your own risk!**

### Dependencies

- cmake >= 3.24
- Ninja (used by the presets as the default generator)
- clang >= 10 or gcc >= 10
- clang-format
- libomp (if multithreading is required. Multithreading can be disabled using the compiler flag `-DMULTITHREADING 0`)
- wasm-opt (part of the [Binaryen](https://github.com/WebAssembly/binaryen) toolkit)

To install on Ubuntu, run:
```
sudo apt-get install cmake clang clang-format ninja-build binaryen
```

### Installing openMP (Linux)

Install from source:

```
git clone -b release/10.x --depth 1 https://github.com/llvm/llvm-project.git \
  && cd llvm-project && mkdir build-openmp && cd build-openmp \
  && cmake ../openmp -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLIBOMP_ENABLE_SHARED=OFF \
  && cmake --build . --parallel \
  && cmake --build . --parallel --target install \
  && cd ../.. && rm -rf llvm-project
```

Or install from a package manager, on Ubuntu:

```
sudo apt-get install libomp-dev
```

> Note: on a fresh Ubuntu Kinetic installation, installing OpenMP from source yields a `Could NOT find OpenMP_C (missing: OpenMP_omp_LIBRARY) (found version "5.0")` error when trying to build Barretenberg. Installing from apt worked fine.

### Getting started

Run the bootstrap script. (The bootstrap script will build both the native and wasm versions of barretenberg)

```
cd cpp
./bootstrap.sh
```

### Installing

After the project has been built, such as with `./bootstrap.sh`, you can install the library on your system:

```sh
cmake --install build
```

### Formatting

Code is formatted using `clang-format` and the `./cpp/format.sh` script which is called via a git pre-commit hook.
If you've installed the C++ Vscode extension you should configure it to format on save.

### Testing

Each module has its own tests. e.g. To build and run `ecc` tests:

```bash
# Replace the `default` preset with whichever preset you want to use
cmake --build --preset default --target ecc_tests
cd build
./bin/ecc_tests
```

A shorthand for the above is:

```bash
# Replace the `default` preset with whichever preset you want to use
cmake --build --preset default --target run_ecc_tests
```

Running the entire suite of tests using `ctest`:

```bash
cmake --build --preset default --target test
```

You can run specific tests, e.g.

```
./bin/ecc_tests --gtest_filter=scalar_multiplication.*
```

### Benchmarks

Some modules have benchmarks. The build targets are named `<module_name>_bench`. To build and run, for example `ecc` benchmarks.

```bash
# Replace the `default` preset with whichever preset you want to use
cmake --build --preset default --target ecc_bench
cd build
./bin/ecc_bench
```

A shorthand for the above is:

```bash
# Replace the `default` preset with whichever preset you want to use
cmake --build --preset default --target run_ecc_bench
```

### CMake Build Options

CMake can be passed various build options on its command line:

- `-DCMAKE_BUILD_TYPE=Debug | Release | RelWithAssert`: Build types.
- `-DDISABLE_ASM=ON | OFF`: Enable/disable x86 assembly.
- `-DDISABLE_ADX=ON | OFF`: Enable/disable ADX assembly instructions (for older cpu support).
- `-DMULTITHREADING=ON | OFF`: Enable/disable multithreading using OpenMP.
- `-DTESTING=ON | OFF`: Enable/disable building of tests.
- `-DBENCHMARK=ON | OFF`: Enable/disable building of benchmarks.
- `-DFUZZING=ON | OFF`: Enable building various fuzzers.

If you are cross-compiling, you can use a preconfigured toolchain file:

- `-DCMAKE_TOOLCHAIN_FILE=<filename in ./cmake/toolchains>`: Use one of the preconfigured toolchains.

### WASM build

To build:

```bash
cmake --preset wasm
cmake --build --preset wasm --target barretenberg.wasm
```

The resulting wasm binary will be at `./build-wasm/bin/barretenberg.wasm`.

To run the tests, you'll need to install `wasmtime`.

```
curl https://wasmtime.dev/install.sh -sSf | bash
```

Tests can be built and run like:

```bash
cmake --build --preset wasm --target ecc_tests
wasmtime --dir=.. ./bin/ecc_tests
```

To add gtest filter parameters in a wasm context:

```
wasmtime --dir=.. ./bin/ecc_tests run --gtest_filter=filtertext
```

### Fuzzing build

For detailed instructions look in cpp/docs/Fuzzing.md

To build:

```bash
cmake --preset fuzzing
cmake --build --preset fuzzing
```

Fuzzing build turns off building tests and benchmarks, since they are incompatible with libfuzzer interface.

To turn on address sanitizer add `-DADDRESS_SANITIZER=ON`. Note that address sanitizer can be used to explore crashes.
Sometimes you might have to specify the address of llvm-symbolizer. You have to do it with `export ASAN_SYMBOLIZER_PATH=<PATH_TO_SYMBOLIZER>`.
For undefined behaviour sanitizer `-DUNDEFINED_BEHAVIOUR_SANITIZER=ON`.
Note that the fuzzer can be orders of magnitude slower with ASan (2-3x slower) or UBSan on, so it is best to run a non-sanitized build first, minimize the testcase and then run it for a bit of time with sanitizers.

### Test coverage build

To build:

```bash
cmake --preset coverage
cmake --build --preset coverage
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

Alternatively you can build separate test binaries, e.g. honk_tests or numeric_tests and run **make test** just for them or even just for a single test. Then the report will just show coverage for those binaries.

### VS Code configuration

A default configuration for VS Code is provided by the file [`barretenberg.code-workspace`](barretenberg.code-workspace). These settings can be overridden by placing configuration files in `.vscode/`.

### Doxygen

An alternative might be retrofitting https://www.cryptologie.net/article/553/the-code-is-the-specification-introducing-cargo-spec/ to our needs.

Installation: run boostrap_doxygen.sh.

The file ./Doxyfile is a config for doxygen. There are tons of options. For example, you can generate very cool but often uselessly huge diagrams by setting `HAVE_DOT` to `YES`.

To generate the documentation: `doxygen Doxyfile`

To browse the generated documentation, a good starting point is: ./doxydoc/html/index.html

Reference: http://web.evolbio.mpg.de/~boettcher//other/2016/creating_source_graph.html

#### Notes

All words that contain a dot (.) that is not the last character in the word are considered to be file names. If the word is indeed the name of a documented input file, a link will automatically be created to the documentation of that file.

Large delimiters (`\big(`, `\left(`, also the `cases` environment) are not natively supported.


The purpose of this documentation: 
  - The goal of this documentation is to be self-contained and authoritative.
  - It cannot be used to record speculations or planned developments. It is documentation of what's implemented.
  - It should always be accurate.
  - It should use notation that facilitates the documentation being accurate.
  - It should be "direct" in the sense of the reflecting the code. It should not use unnecessary abstractions or notation that that is unnatural to use in code comments.
  - It should remove ambiguity rather than to add it. It is not just another HackMD document.
  - It should _not depend_ on external references except for published or widely-used papers (PlonK; Honk; Bulletproofs). It should not reference things published on HackMD.
  - It may _make_ external references to publicly-available sources, but that should be to resolve potential confusions that a user might face when reconciling differences between sources.

 
There is a global file barretenberg/spec/macros.tex that allows for using latex macros. This is convenient, it encourages standarization of notation across the repo, and it also enables changing that notation with minimal footprint in the diffs.

Grouping can be used to make pages that list classes or pages together even though they're not grouped together in the code.

Call graph and caller graph are different


"Let's repeat that, because it is often overlooked: to document global objects (functions, typedefs, enum, macros, etc), you must document the file in which they are defined. In other words, there must at least be a /*! \file */ or a ..."

`@example` could be useful. There is a special toolbar icon for examples

useful: `@attention`, `@bug`, `@deprecated` (maintains a deprecated list page), `@note`, `@anchor` + `@ref`, `@tableofcontents`, `@details`?, `@copybrief` and `@copydetails`

[@dontinclude](https://www.doxygen.nl/manual/commands.html#cmddontinclude) could possibly be useful in special situations but it's overly manual to be a reasonable general solution to stitching things together, unfortunately.

It to be relatively easy to include hand-made dot diagrams in documentation.

There is support for creating message sequence charts as in: https://www.mcternan.me.uk//mscgen/ and more general kinds of diagrams as in https://plantuml.com/state-diagram

TODO:
  - Control tree view alla [SO-28938063](https://stackoverflow.com/questions/28938063/customize-treeview-in-doxygen)?
  - Use `@test` tag
  - Use `@todo` tag
  - Use `@deprecated` tag
  - Investigate warnings when using `*_impl.hpp` implementations.
  - use testing namespace everywhere (e.g. in plonk composer tests) to prevent tons of useless test documentation from being created at in general namespace level like plonk

The VS Code extension oijaz.unicode-latex is convenient for parsing latex to unicode, e.g., `\gamma` to becomes `γ`, and this can make latex-heavy comments much easier to parse (`\gamma` seems to render as well as `γ`, to my surprise).

The `@details` tag is 'greedy' int the sense that it wants to include lots of data specified by other tags. As far as I can tell, the most robust strategy for putting documentation with code is to make the `@details` part of the leading code (the one coming before the class/function/whatever definiton) and then putting unwanted stuff inside the function body.

I recall seeing a way to extract parts of comments that are inside function bodies. Not sure a good application of this, though, sounds fidgety.