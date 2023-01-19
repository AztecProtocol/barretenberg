FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        9.1.0 
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  zppbits
  GIT_REPOSITORY https://github.com/eyalz800/zpp_bits
  GIT_TAG        main
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_GetProperties(zppbits)
if(NOT zppbits_POPULATED)
  FetchContent_Populate(zppbits)
endif()

add_library(zppbits INTERFACE)
target_include_directories(zppbits INTERFACE ${zppbits_SOURCE_DIR})