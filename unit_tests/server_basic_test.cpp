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

asio::io_service testAsioService{};

std::thread testWorkingThread{[]()
{
  testAsioService.run();
}};

UniqueWork testWork { new asio::io_service::work(testAsioService)};


const asio::ip::address_v4 testAddress{asio::ip::address_v4::any()};
constexpr uint16_t testPortNumber {10007};

constexpr size_t READ_BUFFER_SIZE {100};


AsyncJoinServer<2> testServer {
  testAddress,
  testPortNumber,
  testOutputStream,
  testErrorStream
};

void sendTestRequest(const StringVector& request, asio::ip::tcp::socket& socket)
{
  /*connect to testServer*/
  asio::ip::tcp::endpoint endpoint{testAddress, testPortNumber};
  socket.connect(endpoint);

  /* build request string*/
  std::stringstream requestStream {};
  for (const auto& command : request)
  {
    requestStream << command << "\n";
  }
  std::string requestString{requestStream.str()};

  /* send request string */
  asio::write(socket, asio::buffer(requestString.c_str(), requestString.size()));
  socket.shutdown(asio::ip::tcp::socket::shutdown_send);
}

void receiveTestReply(StringVector& reply, asio::ip::tcp::socket& socket)
{
  std::array<char, READ_BUFFER_SIZE> readBuffer{};

  system::error_code errorCode{};

  std::stringstream replyStream {};

  do
  {
    auto bytes_transferred {asio::read(socket, asio::buffer(readBuffer), errorCode)};
    std::copy(std::begin(readBuffer),
              std::begin(readBuffer) + bytes_transferred,
              std::ostream_iterator<char>(replyStream));
  }
  while (0 == errorCode);

  std::string replyString{};

  if (errorCode == asio::error::eof)
  {
    while (std::getline(replyStream, replyString))
    {
      reply.push_back(replyString);
    }
  }
}

void getServerOutput(const StringVector& request, StringVector& reply)
{
  asio::ip::tcp::socket socket{testAsioService};

  sendTestRequest(request, socket);

  receiveTestReply(reply, socket);
}

BOOST_AUTO_TEST_CASE(server_start)
{
  testServer.start();
  BOOST_CHECK(testWorkingThread.joinable() == true);
}

BOOST_AUTO_TEST_CASE(insert_record_test)
{
  try
  {
    testDB->createTable("A");
    testDB->createTable("B");

    std::vector<std::string> insertRequest{"INSERT A 123 C++"};

    std::vector<std::string> insertReply{};

    getServerOutput(insertRequest, insertReply);
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

BOOST_AUTO_TEST_CASE(server_stop)
{
  testServer.stop();
  testWork.reset();
  if(testWorkingThread.joinable() == true)
  {
    testWorkingThread.join();
  };
}

BOOST_AUTO_TEST_SUITE_END()
