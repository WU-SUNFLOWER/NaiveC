enable_testing()

project(lexer-test)

add_executable(
  ${PROJECT_NAME}

  lexer-test.cc

  ../../lexer.cc 
  ../../type.cc 
  ../../diag-engine.cc
)

llvm_map_components_to_libnames(llvm_all Support Core)

target_link_libraries(
  ${PROJECT_NAME}
  GTest::gtest_main
  ${llvm_all}
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})