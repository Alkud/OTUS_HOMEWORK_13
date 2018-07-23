// server_basic_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE BASIC_SERVER_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include <set>
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

std::atomic<bool> shouldExit{false};
std::condition_variable terminationNotifier{};


AsyncJoinServer<2> testServer {
  testAddress,
  testPortNumber,
  testOutputStream,
  testErrorStream,
  shouldExit, terminationNotifier
};

void sendGroupTestRequest(const StringVector& groupRequest, asio::ip::tcp::socket& socket)
{
  /*connect to testServer*/
  asio::ip::tcp::endpoint endpoint{testAddress, testPortNumber};
  socket.connect(endpoint);

  /* build request string*/  
  for (const auto& command : groupRequest)
  {
    /* send request string */
    asio::write(socket, asio::buffer(command.c_str(), command.size()));    
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

void getServerOutput(const StringVector& request, StringVector& reply,
                     DebugOutput debug, size_t sendInterval)
{
  for (const auto& command : request)
  {
    asio::ip::tcp::socket socket{testAsioService};

    sendTestRequest(command, socket);

    socket.shutdown(asio::ip::tcp::socket::shutdown_send);

    std::this_thread::sleep_for(std::chrono::milliseconds{sendInterval});

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


void checkServerRequest(
  const StringVector& request, const StringVector& expectedReply,
  DebugOutput debug, bool groupRequest, size_t sendInterval = 200)
{
  testOutputStream.clear();
  testErrorStream.clear();

  testServer.getDb()->clear();

  testServer.getDb()->createTable("A");
  testServer.getDb()->createTable("B");

  std::vector<std::string> testReply{};

  if (true == groupRequest) // send all commands with no gaps, then receive all replies
  {
    getGroupServerOutput(request, testReply, debug);
  }
  else // send a command, receive a reply, wait sendInterval milliseconds
  {
    getServerOutput(request, testReply, debug, sendInterval);
  }

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

BOOST_AUTO_TEST_CASE(insert_record_test)
{
  try
  {
    StringVector insertRequest{"INSERT A 123 C++\n"};
    StringVector insertReply {"> INSERT A 123 C++\n",
                              "< OK\n"};

    checkServerRequest(insertRequest, insertReply,
                       DebugOutput::DebugOff, false, 50);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "insert_record_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(truncate_table_test)
{
  try
  {
    StringVector truncateRequest{"INSERT A 12 qwerty\n",
                               "INSERT B 26 asdfgh\n",
                               "TRUNCATE A\n",
                               "TRUNCATE B\n"};
    StringVector truncateReply {"> INSERT A 12 qwerty\n",
                              "< OK\n",
                              "> INSERT B 26 asdfgh\n",
                              "< OK\n",
                              "> TRUNCATE A\n",
                              "< OK\n",
                              "> TRUNCATE B\n",
                              "< OK\n"};

    checkServerRequest(truncateRequest, truncateReply,
                       DebugOutput::DebugOff, false, 50);

    BOOST_CHECK(testDB->getTotalDataSize() == 0);
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
    StringVector badRequest{ "INSERT A 1 1\n",
                             "INSERT A 1 2\n",
                             "TRUNCATE C\n",
                             "TRUNCATE A 1\n",
                             "TRUNC A\n",
                             "INTERSECT A B\n"};

    StringVector badReply {"> INSERT A 1 1\n",
                           "< OK\n",
                           "> INSERT A 1 2\n",
                           "< ERR duplicate 1\n",
                           "> TRUNCATE C\n",
                           "< ERR not_found C\n",
                           "> TRUNCATE A 1\n",
                           "< ERR bad_request 'TRUNCATE A 1'\n",
                           "> TRUNC A\n",
                           "< ERR bad_request 'TRUNC A'\n",
                           "> INTERSECT A B\n",
                           "< ERR bad_request 'INTERSECT A B'\n",};

    checkServerRequest(badRequest, badReply,
                       DebugOutput::DebugOff, false, 50);

    BOOST_CHECK(testDB->getTotalDataSize() == 1);

  }
  catch (const std::exception& ex)
  {
    std::cerr << "bad_requests_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(intersection_test)
{
  try
  {
    StringVector insertRequest{};
    StringVector insertReply{};

    for (size_t idx{0}; idx < 100; ++idx)
    {
      insertRequest.push_back(std::string{"INSERT A "}
                              + std::to_string(idx * 2) + " "
                              + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"> INSERT A "}
                            + std::to_string(idx * 2) + " "
                            + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"< OK\n"});

      insertRequest.push_back(std::string{"INSERT B "}
                              + std::to_string(idx * 2 + 1) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"> INSERT B "}
                              + std::to_string(idx * 2 + 1) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"< OK\n"});
    }


    for (size_t idx{200}; idx < 210; ++idx)
    {
      insertRequest.push_back(std::string{"INSERT A "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"> INSERT A "}
                            + std::to_string(idx) + " "
                            + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"< OK\n"});

      insertRequest.push_back(std::string{"INSERT B "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"> INSERT B "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"< OK\n"});
    }

    //    table A             table B
    //    id | name           id | name
    //     0 |  0              1 |   1
    //     2 |  2              3 |   3
    //     4 |  4              5 |   5
    //     6 |  6              7 |   7
    //     8 |  8              9 |   9
    //    10 | 10             11 |  11
    //    12 | 12             13 |  13
    //    14 | 14             15 |  15
    //    16 | 16             17 |  17
    //    18 | 18             19 |  19
    //     ......              ......
    //   198 | 198           199 | 199
    //   200 | 400           200 | 401
    //   201 | 402           201 | 403
    //   202 | 404           202 | 404
    //     ......              ......
    //   209 | 418           209 | 419

    insertRequest.push_back("INTERSECTION\n");

    insertReply.push_back("> INTERSECTION\n");

    for (size_t idx{200}; idx < 210; ++idx)
    {
      insertReply.push_back(std::string{
                              "< "
                            + std::to_string(idx) + ","
                            + std::to_string(idx * 2) + ","
                            + std::to_string(idx * 2 + 1) + "\n"});
    }

    //    id | name | name
    //   200 | 400  | 401
    //   201 | 402  | 403
    //   202 | 404  | 405
    //   203 | 406  | 407
    //   204 | 408  | 409
    //   205 | 410  | 411
    //     ..............
    //   209 | 418  | 419

    insertReply.push_back("< OK\n");

    checkServerRequest(insertRequest, insertReply,
                       DebugOutput::DebugOff, true);

    checkServerRequest(insertRequest, insertReply,
                       DebugOutput::DebugOff, false, 50);

    BOOST_CHECK(testDB->getTotalDataSize() == 220);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "intersection_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(symmetric_difference_test)
{
  try
  {
    StringVector insertRequest{};
    StringVector insertReply{};

    for (size_t idx{0}; idx < 100; ++idx)
    {
      insertRequest.push_back(std::string{"INSERT A "}
                              + std::to_string(idx * 2) + " "
                              + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"> INSERT A "}
                            + std::to_string(idx * 2) + " "
                            + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"< OK\n"});

      insertRequest.push_back(std::string{"INSERT B "}
                              + std::to_string(idx * 2 + 1) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"> INSERT B "}
                              + std::to_string(idx * 2 + 1) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"< OK\n"});
    }


    for (size_t idx{200}; idx < 210; ++idx)
    {
      insertRequest.push_back(std::string{"INSERT A "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"> INSERT A "}
                            + std::to_string(idx) + " "
                            + std::to_string(idx * 2) + "\n");
      insertReply.push_back(std::string{"< OK\n"});

      insertRequest.push_back(std::string{"INSERT B "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"> INSERT B "}
                              + std::to_string(idx) + " "
                              + std::to_string(idx * 2 + 1) + "\n");
      insertReply.push_back(std::string{"< OK\n"});
    }

    //    table A             table B
    //    id | name           id | name
    //     0 |  0              1 |   1
    //     2 |  2              3 |   3
    //     4 |  4              5 |   5
    //     6 |  6              7 |   7
    //     8 |  8              9 |   9
    //    10 | 10             11 |  11
    //    12 | 12             13 |  13
    //    14 | 14             15 |  15
    //    16 | 16             17 |  17
    //    18 | 18             19 |  19
    //     ......              ......
    //   198 | 198           199 | 199
    //   200 | 400           200 | 401
    //   201 | 402           201 | 403
    //   202 | 404           202 | 404
    //     ......              ......
    //   209 | 418           209 | 419

    insertRequest.push_back("SYMMETRIC_DIFFERENCE\n");

    insertReply.push_back("> SYMMETRIC_DIFFERENCE\n");

    for (size_t idx{0}; idx < 200; ++idx)
    {
      if (idx % 2 == 0)
      {
        insertReply.push_back(std::string{
                                "< "
                              + std::to_string(idx) + ","
                              + std::to_string(idx) + "," + "\n"});
      }
      else
      {
        insertReply.push_back(std::string{
                                "< "
                              + std::to_string(idx) + "," + ","
                              + std::to_string(idx) + "\n"});
      }
    }

    //    id | name | name
    //     0 |   0  |
    //     1 |      |   1
    //     2 |   2  |
    //     3 |      |   3
    //     4 |   4  |
    //     5 |      |   5
    //     ..............
    //   198 | 198  |
    //   199 |      | 199


    insertReply.push_back("< OK\n");

    checkServerRequest(insertRequest, insertReply,
                      DebugOutput::DebugOff, true);

    checkServerRequest(insertRequest, insertReply,
                       DebugOutput::DebugOff, false, 50);

    BOOST_CHECK(testDB->getTotalDataSize() == 220);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "intersection_test failed. " << ex.what();
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
