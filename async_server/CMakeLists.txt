#CmakeList.txt for async_server library in OTUS Homework #13

# set library name and content
add_library(async_server
  server_common_types.h
  async_server.h
  async_acceptor.h async_acceptor.cpp
  async_reader.h async_reader.cpp
  db_manager.h db_manager.cpp
  db_command_translator.h db_command_translator.cpp
)

find_package(Boost COMPONENTS system REQUIRED)

target_include_directories(async_server PUBLIC ../naive_db)

target_link_libraries(async_server pthread naive_db ${Boost_LIBRARIES})
