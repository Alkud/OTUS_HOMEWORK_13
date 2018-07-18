// async_command_server.h in Otus homework#13 project

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <csignal>
#include <iostream>
#include <boost/asio.hpp>
#include "async_acceptor.h"
#include "server_common_types.h"
#include "db_manager.h"

template<size_t workinThreadCount = 2>
class AsyncJoinServer
{
public:

  AsyncJoinServer() = delete;

  AsyncJoinServer(
    const asio::ip::address_v4 newAddress,
    const uint16_t newPortNumber,
    std::ostream& newOutputStream,
    std::ostream& newErrorStream
  ) :
  address{newAddress},
  portNumber{newPortNumber},

  outputStream{newOutputStream},
  errorStream{newErrorStream},

  service{ new asio::io_service},
  work {new asio::io_service::work(*service)},

  isStarted{false},
  shouldExit{false},

  terminationLock{},
  terminationNotifier{},
  acceptorStopped{true},
  dbManagerStopped{true},

  asyncAcceptor{ new AsyncAcceptor (
    newAddress,
    newPortNumber,    
    service,
    terminationNotifier,
    acceptorStopped,
    newOutputStream,
    newErrorStream,
    outputLock
  )},

  workingThreads{},

  database{ new NaiveDB },

  dbManager{ new DbManager (
    database,
    terminationNotifier,
    dbManagerStopped,
    outputStream,
    errorStream,
    outputLock
  )

  }

  {}

  ~AsyncJoinServer()
  {
    #ifdef NDEBUG
    #else
      //std::cout << "-- Server destructor\n";
    #endif

    stop();
  }

  void start()
  {
    if (isStarted.load() == true)
    {
      return;
    }

    isStarted.store(true);

    asyncAcceptor->start();

    acceptorStopped.store(false);

    for (size_t idx{0}; idx < workinThreadCount; ++idx)
    {
      workingThreads.push_back(std::thread{&AsyncJoinServer::run, this, service});
    }
  }

  void stop()
  {
    if (shouldExit.load() == true)
    {
      return;
    }

    shouldExit.store(true);

    #ifdef NDEBUG
    #else
      //std::cout << "-- called Server::stop()\n";
    #endif    

    std::this_thread::sleep_for(1s);

    asyncAcceptor->stop();    

    #ifdef NDEBUG
    #else
      //std::cout << "-- waiting Acceptor termination. Termination flag: "
      //          << std::boolalpha << acceptorStopped.load() << "\n";
    #endif

    while (acceptorStopped.load() != true)
    {
      #ifdef NDEBUG
      #else
        //std::cout << "-- waiting Acceptor termination. Termination flag: "
        //          << std::boolalpha << acceptorStopped.load() << "\n";
      #endif

      std::unique_lock<std::mutex> lockTermination{terminationLock};
      terminationNotifier.wait_for(lockTermination, 100ms,[this]()
      {
        return acceptorStopped.load() == true;
      });
    }

    work.reset();

    for (auto& thread : workingThreads)
    {
      if (thread.joinable() == true)
      {
        thread.join();
      }
    }

    service->stop();

    while(service->stopped() != true)
    {}

    #ifdef NDEBUG
    #else
      //std::cout << "-- Server stopped\n";
    #endif
  }

  void processDbReply(
    SharedStringVector message,
    SharedSocket
  )
  {
    asio::async_write(*socket, asio::buffer(*message),
    [this, message](const system::error_code& error, std::size_t bytes_transferred)
    {
      if (!error)
      {
        std::lock_guard<std::mutex> lockOutput{outputLock};
        outputStream << message;
      }
     });
  }

  SharedConstNaiveDB getDB()
  {
    return std::const_pointer_cast<const NaiveDB>(database);
  }

  std::mutex& getScreenOutputLock()
  {
    return outputLock;
  }

private:

  void run(SharedService service) noexcept
  {
    try
    {
      service->run();
    }
    catch (const std::exception& ex)
    {
      std::lock_guard<std::mutex> lockOutput{outputLock};
      errorStream << "Server stopped. Reason: " << ex.what() << '\n';
    }
  }


  asio::ip::address_v4 address;
  uint16_t portNumber;

  std::ostream& outputStream;
  std::ostream& errorStream;

  SharedService service;
  UniqueWork work;


  std::atomic<bool> isStarted;
  std::atomic<bool> shouldExit;

  std::mutex terminationLock;
  std::condition_variable terminationNotifier;
  std::atomic<bool> acceptorStopped;
  std::atomic<bool> dbManagerStopped;

  std::unique_ptr<AsyncAcceptor> asyncAcceptor;

  std::vector<std::thread> workingThreads;

  SharedNaiveDB database;
  UniqueDbManager dbManager;

  static std::mutex outputLock;
};

template<size_t workinThreadCount>
std::mutex AsyncJoinServer<workinThreadCount>::outputLock {};
