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

  service{},
  work {new asio::io_service::work(service)},

  terminationLock{},
  terminationNotifier{},
  acceptorStopped{true},

  asyncAcceptor{ new AsyncAcceptor (
    newAddress,
    newPortNumber,
    newOutputStream,
    service,
    terminationNotifier,
    acceptorStopped,
    outputLock
  )}
  {}

  ~AsyncJoinServer()
  {
    #ifdef NDEBUG
    #else
      //std::cout << "-- Server destructor\n";
    #endif

    if (acceptorStopped.load() != true)
    {
      stop();
    }
  }

  void start()
  {
    asyncAcceptor->start();

    acceptorStopped.store(false);

    for (size_t idx{0}; idx < workinThreadCount; ++idx)
    {
      workingThreads.push_back(std::thread{&AsyncJoinServer::run, this, service});
    }
  }

  void stop()
  {
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

    service.stop();

    while(service.stopped() != true)
    {}

    #ifdef NDEBUG
    #else
      //std::cout << "-- Server stopped\n";
    #endif
  }

  std::mutex& getScreenOutputLock()
  {
    return outputLock;
  }

private:

  void run(asio::io_service& service) noexcept
  {
    try
    {
      service.run();
    }
    catch (const std::exception& ex)
    {
      std::lock_guard<std::mutex> lockOutput{outputLock};
      std::cerr << "Server stopped. Reason: " << ex.what() << '\n';
    }
  }


  asio::ip::address_v4 address;
  uint16_t portNumber;

  std::ostream& outputStream;
  std::ostream& errorStream;

  asio::io_service service;
  UniqueWork work;

  std::mutex terminationLock;
  std::condition_variable terminationNotifier;
  std::atomic<bool> acceptorStopped;

  std::unique_ptr<AsyncAcceptor> asyncAcceptor;

  std::vector<std::thread> workingThreads;

  static std::mutex outputLock;
};

template<size_t workinThreadCount>
std::mutex AsyncJoinServer<workinThreadCount>::outputLock {};
