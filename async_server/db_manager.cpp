#include "db_manager.h"

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

  processor->stop();

  while(processor->stopped() != true)
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


void DbManager::processRequest(const CommandReaction& request)
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

  auto commandCode {std::get<0>(request)};
  auto commandArguments {std::get<1>(request)};
  auto socket {std::get<2>(request)};
}














void DbManager::buildHandlers()
{
  /*--------------------------------------------------------------*/
  dispatchingHandlers[DBCommands::EMPTY] = [this](
    const std::vector<std::string> arguments, const SharedSocket socket)
  {
    std::string message{arguments[0]};

    asio::async_write(*socket, asio::buffer(message),
    [this](const system::error_code& error, std::size_t bytes_transferred)
    {
      if (!error)
      {
        std::lock_guard<std::mutex> lockOutput{outputLock};
      }
     });
  };

  /*--------------------------------------------------------------*/
  dispatchingHandlers[DBCommands::INSERT] = [this](
    const std::vector<std::string> arguments, const SharedSocket socket)
  {
    std::string table{arguments[0]};
    int id{std::stoi(arguments[1])};
    std::string name{arguments[2]};

    std::string message{};

    if (storage->insertNameToTable(table, id, name))
    {
      message = "OK\n";
    }
    else
    {
      message = std::string{"ERR duplicate "} + arguments[1];
    }

    asio::async_write(*socket, asio::buffer(message),
    [this, message](const system::error_code& error, std::size_t bytes_transferred)
    {
      if (!error)
      {
        std::lock_guard<std::mutex> lockOutput{outputLock};
        outputStream << message;
      }
     });
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
