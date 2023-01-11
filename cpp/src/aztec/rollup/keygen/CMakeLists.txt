add_executable(
    keygen
    main.cpp
)

# Disables an error due to possible string overflow 
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(
        keygen
        PRIVATE
        -Wno-stringop-overflow
        -Wno-array-bounds
    )
endif()

target_link_libraries(
    keygen
    PRIVATE
    rollup_proofs_root_verifier
    env
)