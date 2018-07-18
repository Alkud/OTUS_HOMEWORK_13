#pragma once

#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include "server_common_types.h"
#include "naive_db.h"

const size_t SIZE_THRESHOLD = 1000;
const size_t DEFAULT_THREAD_COUNT = 2;
const size_t THREAD_COUNT_INCREMENT = 2;


class DbManager
{
public:
  DbManager(SharedNaiveDB&& newStorage,
            std::condition_variable& newTerminationNotifier,
            std::atomic<bool>& newTerminationFlag,
            std::ostream& newOutputStream,
            std::ostream& newErrorStream,
            std::mutex& newOutputLock);

  DbManager(const SharedNaiveDB& newStorage,
            std::condition_variable& newTerminationNotifier,
            std::atomic<bool>& newTerminationFlag,
            std::ostream& newOutputStream,
            std::ostream& newErrorStream,
            std::mutex& newOutputLock);

  DbManager(std::condition_variable& newTerminationNotifier,
            std::atomic<bool>& newTerminationFlag,
            std::ostream& newOutputStream,
            std::ostream& newErrorStream,
            std::mutex& newOutputLock);


  DbManager() = delete;
  DbManager(DbManager&& other) = delete;
  DbManager(const DbManager& other) = delete;

  DbManager& operator=(DbManager&& other) = delete;
  DbManager& operator=(const DbManager& other) = delete;

  void start();

  void stop();

  void processRequest(const CommandReaction& request);

private:

  std::map<DBCommands, std::function<
    void(const std::vector<std::string>, const SharedSocket)>
  > dispatchingHandlers;

  std::map<DBCommands, std::function<
    void(const std::vector<std::string>, const SharedSocket)>
  > processingHandlers;

  void buildHandlers();

  void run(SharedService service) noexcept;

  void addProcessingThreads();

  SharedNaiveDB storage;

  SharedService dispatcher;
  UniqueWork dispatcherWork;

  SharedService processor;
  UniqueWork processorWork;

  std::mutex accessLock{};
  std::thread dispatchingThread;
  std::vector<std::thread> processingThreads;

  std::condition_variable& terminationNotifier;
  std::atomic<bool>& terminationFlag;

  std::atomic<bool> isStarted;
  std::atomic<bool> shouldExit;

  std::ostream& outputStream;
  std::ostream& errorStream;
  std::mutex& outputLock;
};
