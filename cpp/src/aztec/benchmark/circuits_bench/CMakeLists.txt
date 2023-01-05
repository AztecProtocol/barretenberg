add_executable(circuits_bench circuits.bench.cpp)

target_link_libraries(
  circuits_bench
  stdlib_primitives
  rollup_proofs_join_split
  rollup_proofs_account
  rollup_proofs_claim
  env
  benchmark
)

add_custom_target(
    run_circuits_bench
    COMMAND circuits_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)