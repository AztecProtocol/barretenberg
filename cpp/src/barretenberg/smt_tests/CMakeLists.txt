add_subdirectory(terms)
add_subdirectory(solver)
add_subdirectory(circuit)
add_subdirectory(examples_and_tests)

barretenberg_module(smt_tests smt_solver smt_circuit smt_terms) # TODO(alex): didn't work