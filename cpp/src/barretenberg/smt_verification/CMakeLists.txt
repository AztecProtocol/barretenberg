barretenberg_module(smt_verification common ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-lib/lib/libcvc5.so.1)

barretenberg_module(smt_examples common proof_system stdlib_primitives smt_verification)
#barretenberg_module(smt_polynomials common proof_system stdlib_primitives smt_verification)
#barretenberg_module(smt_bigfield common proof_system stdlib_primitives smt_verification)