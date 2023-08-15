add_executable(poly_smt 
    main.cpp 
    check_sound_circ.cpp 
    polynomials/function_to_check.cpp
)

message(${CMAKE_SOURCE_DIR})
target_link_libraries(
  poly_smt
  ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1
  common
)                                                                                                                                                                                 


add_executable(bf_mul_smt 
    main.cpp 
    check_sound_circ.cpp 
    bigfield/function_to_check.cpp
)

message(${CMAKE_SOURCE_DIR})
target_link_libraries(
  bf_mul_smt
  ${CMAKE_SOURCE_DIR}/src/cvc5/tmp-build/lib/libcvc5.so.1
  common
)      