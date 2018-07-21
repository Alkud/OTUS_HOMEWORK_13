// server_advanced_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE ADVANCED_SERVER_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include <set>
#include <thread>
#include "async_server.h"
#include "naive_db.h"


BOOST_AUTO_TEST_SUITE(joinserver_advanced_server_test)

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

void sendGroupTestRequest(const std::vector<StringVector>& groupRequests,
                          std::vector<asio::ip::tcp::socket>& sockets)
{
  auto threadCount {groupRequests.size()};

  std::vector<std::thread> sendingThreads{};

  for (size_t idx{0}; idx < threadCount; ++idx)
  {
    sendingThreads.push_back(std::thread{[&groupRequest = groupRequests[idx],
                                          &socket = sockets[idx]]()
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
    }
    });
  }

  for (auto& thread : sendingThreads)
  {
    if (thread.joinable() == true)
    {
      thread.join();
    }
  }
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
}


void receiveTestReply(std::vector<StringVector>& replies,
                      std::vector<asio::ip::tcp::socket>& sockets)
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
      reply.push_back(replyString + "\n");
    }
  }
  else
  {
    std::cout << "receive reply error : " << errorCode.message() << "\n";
  }
}

void getGroupServerOutput(const std::vector<StringVector>& requests,
                          const std::vector<StringVector>& replies,
                          DebugOutput debug)
{

  std::vector<asio::ip::tcp::socket> sockets{};
  sockets.resize(requests.size());
  for (auto& socket : sockets)
  {
    socket = asio::ip::tcp::socket{testAsioService};
  }


  sendGroupTestRequest(requests, sockets);

  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& string : request)
    {
      std::cout << "> " << string;
    }
  }

  socket.shutdown(asio::ip::tcp::socket::shutdown_send);

  receiveTestReply(reply, socket);

  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& string : reply)
    {
      std::cout << "< " << string;
    }
  }
}


void checkServerRequest(
  const std::vector<StringVector>& requests, const StringVector& expectedReply,
  DebugOutput debug, bool groupRequest, size_t sendInterval = 200)
{
  testOutputStream.clear();
  testErrorStream.clear();

  testServer.getDb()->clear();

  testServer.getDb()->createTable("A");
  testServer.getDb()->createTable("B");

  std::vector<StringVector> testReplies{};

  getGroupServerOutput(requests, testReplies, debug);

  std::string actualCout{testOutputStream.str()};
  std::string actualErrOut{testErrorStream.str()};

  if (groupRequest != true) // non-group request should not mix commands and replies
  {
    StringVector splitCout{};
    std::string coutString {};
    while(std::getline(testOutputStream, coutString))
    {
      splitCout.push_back(coutString + "\n");
    }
    BOOST_CHECK_EQUAL_COLLECTIONS(splitCout.begin(), splitCout.end(),
                                  expectedReply.begin(), expectedReply.end());
  }
  else
  {
    std::multiset<std::string> mixedCout;

    std::string coutString {};
    while(std::getline(testOutputStream, coutString))
    {
      mixedCout.insert(coutString + "\n");
    }

    std::multiset<std::string> mixedReply{expectedReply.begin(), expectedReply.end()};

    BOOST_CHECK_EQUAL_COLLECTIONS(mixedCout.begin(), mixedCout.end(),
                                  mixedReply.begin(), mixedReply.end());
  }


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

BOOST_AUTO_TEST_CASE(multiple_duplicates_test)
{
  try
  {

  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_duplicates_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(multiple_truncate_test)
{
  try
  {

  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_truncate_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}


BOOST_AUTO_TEST_CASE(server_stop)
{
  try
  {
    testServer.stop();
    testWork.reset();
    if(testWorkingThread.joinable() == true)
    {
      testWorkingThread.join();
    };
  }
  catch (const std::exception& ex)
  {
    std::cerr << "server_stop failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
