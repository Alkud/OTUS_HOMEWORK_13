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

std::vector<std::thread> testWorkingThreads{};

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

void sendGroupTestRequest(const std::vector<StringVector>& groupRequests,
                          std::vector<asio::ip::tcp::socket>& sockets,
                          const size_t requestsDelay)
{
  auto threadCount {groupRequests.size()};

  std::vector<std::thread> sendingThreads{};

  for (size_t idx{0}; idx < threadCount; ++idx)
  {

    sendingThreads.push_back(std::thread{[groupRequest = groupRequests[idx],
                                          &socket = sockets[idx]]()
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
    });

    if (requestsDelay > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{requestsDelay});
    }
  }

  for (auto& thread : sendingThreads)
  {
    if (thread.joinable() == true)
    {
      thread.join();
    }
  }
}


void receiveTestReply(std::vector<StringVector>& replies,
                      std::vector<asio::ip::tcp::socket>& sockets,
                      const size_t requestsDelay)
{

  auto threadCount {replies.size()};

  std::vector<std::thread> receivingThreads{};

  std::mutex repliesLock{};

  for (size_t idx{0}; idx < threadCount; ++idx)
  {
    receivingThreads.push_back(std::thread{[&reply = replies[idx],
                                          &socket = sockets[idx],
                                          &lock = repliesLock]()
    {
      std::array<char, READ_BUFFER_SIZE> readBuffer{};
      system::error_code errorCode{};
      std::stringstream replyStream {};

      size_t bytes_transferred{};
      do
      {
        bytes_transferred = asio::read(socket, asio::buffer(readBuffer), errorCode);
        std::copy(std::begin(readBuffer),
                  std::begin(readBuffer) + bytes_transferred,
                  std::ostream_iterator<char>(replyStream));
      }
      while (errorCode == 0);
      std::string replyString{};
      if (errorCode == asio::error::eof)
      {
        while (std::getline(replyStream, replyString))
        {
          std::lock_guard<std::mutex> lockCommonVector{lock};
          reply.push_back(replyString + "\n");
        }
      }
      else
      {
        std::cout << "receive reply error : " << errorCode.message() << "\n";
      }
    }
    });

    if (requestsDelay > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{requestsDelay});
    }
  }

  for (auto& thread : receivingThreads)
  {
    if (thread.joinable() == true)
    {
      thread.join();
    }
  }
}

void getGroupServerOutput(const std::vector<StringVector>& requests,
                          std::vector<StringVector>& replies,
                          DebugOutput debug,
                          const size_t requestsDelay)
{
  auto requestCount{requests.size()};

  std::vector<asio::ip::tcp::socket> sockets{};
  for (size_t idx {0}; idx < requestCount; ++idx)
  {
    sockets.emplace_back(testAsioService);
  }


  replies.resize(requestCount);

  std::thread writingThread{[&requests, &sockets, &requestsDelay]()
  {
     sendGroupTestRequest(requests, sockets, requestsDelay);
  }};

  std::this_thread::sleep_for(10ms);

  std::thread readingThread{[&replies, &sockets, &requestsDelay]()
  {
     receiveTestReply(replies, sockets, requestsDelay);
  }};

  writingThread.join();

  for (auto& socket : sockets)
  {
    socket.shutdown(asio::ip::tcp::socket::shutdown_send);
  }

  readingThread.join();


  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& request : requests)
    {
      for (const auto& string : request)
      {
        std::cout << "> " << string;
      }
    }
  }  

  if (DebugOutput::DebugOn == debug)
  {
    for (const auto& reply : replies)
    {
      for (const auto& string : reply)
      {
        std::cout << "< " << string;
      }
    }
  }
}


void checkServerRequest(
  const std::vector<StringVector>& requests,
  const std::vector<StringVector>& expectedReplies,
  const StringVector& expectedCout,
  DebugOutput debug,
  const size_t requestsDelay
)
{
  testOutputStream.clear();
  testErrorStream.clear();

  testServer.getDb()->clear();

  testServer.getDb()->createTable("A");
  testServer.getDb()->createTable("B");

  std::vector<StringVector> testReplies{};

  getGroupServerOutput(requests, testReplies, debug, requestsDelay);

  std::string actualCout{testOutputStream.str()};
  std::string actualErrOut{testErrorStream.str()};

  auto requestCount{requests.size()};

  for (size_t idx{0}; idx < requestCount; ++idx)
  {
    std::multiset<std::string> mixedExpectedReply{
      expectedReplies[idx].begin(),
      expectedReplies[idx].end()
    };

    std::multiset<std::string> mixedTestReply{
      testReplies[idx].begin(),
      testReplies[idx].end()
    };

    BOOST_CHECK_EQUAL_COLLECTIONS(
      mixedExpectedReply.begin(), mixedExpectedReply.end(),
      mixedTestReply.begin(), mixedTestReply.end());
  }

  std::multiset<std::string> mixedCout;

  std::string coutString {};
  while(std::getline(testOutputStream, coutString))
  {
    mixedCout.insert(coutString + "\n");
  }

  std::multiset<std::string> mixedReply;

  for (const auto& testReply : testReplies)
  {
    for (const auto& replyString : testReply)
    {
      mixedReply.insert(replyString);
    }
  }

  std::multiset<std::string> expectedMixedCout;

  for (const auto& expectedString : expectedCout)
  {
    expectedMixedCout.insert(expectedString);
  }


  BOOST_CHECK_EQUAL_COLLECTIONS(
    mixedCout.begin(), mixedCout.end(),
    expectedMixedCout.begin(), expectedMixedCout.end());

  for (const auto& request : requests)
  {
    for (const auto& requestString : request)
    {
      BOOST_CHECK(actualCout.find(requestString) != std::string::npos);
    }
  }

  for (const auto& testReply : testReplies)
  {
    for (const auto& replyString : testReply)
    {
      BOOST_CHECK(actualCout.find(replyString) != std::string::npos);
    }
  }

  BOOST_CHECK(actualErrOut.empty() == true);
}



BOOST_AUTO_TEST_CASE(server_start)
{
  testServer.start();

  for(size_t idx{0}; idx < 4; ++idx)
  {
    testWorkingThreads.emplace_back(
    []()
    {
      testAsioService.run();
    });
  }

  for (const auto& thread : testWorkingThreads)
  {
    BOOST_CHECK(thread.joinable() == true);
  }

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

BOOST_AUTO_TEST_CASE(multiple_inserts_test)
{
  try
  {
    std::vector<StringVector> insertRequests{2};
    std::vector<StringVector> insertReplies{2};
    StringVector insertCout{};

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      insertRequests[0].push_back(std::string{"INSERT A "}
                                  + std::to_string(idx * 2) + " "
                                  + std::to_string(idx * 2) + "\n");

             insertCout.push_back(std::string{"> INSERT A "}
                                  + std::to_string(idx * 2) + " "
                                  + std::to_string(idx * 2) + "\n");

       insertReplies[0].push_back(std::string{"OK\n"});

            insertCout.push_back (std::string{"< OK\n"});
    }

    for(size_t idx {1000}; idx < 2000; ++idx)
    {
      insertRequests[1].push_back(std::string{"INSERT B "}
                                  + std::to_string(idx * 2) + " "
                                  + std::to_string(idx * 2) + "\n");

             insertCout.push_back(std::string{"> INSERT B "}
                                  + std::to_string(idx * 2) + " "
                                  + std::to_string(idx * 2) + "\n");

      insertReplies[1].push_back (std::string{"OK\n"});

            insertCout.push_back (std::string{"< OK\n"});
    }

    checkServerRequest(insertRequests, insertReplies, insertCout,
                       DebugOutput::DebugOff, 0);

    BOOST_CHECK(testDB->getTotalDataSize() == 2000);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_duplicates_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(multiple_duplicates_test)
{
  try
  {
    std::vector<StringVector> duplicateRequests{2};
    std::vector<StringVector> duplicateReplies{2};
    StringVector duplicateCout{};

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      duplicateRequests[0].push_back(std::string{"INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx) + "\n");

             duplicateCout.push_back(std::string{"> INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx) + "\n");

       duplicateReplies[0].push_back(std::string{"OK\n"});

             duplicateCout.push_back (std::string{"< OK\n"});
    }

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      duplicateRequests[1].push_back(std::string{"INSERT A "}
                                    + std::to_string(idx) + " "
                                    + std::to_string(idx) + "\n");

            duplicateCout.push_back(std::string{"> INSERT A "}
                                    + std::to_string(idx) + " "
                                    + std::to_string(idx) + "\n");

      duplicateReplies[1].push_back (std::string{"ERR duplicate "}
                                     + std::to_string(idx) + "\n");

            duplicateCout.push_back (std::string{"< ERR duplicate "}
                                     + std::to_string(idx) + "\n");
    }

    checkServerRequest(duplicateRequests, duplicateReplies, duplicateCout,
                       DebugOutput::DebugOff, 15);

    BOOST_CHECK(testDB->getTotalDataSize() == 1000);
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
    std::vector<StringVector> truncateRequests{2};
    std::vector<StringVector> truncateReplies{2};
    StringVector truncateCout{};

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      truncateRequests[0].push_back(std::string{"INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx) + "\n");

             truncateCout.push_back(std::string{"> INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx) + "\n");

       truncateReplies[0].push_back(std::string{"OK\n"});

             truncateCout.push_back (std::string{"< OK\n"});

       truncateRequests[0].push_back(std::string{"INSERT B "}
                                            + std::to_string(idx) + " "
                                            + std::to_string(idx) + "\n");

              truncateCout.push_back(std::string{"> INSERT B "}
                                            + std::to_string(idx) + " "
                                            + std::to_string(idx) + "\n");

        truncateReplies[0].push_back(std::string{"OK\n"});

             truncateCout.push_back (std::string{"< OK\n"});
    }

    for(size_t idx {0}; idx < 1000; ++idx)
    {
       truncateRequests[0].push_back(std::string{"TRUNCATE A\n"});

              truncateCout.push_back(std::string{"> TRUNCATE A\n"});

        truncateReplies[0].push_back(std::string{"OK\n"});

             truncateCout.push_back (std::string{"< OK\n"});

       truncateRequests[0].push_back(std::string{"TRUNCATE B\n"});

              truncateCout.push_back(std::string{"> TRUNCATE B\n"});

        truncateReplies[0].push_back(std::string{"OK\n"});

             truncateCout.push_back (std::string{"< OK\n"});
    }

        checkServerRequest(truncateRequests, truncateReplies, truncateCout,
                           DebugOutput::DebugOff, 15);

        BOOST_CHECK(testDB->getTotalDataSize() == 0);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_truncate_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(multiple_intersection_test)
{
  try
  {
    std::vector<StringVector> intersectionRequests{2};
    std::vector<StringVector> intersectionReplies{2};
    StringVector intersectionCout{};

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      intersectionRequests[0].push_back(std::string{"INSERT A "}
                                     + std::to_string(idx * 2) + " "
                                     + std::to_string(idx * 2) + "\n");

             intersectionCout.push_back(std::string{"> INSERT A "}
                                     + std::to_string(idx * 2) + " "
                                     + std::to_string(idx * 2) + "\n");

       intersectionReplies[0].push_back(std::string{"OK\n"});

             intersectionCout.push_back (std::string{"< OK\n"});

       intersectionRequests[0].push_back(std::string{"INSERT B "}
                                            + std::to_string(idx * 2 + 1) + " "
                                            + std::to_string(idx * 2 + 1) + "\n");

              intersectionCout.push_back(std::string{"> INSERT B "}
                                            + std::to_string(idx * 2 + 1) + " "
                                            + std::to_string(idx * 2 + 1) + "\n");

        intersectionReplies[0].push_back(std::string{"OK\n"});

             intersectionCout.push_back (std::string{"< OK\n"});
    }

    for(size_t idx {8100}; idx < 8200; ++idx)
    {
      intersectionRequests[1].push_back(std::string{"INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx * 2) + "\n");

             intersectionCout.push_back(std::string{"> INSERT A "}
                                     + std::to_string(idx) + " "
                                     + std::to_string(idx * 2) + "\n");

       intersectionReplies[1].push_back(std::string{"OK\n"});

             intersectionCout.push_back (std::string{"< OK\n"});

       intersectionRequests[1].push_back(std::string{"INSERT B "}
                                            + std::to_string(idx) + " "
                                            + std::to_string(idx * 2 + 1) + "\n");

              intersectionCout.push_back(std::string{"> INSERT B "}
                                            + std::to_string(idx) + " "
                                            + std::to_string(idx * 2 + 1) + "\n");

        intersectionReplies[1].push_back(std::string{"OK\n"});

             intersectionCout.push_back (std::string{"< OK\n"});
    }

       intersectionRequests[0].push_back(std::string{"INTERSECTION\n"});

              intersectionCout.push_back(std::string{"> INTERSECTION\n"});

       intersectionRequests[1].push_back(std::string{"INTERSECTION\n"});

              intersectionCout.push_back(std::string{"> INTERSECTION\n"});

    for(size_t idx {8100}; idx < 8200; ++idx)
    {
              intersectionCout.push_back(std::string{"< "}
                                            + std::to_string(idx) + ","
                                            + std::to_string(idx * 2) + ","
                                            + std::to_string(idx * 2 + 1) + "\n");

        intersectionReplies[0].push_back(std::to_string(idx) + ","
                                            + std::to_string(idx * 2) + ","
                                            + std::to_string(idx * 2 + 1) + "\n");

              intersectionCout.push_back(std::string{"< "}
                                            + std::to_string(idx) + ","
                                            + std::to_string(idx * 2) + ","
                                            + std::to_string(idx * 2 + 1) + "\n");

        intersectionReplies[1].push_back(std::to_string(idx) + ","
                                            + std::to_string(idx * 2) + ","
                                           + std::to_string(idx * 2 + 1) + "\n");
    }

    intersectionCout.push_back(std::string{"< OK\n"});
    intersectionReplies[0].push_back(std::string{"OK\n"});
    intersectionCout.push_back(std::string{"< OK\n"});
    intersectionReplies[1].push_back(std::string{"OK\n"});

    checkServerRequest(intersectionRequests, intersectionReplies, intersectionCout,
                       DebugOutput::DebugOff, 15);

    BOOST_CHECK(testDB->getTotalDataSize() == 2200);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_intersections_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(multiple_symmetric_difference_test)
{
  try
  {
    std::vector<StringVector> symmetricDifferenceRequests{4};
    std::vector<StringVector> symmetricDifferenceReplies{4};
    StringVector symmetricDifferenceCout{};

    for(size_t idx {0}; idx < 1000; ++idx)
    {
      symmetricDifferenceRequests[0].push_back(std::string{"INSERT A "}
                                                   + std::to_string(idx) + " "
                                                   + std::to_string(idx) + "\n");

             symmetricDifferenceCout.push_back(std::string{"> INSERT A "}
                                                  + std::to_string(idx) + " "
                                                  + std::to_string(idx) + "\n");

       symmetricDifferenceReplies[0].push_back(std::string{"OK\n"});

             symmetricDifferenceCout.push_back (std::string{"< OK\n"});

       symmetricDifferenceRequests[1].push_back(std::string{"INSERT B "}
                                                   + std::to_string(idx) + " "
                                                   + std::to_string(idx) + "\n");

              symmetricDifferenceCout.push_back(std::string{"> INSERT B "}
                                                   + std::to_string(idx) + " "
                                                   + std::to_string(idx) + "\n");

        symmetricDifferenceReplies[1].push_back(std::string{"OK\n"});

             symmetricDifferenceCout.push_back (std::string{"< OK\n"});
    }



    for(size_t idx {8010}; idx < 8020; ++idx)
    {
      symmetricDifferenceRequests[0].push_back(std::string{"INSERT A "}
                                                  + std::to_string(idx * 2) + " "
                                                  + std::to_string(idx * 2) + "\n");

             symmetricDifferenceCout.push_back(std::string{"> INSERT A "}
                                                  + std::to_string(idx * 2) + " "
                                                  + std::to_string(idx * 2) + "\n");

       symmetricDifferenceReplies[0].push_back(std::string{"OK\n"});

             symmetricDifferenceCout.push_back (std::string{"< OK\n"});

       symmetricDifferenceRequests[1].push_back(std::string{"INSERT B "}
                                                   + std::to_string(idx * 2 + 1) + " "
                                                   + std::to_string(idx * 2 + 1) + "\n");

              symmetricDifferenceCout.push_back(std::string{"> INSERT B "}
                                                   + std::to_string(idx * 2 + 1) + " "
                                                   + std::to_string(idx * 2 + 1) + "\n");

        symmetricDifferenceReplies[1].push_back(std::string{"OK\n"});

             symmetricDifferenceCout.push_back (std::string{"< OK\n"});
    }

       symmetricDifferenceRequests[2].push_back(std::string{"SYMMETRIC_DIFFERENCE\n"});

              symmetricDifferenceCout.push_back(std::string{"> SYMMETRIC_DIFFERENCE\n"});




       symmetricDifferenceRequests[3].push_back(std::string{"SYMMETRIC_DIFFERENCE\n"});

              symmetricDifferenceCout.push_back(std::string{"> SYMMETRIC_DIFFERENCE\n"});

    for(size_t idx {8010}; idx < 8020; ++idx)
    {
             symmetricDifferenceCout.push_back(std::string{"< "}
                                                   + std::to_string(idx * 2) + ","
                                                   + std::to_string(idx * 2) + ","
                                                   + "\n");

        symmetricDifferenceReplies[2].push_back(std::to_string(idx * 2) + ","
                                                   + std::to_string(idx * 2) + ","
                                                   + "\n");

              symmetricDifferenceCout.push_back(std::string{"< "}
                                                   + std::to_string(idx * 2) + ","
                                                   + std::to_string(idx * 2) + ","
                                                   + "\n");

        symmetricDifferenceReplies[3].push_back(std::to_string(idx * 2) + ","
                                                   + std::to_string(idx * 2) + ","
                                                   + "\n");

              symmetricDifferenceCout.push_back(std::string{"< "}
                                                   + std::to_string(idx * 2 + 1) + ","
                                                   + ","
                                                   + std::to_string(idx * 2 + 1) + "\n");

        symmetricDifferenceReplies[2].push_back(std::to_string(idx * 2 + 1) + ","
                                                   + ","
                                                   + std::to_string(idx * 2 + 1) + "\n");

              symmetricDifferenceCout.push_back(std::string{"< "}
                                                   + std::to_string(idx * 2 + 1) + ","
                                                   + ","
                                                   + std::to_string(idx * 2 + 1) + "\n");

        symmetricDifferenceReplies[3].push_back(std::to_string(idx * 2 + 1) + ","
                                                   + ","
                                                   + std::to_string(idx * 2 + 1) + "\n");
    }

    symmetricDifferenceCout.push_back(std::string{"< OK\n"});
    symmetricDifferenceReplies[2].push_back(std::string{"OK\n"});
    symmetricDifferenceCout.push_back(std::string{"< OK\n"});
    symmetricDifferenceReplies[3].push_back(std::string{"OK\n"});

    checkServerRequest(symmetricDifferenceRequests,
                       symmetricDifferenceReplies,
                       symmetricDifferenceCout,
                       DebugOutput::DebugOff, 200);

    BOOST_CHECK(testDB->getTotalDataSize() == 2020);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "multiple_symmetric_difference_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(server_stop)
{
  try
  {
    testServer.stop();
    testWork.reset();

    for (auto& thread : testWorkingThreads)
    {
      if(thread.joinable() == true)
      {
        thread.join();
      };
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "server_stop failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
