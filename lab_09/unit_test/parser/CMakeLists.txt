enable_testing()

add_executable(
  parser-test
  parser-test.cc

  ../../lexer.cc 
  ../../type.cc 
  ../../diag-engine.cc
  ../../parser.cc 
  ../../print-visitor.cc
  ../../sema.cc 
  ../../scope.cc
)

llvm_map_components_to_libnames(llvm_all Support Core)

#message(STATUS "iiicp: ${llvm_all}")

target_link_libraries(
  parser-test
  GTest::gtest_main
  ${llvm_all}
)

include(GoogleTest)
gtest_discover_tests(parser-test)