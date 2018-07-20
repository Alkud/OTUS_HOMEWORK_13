// command_translation_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE BASIC_SERVER_TEST

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

enum class DebugOutput
{
  DebugOff = 0,
  DebugOn = 1
}
;

const asio::ip::address_v4 testAddress{asio::ip::address_v4::any()};
constexpr uint16_t testPortNumber {10007};

constexpr size_t READ_BUFFER_SIZE {100};


AsyncJoinServer<2> testServer {
  testAddress,
  testPortNumber,
  testOutputStream,
  testErrorStream
};

void sendGroupTestRequest(const StringVector& groupRequest, asio::ip::tcp::socket& socket)
{
  /*connect to testServer*/
  asio::ip::tcp::endpoint endpoint{testAddress, testPortNumber};
  socket.connect(endpoint);

  /* build request string*/
  std::stringstream requestStream {};
  for (const auto& command : groupRequest)
  {
    /* send request string */
    asio::write(socket, asio::buffer(command.c_str(), command.size()));
    requestStream << command;
  }

  //socket.shutdown(asio::ip::tcp::socket::shutdown_send);
}

void sendTestRequest(const std::string& request, asio::ip::tcp::socket& socket)
{
  /*connect to testServer*/
  asio::ip::tcp::endpoint endpoint{testAddress, testPortNumber};
  if (socket.is_open() != true)
  {
    socket.connect(endpoint);
  }

  /* send request string */
  asio::write(socket, asio::buffer(request.c_str(), request.size()));
  //socket.shutdown(asio::ip::tcp::socket::shutdown_send);
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

  //socket.shutdown(asio::ip::tcp::socket::shutdown_receive);

  std::string replyString{};

  if (errorCode == asio::error::eof)
  {
    while (std::getline(replyStream, replyString))
    {
      reply.push_back(replyString + "\n");
    }
  }
  else
  {
    std::cout << "receive reply error : " << errorCode.message() << "\n";
  }
}

void getGroupServerOutput(const StringVector& request, StringVector& reply, DebugOutput debug)
{
  asio::ip::tcp::socket socket{testAsioService};

  sendGroupTestRequest(request, socket);

  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& string : request)
    {
      std::cout << "> " << string;
    }
  }

  receiveTestReply(reply, socket);

  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& string : reply)
    {
      std::cout << "< " << string;
    }
  }
}

void getServerOutput(const StringVector& request, StringVector& reply, DebugOutput debug)
{
  for (const auto& command : request)
  {
    asio::ip::tcp::socket socket{testAsioService};

    sendTestRequest(command, socket);

    if (DebugOutput::DebugOn == debug)
    {
      std::cout << "> " << command;
    }

    StringVector commandReply{};

    receiveTestReply(commandReply, socket);

    for (const auto& string : commandReply)
    {
      reply.push_back(string);

      if (DebugOutput::DebugOn == debug)
      {
        std::cout << "< " << string;
      }
    }
  }
}


void checkServerRequest(const StringVector& request,
  const StringVector& expectedReply, DebugOutput debug)
{
  testOutputStream.clear();
  testErrorStream.clear();

  testServer.getDb()->clear();

  testServer.getDb()->createTable("A");
  testServer.getDb()->createTable("B");

  std::vector<std::string> testReply{};

  getServerOutput(request, testReply, debug);

  std::string actualCout{testOutputStream.str()};
  std::string actualErrOut{testErrorStream.str()};

  StringVector splitCout{};

  std::string coutString {};
  while(std::getline(testOutputStream, coutString))
  {
    splitCout.push_back(coutString + "\n");
  }


  BOOST_CHECK_EQUAL_COLLECTIONS(splitCout.begin(), splitCout.end(),
                                expectedReply.begin(), expectedReply.end());

  for (const auto& requestString : request)
  {
    BOOST_CHECK(actualCout.find(requestString) != std::string::npos);
  }

  for (const auto& replyString : testReply)
  {
    BOOST_CHECK(actualCout.find(replyString) != std::string::npos);
  }

  BOOST_CHECK(actualErrOut.empty() == true);
}



BOOST_AUTO_TEST_CASE(server_start)
{
  testServer.start();
  BOOST_CHECK(testWorkingThread.joinable() == true);

  testDB->createTable("R");
  testDB->createTable("S");
  testServer.adoptDb(testDB);

  BOOST_CHECK(testDB.get() == testServer.getDb().get());

  testDB->clear();
  BOOST_CHECK(testServer.getDb()->getTotalDataSize() == 0);

  auto tempDB {std::make_shared<NaiveDB>()};

  tempDB->createTable("X");
  tempDB->createTable("Y");
  tempDB->createTable("Z");

  testServer.swapDb(tempDB);
  BOOST_CHECK(testServer.getDb()->getTablesSize() == 3);
  BOOST_CHECK(tempDB->getTablesSize() == 0);

  testServer.swapDb(tempDB);
  BOOST_CHECK(testServer.getDb()->getTotalDataSize() == 0);
  BOOST_CHECK(testDB.get() == testServer.getDb().get());
}

BOOST_AUTO_TEST_CASE(insert_record_test)
{
  try
  {
//    StringVector insertRequest{"INSERT A 123 C++\n"};
//    StringVector insertReply {"> INSERT A 123 C++\n",
//                              "< OK\n"};

//    checkServerRequest(insertRequest, insertReply, DebugOutput::DebugOff);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "insert__recird_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(truncate_table_test)
{
  try
  {
    StringVector insertRequest{"INSERT A 12 qwerty\n",
                               "INSERT B 26 asdfgh\n",
                               "TRUNCATE A\n",
                               "TRUNCATE B\n"};
    StringVector insertReply {"> INSERT A 12 qwerty\n",
                              "< OK\n",
                              "> INSERT B 26 asdfgh\n",
                              "< OK\n",
                              "> TRUNCATE A\n",
                              "< OK\n",
                              "> TRUNCATE B\n",
                              "< OK\n"};

    checkServerRequest(insertRequest, insertReply, DebugOutput::DebugOn);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "truncate_table_test failed. " << ex.what();
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
