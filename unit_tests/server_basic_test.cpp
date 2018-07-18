// command_translation_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE OTUS_HW_13_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

#include "async_server.h"
#include "naive_db.h"


BOOST_AUTO_TEST_SUITE(joinserver_basic_server_test)

SharedNaiveDB testDB { new NaiveDB };

std::stringstream testOutputStream{};
std::stringstream testErrorStream{};

//AsyncJoinServer<2> testServer {
//
//}


BOOST_AUTO_TEST_CASE(known_commands_test)
{
  try
  {

  }
  catch (const std::exception& ex)
  {
    std::cerr << "known_commands_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(unknown_commands_test)
{
  try
  {
  }
  catch (const std::exception& ex)
  {
    std::cerr << "copy_table_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(bad_requests_test)
{
  try
  {

  }
  catch (const std::exception& ex)
  {
    std::cerr << "bad_requests_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
