// async_acceptor.h in Otus homework#13 project

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <array>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include "async_reader.h"
#include "server_common_types.h"


class AsyncAcceptor
{
public:

  AsyncAcceptor() = delete;

  AsyncAcceptor(const asio::ip::address_v4 newAddress,
                const uint16_t newPortNumber,                
                const SharedService& newService,
                std::condition_variable& newTerminationNotifier,
                std::atomic<bool>& newTerminationFlag,
                std::ostream& newOutputStream,
                std::ostream& newErrorStream,
                std::mutex& newOutputLock);

  void start();

  void stop();

private:

  void doAccept();
  void onAcception(SharedSocket acceptedSocket);

  asio::ip::address_v4 address;
  uint16_t portNumber;
  SharedService service;
  asio::ip::tcp::endpoint endpoint;  
  asio::ip::tcp::acceptor acceptor;  

  SharedAsyncReader currentReader;

  std::atomic<size_t> activeReaderCount;
  std::mutex terminationLock;

  std::condition_variable& terminationNotifier;
  std::atomic<bool>& terminationFlag;

  std::atomic<bool> isStarted;
  std::atomic<bool> shouldExit;

  std::ostream& outputStream;
  std::ostream& errorStream;
  std::mutex& outputLock;
};
