// command_translation_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE COMMAND_TRANSLATION_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

#include "db_command_translator.h"


BOOST_AUTO_TEST_SUITE(joinserver_command_translation_test)

asio::io_service testService{};
auto testSocket {std::make_shared<asio::ip::tcp::socket>(testService)};

BOOST_AUTO_TEST_CASE(known_commands_test)
{
  try
  {
    std::string request{};
    DbCommandReaction expectedReaction{};
    DbCommandReaction actualReaction{};

    request = "INSERT SomeTable 1 SomeName";

    expectedReaction = {
      DbCommands::INSERT,
      {"SomeTable", "1", "SomeName"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECTION";

    expectedReaction = {
      DbCommands::INTERSECTION,
      {"A", "B"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "SYMMETRIC_DIFFERENCE";

    expectedReaction = {
      DbCommands::SYMMETRIC_DIFFERENCE,
      {"A", "B"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE SomeTable";

    expectedReaction = {
      DbCommands::TRUNCATE,
      {"SomeTable"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

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
    DbCommandReaction expectedReaction{};
    DbCommandReaction actualReaction{};

    request = "INJECT SomeTable 1 SomeName";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'INJECT SomeTable 1 SomeName'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECT";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'INTERSECT'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "ASYMMETRIC_DIFFERENCE";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'ASYMMETRIC_DIFFERENCE'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'TRUNCATE'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "unknown_commands_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(bad_requests_test)
{
  try
  {
    std::string request{};
    DbCommandReaction expectedReaction{};
    DbCommandReaction actualReaction{};

    request = "INSERT SomeTable SomeName";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'INSERT SomeTable SomeName'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INSERT SomeTable SomeId SomeName";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'INSERT SomeTable SomeId SomeName'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "INTERSECTION A B";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'INTERSECTION A B'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "SYMMETRIC_DIFFERENCE B A";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'SYMMETRIC_DIFFERENCE B A'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);


    request = "TRUNCATE SomeTable please";

    expectedReaction = {
      DbCommands::EMPTY,
      {"ERR bad_request 'TRUNCATE SomeTable please'\n"},
      testSocket
    };

    actualReaction = DbCommandTranslator::translate(request, testSocket);

    BOOST_CHECK(actualReaction == expectedReaction);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "bad_requests_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
