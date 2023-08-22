barretenberg_module(smt_verification  ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1)

barretenberg_module(smt_examples     smt_verification proof_system stdlib_primitives)
barretenberg_module(smt_polynomiasls smt_verification proof_system stdlib_primitives)
barretenberg_module(smt_bigfieldd    smt_verification proof_system stdlib_primitives)