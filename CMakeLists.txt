#CmakeList.txt for OTUS Homework #13

#Check CMake version
cmake_minimum_required(VERSION 3.2)

#Set project name, set PROJECT_VERSION_MINOR value equal to TRAVIS_BUILD NUMBER
if (DEFINED ENV{TRAVIS_BUILD_NUMBER})
  set (VERSION_PATCH $ENV{TRAVIS_BUILD_NUMBER})
else ()
  set (VERSION_PATCH 1)
endif()

set(CMAKE_CXX_STANDARD 14)

project(join_server VERSION 0.0.${VERSION_PATCH})

#add library subdirectories
add_subdirectory(naive_db)
add_subdirectory(async_server)

#add tests
add_subdirectory(unit_tests)

set (BOOST_MIN_VERSION "1.54.0")

#Find Boost library components
find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads)

set(BOOST_DEBUG                ON)
set(Boost_USE_MULTITHREADED    ON)

set(server_process
    ${PROJECT_SOURCE_DIR}/server_process.h
    ${PROJECT_SOURCE_DIR}/server_process.cpp)

#Set executable target
add_executable(join_server join_server.cpp ${server_process})

#Enable tests
enable_testing()

#Set properties and compile options for executable target
set_target_properties(join_server
                      PROPERTIES CXX_STANDARD 14
                      CXX_STANDARD_REQUIRED ON
                      COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

#Link
target_link_libraries(join_server pthread async_server naive_db ${Boost_LIBRARIES})

#Install
install(TARGETS join_server RUNTIME DESTINATION bin)


set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT alexandr_kudinov@mail.ru)

include(CPack)
