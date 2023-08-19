add_executable(poly_smt 
    polynomials/main.cpp 
    polynomials/create_circuit.cpp 
    polynomials/create_circuit.hpp
)

target_link_libraries(
  poly_smt
  ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1
  smt_circuit
  proof_system
  common
)                                                                                                                                                                                 

add_executable(bf_mul_smt 
    bigfield/main.cpp 
    bigfield/create_circuit.cpp 
    bigfield/create_circuit.cpp
)

target_link_libraries(
  bf_mul_smt
  ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1
  smt_circuit
  proof_system
  common
)

barretenberg_module(smt_examples smt_circuit proof_system stdlib_primitives ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1) 
# TODO(alex): stil doesn't work