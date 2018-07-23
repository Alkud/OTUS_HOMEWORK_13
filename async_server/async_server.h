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
#include "db_command_translator.h"
#include "db_manager.h"

template<size_t workingThreadCount = 2>
class AsyncJoinServer
{
public:

  AsyncJoinServer() = delete;

  AsyncJoinServer(
    const asio::ip::address_v4 newAddress,
    const uint16_t newPortNumber,
    std::ostream& newOutputStream,
    std::ostream& newErrorStream,
    std::atomic<bool>& newAppTerminationFlag,
    std::condition_variable& newAppTerminationNotifier
  ) :
  address{newAddress},
  portNumber{newPortNumber},

  outputStream{newOutputStream},
  errorStream{newErrorStream},

  netService{ new asio::io_service},
  netWork {new asio::io_service::work(*netService)},

  requestService{ new asio::io_service},
  requestWork {new asio::io_service::work(*requestService)},

  isStarted{false},
  shouldExit{false},

  terminationLock{},
  terminationNotifier{},
  acceptorStopped{true},
  dbManagerStopped{true},

  asyncAcceptor{ new AsyncAcceptor (
    newAddress,
    newPortNumber,    
    netService,
    requestService,

    [this](const SharedDbCommandReaction& request)
    {
      processDbRequest(request);
    },

    terminationNotifier,
    acceptorStopped,
    newOutputStream,
    newErrorStream,
    outputLock
  )},

  netThreads{},
  requestThread{},

  database{ new NaiveDB },

  dbManager{ new DbManager (
    database,
    terminationNotifier,
    dbManagerStopped,
    outputStream,
    errorStream,
    outputLock
  )},

  appTerminationFlag{newAppTerminationFlag},
  appTerminationNotifier{newAppTerminationNotifier}

  {}

  ~AsyncJoinServer()
  {
    #ifdef NDEBUG
    #else
      //std::cout << "-- Server destructor\n";
    #endif

    stop();

    if (controller.joinable())
    {
      controller.join();
    }
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

    for (size_t idx{0}; idx < workingThreadCount; ++idx)
    {
      netThreads.push_back(std::thread{&AsyncJoinServer<workingThreadCount>::run, this, netService});
    }

    requestThread = std::thread{&AsyncJoinServer::run, this, requestService};

    dbManager->start();

    dbManagerStopped.store(false);

    controller =std::thread{[this]()
    {
      std::unique_lock<std::mutex> lockTermination{terminationLock};
      terminationNotifier.wait(lockTermination,[this]()
      {
        return (acceptorStopped.load() == true
                || dbManagerStopped.load() == true);
      });

      if (dbManagerStopped.load() == true)
      {
        stop();
        appTerminationFlag.store(true);
        appTerminationNotifier.notify_all();
      }
    }};
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

    netWork.reset();

    asyncAcceptor->stop();

    for (auto& thread : netThreads)
    {
      if (thread.joinable() == true)
      {
        thread.join();
      }
    }

    netService->stop();

    while(netService->stopped() != true)
    {}


    requestWork.reset();

    if (requestThread.joinable() == true)
    {
      requestThread.join();
    }

    requestService->stop();

    while(requestService->stopped() != true)
    {}

    dbManager->stop();

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

    while (dbManagerStopped.load() != true)
    {
      std::unique_lock<std::mutex> lockTermination{terminationLock};
      terminationNotifier.wait_for(lockTermination, 100ms,[this]()
      {
        return dbManagerStopped.load() == true;
      });
    }

    #ifdef NDEBUG
    #else
      //std::cout << "-- Server stopped\n";
    #endif
  }

  SharedNaiveDB getDb()
  {
    return database;
  }

  SharedConstNaiveDB getDb() const
  {
    return std::const_pointer_cast<const NaiveDB>(database);
  }

  void adoptDb(const SharedNaiveDB& newDb)
  {
    database = newDb;
    dbManager->adoptDb(database);
  }

  void swapDb(SharedNaiveDB& newDb)
  {
    database.swap(newDb);
    dbManager->adoptDb(database);
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
      stop();
    }
  }

  void processDbRequest(const SharedDbCommandReaction& request)
  {
    std::lock_guard<std::mutex> lockOutput{outputLock};
    outputStream << "> " << DbCommandTranslator::restore(*request) << "\n";

    dbManager->processRequest(request,
      [this](const SharedStringVector& message, const SharedSocket& socket)
    {
      //std::cout << "process db request" << (*message)[0] << "\n";
      processDbReply(message, socket);
    });
  }

  void processDbReply(
    const SharedStringVector& message,
    const SharedSocket& socket
  )
  {
    for (const auto& writeString : *message)
    {
      asio::async_write(*socket, asio::buffer(writeString),
      [this, writeString, socket](const system::error_code& error, std::size_t bytes_transferred)
      {
        if (!error)
        {
          std::lock_guard<std::mutex> lockOutput{outputLock};
          outputStream << "< " << writeString;

          //outputStream << "socket use count: " << socket.use_count() << "\n";

          //socket->shutdown(asio::ip::tcp::socket::shutdown_send);
        }
      });
    }
  }


  asio::ip::address_v4 address;
  uint16_t portNumber;

  std::ostream& outputStream;
  std::ostream& errorStream;

  SharedService netService;
  UniqueWork netWork;

  SharedService requestService;
  UniqueWork requestWork;


  std::atomic<bool> isStarted;
  std::atomic<bool> shouldExit;

  std::mutex terminationLock;
  std::condition_variable terminationNotifier;
  std::atomic<bool> acceptorStopped;
  std::atomic<bool> dbManagerStopped;

  std::unique_ptr<AsyncAcceptor> asyncAcceptor;

  std::vector<std::thread> netThreads;
  std::thread requestThread;

  SharedNaiveDB database;
  UniqueDbManager dbManager;

  static std::mutex outputLock;

  std::thread controller{};
  std::atomic<bool>& appTerminationFlag;
  std::condition_variable& appTerminationNotifier;
};

template<size_t workingThreadCount>
std::mutex AsyncJoinServer<workingThreadCount>::outputLock {};
