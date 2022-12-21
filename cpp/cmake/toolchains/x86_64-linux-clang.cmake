# This branch is a workaround for the mainframe where both clang10 and clang15 
# are installed, but the clang binaries points to clang-10. 
# In this situation, there will be an explicit symlink to clang++-15
# which does not exist on a system with only one clang installation.
find_program(CLANGXX_15 "clang++-15")
if (CLANGXX_15)
    set(CMAKE_C_COMPILER "clang-15")
    set(CMAKE_CXX_COMPILER "clang++-15")
else()
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()