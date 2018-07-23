project(unit_tests)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Threads)

add_executable(db_operations_test  db_operations_test.cpp)
add_executable(command_translation_test command_translation_test.cpp)
add_executable(server_basic_test server_basic_test.cpp)
add_executable(server_advanced_test server_advanced_test.cpp)

target_include_directories(db_operations_test PRIVATE ${Boost_INCLUDE_DIR} ../naive_db)
target_include_directories(command_translation_test PRIVATE ${Boost_INCLUDE_DIR} ../async_server)
target_include_directories(server_basic_test PRIVATE ${Boost_INCLUDE_DIR} ../naive_db ../async_server)
target_include_directories(server_advanced_test PRIVATE ${Boost_INCLUDE_DIR} ../naive_db ../async_server)

set_target_properties(db_operations_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK  
)

set_target_properties(command_translation_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
)

set_target_properties(server_basic_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
)

set_target_properties(server_advanced_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
)


target_link_libraries(db_operations_test
  pthread
  naive_db
  ${Boost_LIBRARIES}
)

target_link_libraries(command_translation_test
  pthread
  async_server
  ${Boost_LIBRARIES}
)

target_link_libraries(server_basic_test
  pthread
  naive_db
  async_server
  ${Boost_LIBRARIES}
)

target_link_libraries(server_advanced_test
  pthread
  naive_db
  async_server
  ${Boost_LIBRARIES}
)

enable_testing()

add_test(NAME TEST_DB_OPERATIONS
         COMMAND db_operations_test)

add_test(NAME TEST_COMMAND_TRANSLATION
         COMMAND command_translation_test)

add_test(NAME TEST_SERVER_BASIC_OPERATIONS
         COMMAND server_basic_test)

add_test(NAME TEST_SERVER_MULTITHREAD_OPERATIONS
         COMMAND server_advanced_test)


