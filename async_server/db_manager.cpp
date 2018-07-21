#include "db_manager.h"
#include <iostream>

DbManager::DbManager(SharedNaiveDB&& newStorage,
                     std::condition_variable& newTerminationNotifier,
                     std::atomic<bool>& newTerminationFlag,
                     std::ostream& newOutputStream,
                     std::ostream& newErrorStream,
                     std::mutex& newOutputLock) :
  storage{},

  dispatcher{ new asio::io_service },
  dispatcherWork { new asio::io_service::work(*dispatcher) },

  processor{ new asio::io_service },
  processorWork { new asio::io_service::work(*processor) },

  dispatchingThread{},
  processingThreads{},

  terminationNotifier{newTerminationNotifier},
  terminationFlag{newTerminationFlag},
  isStarted{false},
  shouldExit{false},

  outputStream{newOutputStream},
  errorStream{newErrorStream},
  outputLock{newOutputLock}
{
  storage.swap(newStorage);
}

DbManager::DbManager(const SharedNaiveDB& newStorage,
                     std::condition_variable& newTerminationNotifier,
                     std::atomic<bool>& newTerminationFlag,
                     std::ostream& newOutputStream,
                     std::ostream& newErrorStream,
                     std::mutex& newOutputLock):
  storage{newStorage},

  dispatcher{ new asio::io_service },
  dispatcherWork { new asio::io_service::work(*dispatcher) },

  processor{ new asio::io_service },
  processorWork { new asio::io_service::work(*processor) },

  dispatchingThread{},
  processingThreads{},

  terminationNotifier{newTerminationNotifier},
  terminationFlag{newTerminationFlag},
  isStarted{false},
  shouldExit{false},

  outputStream{newOutputStream},
  errorStream{newErrorStream},
  outputLock{newOutputLock}
{}

DbManager::DbManager(std::condition_variable& newTerminationNotifier,
                     std::atomic<bool>& newTerminationFlag,
                     std::ostream& newOutputStream,
                     std::ostream& newErrorStream,
                     std::mutex& newOutputLock):

  storage{ new NaiveDB{}},

  dispatcher{ new asio::io_service },
  dispatcherWork { new asio::io_service::work(*dispatcher) },

  processor{ new asio::io_service },
  processorWork { new asio::io_service::work(*processor) },

  dispatchingThread{},
  processingThreads{},

  terminationNotifier{newTerminationNotifier},
  terminationFlag{newTerminationFlag},
  shouldExit{false},

  outputStream{newOutputStream},
  errorStream{newErrorStream},
  outputLock{newOutputLock}
{}

void DbManager::start()
{
  if (isStarted.load() == true)
  {
    return;
  }

  isStarted.store(true);

  buildHandlers();

  dispatchingThread = std::thread {&DbManager::run, this, dispatcher};

  for (size_t idx{0}; idx < DEFAULT_THREAD_COUNT; ++idx)
  {
    processingThreads.push_back(std::thread{&DbManager::run, this, processor});
  }

  if (storage->getTotalDataSize() > SIZE_THRESHOLD)
  {
    addProcessingThreads();
  }
}

void DbManager::stop()
{
  if (shouldExit.load() == true)
  {
    return;
  }

  shouldExit.store(true);

  dispatcherWork.reset();

  if (dispatchingThread.joinable() == true)
  {
    dispatchingThread.join();
  }

  dispatcher->stop();

  while(dispatcher->stopped() != true)
  {}

  processorWork.reset();

  for (auto& thread : processingThreads)
  {
    if (thread.joinable() == true)
    {
      thread.join();
    }
  }

  processor->stop();

  while(processor->stopped() != true)
  {}

}

void DbManager::adoptDb(const SharedNaiveDB& newDb)
{
  storage = newDb;
}

void DbManager::swapDb(SharedNaiveDB& newDb)
{
  storage.swap(newDb);
}

SharedNaiveDB DbManager::getDb()
{
  return storage;
}


void DbManager::processRequest(const SharedDbCommandReaction& request, ServerReplyCallback callback)
{
  if (shouldExit.load() == true)
  {
    return;
  }

  std::unique_lock<std::mutex> lockAccess{accessLock};  

  if (storage->getTotalDataSize() > SIZE_THRESHOLD)
  {
    addProcessingThreads();
  }

  auto commandCode {std::get<0>(*request)};
  auto commandArguments {std::get<1>(*request)};
  auto socket {std::get<2>(*request)};

  dispatcher->post([this, commandCode, commandArguments, socket, callback]()
  {
    dispatchingHandlers[commandCode](commandArguments, socket, callback);
  });
}














void DbManager::buildHandlers()
{
  /*--------------------------------------------------------------*/
  dispatchingHandlers[DbCommands::EMPTY] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    SharedStringVector message { new StringVector ()};

    message->push_back(arguments[0]);

    callback(message, socket);
  };

  /*--------------------------------------------------------------*/
  dispatchingHandlers[DbCommands::INSERT] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    std::string table{arguments[0]};
    int id{std::stoi(arguments[1])};
    std::string name{arguments[2]};

    SharedStringVector message { new StringVector ()};

    auto insertResult {storage->insertNameToTable(table, id, name)};

    if (DbOpResult::OK == insertResult)
    {
      message->push_back("OK\n");
    }
    else if (DbOpResult::ID_DUPLICATE == insertResult)
    {
      message->push_back(std::string{"ERR duplicate "} + arguments[1] + "\n");
    }
    else
    {
      message->push_back(std::string{"ERR not_found "} + table + "\n");
    }

    callback(message, socket);
  };

  /*--------------------------------------------------------------*/
  dispatchingHandlers[DbCommands::INTERSECTION] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    processor->post([this, arguments, socket, callback]()
    {
      processingHandlers[DbCommands::INTERSECTION](
        arguments,socket,callback);
    });
  };

  /*--------------------------------------------------------------*/
  dispatchingHandlers[DbCommands::SYMMETRIC_DIFFERENCE] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    processor->post([this, arguments, socket, callback]()
    {
      processingHandlers[DbCommands::SYMMETRIC_DIFFERENCE](
        arguments,socket,callback);
    });
  };

  /*--------------------------------------------------------------*/
  dispatchingHandlers[DbCommands::TRUNCATE] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    std::string table{arguments[0]};

    SharedStringVector message { new StringVector ()};

    if (true == storage->clearTable(table))
    {
      message->push_back("OK\n");
    }
    else
    {
      message->push_back(std::string{"ERR not_found "} + table + "\n");
    }

    callback(message, socket);
  };

  /*--------------------------------------------------------------*/
  processingHandlers[DbCommands::INTERSECTION] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    auto tableA{arguments[0]};
    auto tableB{arguments[1]};

    SharedStringVector message { new StringVector () };

    if (storage->containsTable(tableA) != true)
    {
      message->push_back(std::string{"ERR not_found "} + tableA + "\n");
      callback(message, socket);
      return;
    }
    else if (storage->containsTable(tableB) != true)
    {
      message->push_back(std::string{"ERR not_found "} + tableB + "\n");
      callback(message, socket);
      return;
    }

    auto intersectionResult {storage->getIntersection(tableA, tableB)};

    std::string resultString{};

    for (const auto& resultElement : *intersectionResult)
    {
      resultString = std::to_string(resultElement.first)          // id
                     + "," + resultElement.second.first           // table A element
                     + "," + resultElement.second.second + "\n";  // table B element

      message->push_back(resultString);
    }

    message->push_back("OK\n");

    callback(message, socket);
  };

  processingHandlers[DbCommands::SYMMETRIC_DIFFERENCE] = [this](
    const std::vector<std::string>& arguments, const SharedSocket& socket, ServerReplyCallback callback)
  {
    auto tableA{arguments[0]};
    auto tableB{arguments[1]};

    SharedStringVector message { new StringVector () };

    if (storage->containsTable(tableA) != true)
    {
      message->push_back(std::string{"ERR not_found "} + tableA + "\n");
      callback(message, socket);
      return;
    }
    else if (storage->containsTable(tableB) != true)
    {
      message->push_back(std::string{"ERR not_found "} + tableB + "\n");
      callback(message, socket);
      return;
    }

    auto symmetricDifferenceResult {storage->getSymmetricDifference(tableA, tableB)};

    std::string resultString{};

    for (const auto& resultElement : *symmetricDifferenceResult)
    {
      resultString = std::to_string(resultElement.first)          // id
                     + "," + resultElement.second.first           // table A element
                     + "," + resultElement.second.second + "\n";  // table B element

      message->push_back(resultString);
    }

    message->push_back("OK\n");

    callback(message, socket);
  };
}


void DbManager::run(SharedService service) noexcept
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

void DbManager::addProcessingThreads()
{
  for (size_t idx{0}; idx < THREAD_COUNT_INCREMENT; ++idx)
  {
    processingThreads.push_back(std::thread{&DbManager::run, this, processor});
  }
}
