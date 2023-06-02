add_executable(standard_honk_bench main.bench.cpp standard_honk.bench.cpp)
add_executable(standard_plonk_bench main.bench.cpp standard_plonk.bench.cpp)
add_executable(ultra_honk_bench main.bench.cpp ultra_honk.bench.cpp)
add_executable(ultra_plonk_bench main.bench.cpp ultra_plonk.bench.cpp)

target_link_libraries(
  standard_honk_bench
  stdlib_primitives
  common
  stdlib_sha256
  stdlib_keccak
  stdlib_merkle_tree
  env
  benchmark::benchmark
)

target_link_libraries(
  standard_plonk_bench
  stdlib_primitives
  stdlib_sha256
  stdlib_keccak
  stdlib_merkle_tree
  env
  benchmark::benchmark
)

target_link_libraries(
  ultra_honk_bench
  stdlib_primitives
  stdlib_sha256
  stdlib_keccak
  stdlib_merkle_tree
  env
  benchmark::benchmark
)

target_link_libraries(
  ultra_plonk_bench
  stdlib_primitives
  stdlib_sha256
  stdlib_keccak
  stdlib_merkle_tree
  env
  benchmark::benchmark
)

add_custom_target(
    run_standard_honk_bench
    COMMAND standard_honk_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(
    run_standard_plonk_bench
    COMMAND standard_plonk_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(
    run_ultra_honk_bench
    COMMAND ultra_honk_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(
    run_ultra_plonk_bench
    COMMAND ultra_plonk_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)