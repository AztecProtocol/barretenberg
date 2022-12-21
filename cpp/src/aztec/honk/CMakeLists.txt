barretenberg_module(honk numeric ecc srs proof_system transcript)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(
        honk_test_objects 
        PRIVATE 
        -Wno-error=unused-variable
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(
        honk_test_objects 
        PRIVATE 
        -Wno-error=maybe-uninitialized
        -Wno-error=uninitialized
    )
endif()