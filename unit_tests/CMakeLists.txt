project(unit_tests)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Threads)

add_executable(db_operations_test  db_operations_test.cpp)

target_include_directories(db_operations_test PRIVATE ${Boost_INCLUDE_DIR} ../naive_db)

set_target_properties(db_operations_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK  
)

target_link_libraries(db_operations_test
  naive_db
  ${Boost_LIBRARIES}
)

enable_testing()

add_test(NAME TEST_DB_OPERATIONS
         COMMAND db_operations_test)


