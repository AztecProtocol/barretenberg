if(NOT WASM)
    include(FetchContent)
    FetchContent_Declare(
        leveldb
        GIT_REPOSITORY https://github.com/google/leveldb.git
        GIT_TAG 1.22
        FIND_PACKAGE_ARGS
    )

        set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "LevelDB tests off")
    set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "LevelDB benchmarks off")

    FetchContent_MakeAvailable(leveldb)

    if (leveldb_SOURCE_DIR AND leveldb_BINARY_DIR)
        set_property(DIRECTORY ${leveldb_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL)
        set_property(DIRECTORY ${leveldb_BINARY_DIR} PROPERTY EXCLUDE_FROM_ALL)

    target_compile_options(
        leveldb
        PRIVATE
            -Wno-unknown-warning-option
            -Wno-unused-but-set-variable
            -Wno-sign-conversion
            -Wno-unused-parameter
            -Wno-shorten-64-to-32
            -Wno-implicit-int-conversion
            -Wno-conversion
            -Wno-implicit-fallthrough
    )
    endif()

    link_libraries(leveldb)
endif()

barretenberg_module(stdlib_merkle_tree stdlib_primitives stdlib_blake3s stdlib_pedersen)