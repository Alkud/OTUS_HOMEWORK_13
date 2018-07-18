// async_reader.h in Otus homework#13 project

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include "server_common_types.h"

using namespace boost;

constexpr size_t READ_BUFFER_SIZE = 4096;

class AsyncReader : public std::enable_shared_from_this<AsyncReader>
{
public:

  using SharedSocket = std::shared_ptr<asio::ip::tcp::socket>;  

  AsyncReader() = delete;

  AsyncReader(SharedSocket newSocket,
              asio::ip::tcp::acceptor& newAcceptor,
              std::atomic<size_t>& newReaderCounter,
              std::condition_variable& newTerminationNotifier,
              std::mutex& newTerminationLock,
              std::ostream& newErrorStream, std::mutex& newOutputLock,
              std::atomic<bool>& stopFlag);

  ~AsyncReader();

  void start();

  void stop();

private:

  void doRead();

  void onReading(std::size_t bytes_transferred, SharedSocket socket);

  SharedSocket socket;  

  asio::streambuf readBuffer;
  std::vector<std::string> receivedCommand;

  asio::ip::tcp::acceptor& acceptor;
  std::atomic<size_t>& readerCounter;

  std::condition_variable& terminationNotifier;
  std::mutex& terminationLock;

  std::ostream& errorStream;
  std::mutex& outputLock;

  std::shared_ptr<AsyncReader> sharedThis;

  std::atomic<bool>& shouldExit;

  std::atomic<bool> stopped;
  std::thread controller;
  std::condition_variable controllerNotifier;
};

using SharedAsyncReader = std::shared_ptr<AsyncReader>;
