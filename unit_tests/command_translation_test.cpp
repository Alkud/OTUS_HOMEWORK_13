// db_operations_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE OTUS_HW_13_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

#include "command_translator.h"


BOOST_AUTO_TEST_SUITE(joinserver_command_translation_test)

asio::io_service testService{};
auto testSocket {std::make_shared<asio::ip::tcp::socket>(testService)};

BOOST_AUTO_TEST_CASE(known_commands_test)
{
  try
  {
    std::string request{};
    CommandReaction expectedReaction{};
    CommandReaction actualReaction{};

    request = "INSERT SomeTable 1 SomeName";

    expectedReaction = {
      DBCommands::INSERT,
      {"SomeTable", "1", "SomeName"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECTION";

    expectedReaction = {
      DBCommands::INTERSECTION,
      {},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "SYMMETRIC_DIFFERENCE";

    expectedReaction = {
      DBCommands::SYMMETRIC_DIFFERENCE,
      {},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE SomeTable";

    expectedReaction = {
      DBCommands::TRUNCATE,
      {"SomeTable"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);
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
    std::string request{};
    CommandReaction expectedReaction{};
    CommandReaction actualReaction{};

    request = "INJECT SomeTable 1 SomeName";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'INJECT SomeTable 1 SomeName'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECT";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'INTERSECT'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "ASYMMETRIC_DIFFERENCE";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'ASYMMETRIC_DIFFERENCE'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'TRUNCATE'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);
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
    std::string request{};
    CommandReaction expectedReaction{};
    CommandReaction actualReaction{};

    request = "INSERT SomeTable SomeName";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'INSERT SomeTable SomeName'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INSERT SomeTable SomeId SomeName";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'INSERT SomeTable SomeId SomeName'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECTION A B";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'INTERSECTION A B'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "SYMMETRIC_DIFFERENCE B A";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'SYMMETRIC_DIFFERENCE B A'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE SomeTable please";

    expectedReaction = {
      DBCommands::EMPTY,
      {"ERR bad request: 'TRUNCATE SomeTable please'\n"},
      testSocket
    };

    actualReaction = CommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "bad_requests_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
