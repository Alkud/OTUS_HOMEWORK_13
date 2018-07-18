// async_acceptor.cpp in Otus homework#13 project

#include "async_acceptor.h"

#include <string>
#include <vector>
#include <chrono>
#include <csignal>
#include <boost/bind.hpp>

using namespace std::chrono_literals;

AsyncAcceptor::AsyncAcceptor(const asio::ip::address_v4 newAddress,
  const uint16_t newPortNumber,
  asio::io_service& newService,
  std::condition_variable& newTerminationNotifier,
  std::atomic<bool>& newTerminationFlag,
  std::ostream& newErrorStream, std::mutex& newOutputLock) :

address{newAddress}, portNumber{newPortNumber}, service{newService},
endpoint{address, portNumber}, acceptor{service, endpoint},

currentReader{}, activeReaderCount{},
terminationLock{},

terminationNotifier{newTerminationNotifier},
terminationFlag{newTerminationFlag},

shouldExit{false},
errorStream{newErrorStream},
outputLock{newOutputLock}
{}

void AsyncAcceptor::start()
{  
  terminationFlag.store(false);
  doAccept();
}

void AsyncAcceptor::stop()
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- Acceptor stop\n";
  #endif

  shouldExit.store(true);

  if (acceptor.is_open())
  {
    #ifdef NDEBUG
    #else
      //std::cout << "-- acceptor close\n";
    #endif

    acceptor.close();
  }

  while (activeReaderCount.load() != 0)
  {
    std::unique_lock<std::mutex> lockTermination{terminationLock};
    terminationNotifier.wait_for(lockTermination, 100ms, [this]()
    {
      #ifdef NDEBUG
      #else
        //std::cout << "\n-- Acceptor waiting. Active readers: " << activeReaderCount.load() << "\n";
      #endif

      return activeReaderCount.load() == 0;
    });
    lockTermination.unlock();
  }

  #ifdef NDEBUG
  #else
    //std::cout << "-- acceptor sets termination flag\n";
  #endif

  terminationFlag.store(true);
  terminationNotifier.notify_all();
}

void AsyncAcceptor::doAccept()
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- start doAccept\n";
  #endif

  auto socket {std::make_shared<asio::ip::tcp::socket>(service)};

  acceptor.async_accept(*socket.get(), [this, socket](const system::error_code& error)
  {
    if (!error)
    {
      if (shouldExit.load() != true)
      {
        onAcception(socket);
      }
    }
    else if (error != asio::error::operation_aborted)
    {
      std::lock_guard<std::mutex> lockOutput{outputLock};

      errorStream << "Acceptor stopped. Reason: "
                  << error.message()
                  << ". Error code: " << error.value() << '\n';
    }
  });
}

void AsyncAcceptor::onAcception(SharedSocket acceptedSocket)
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- start onAcception\n";
  #endif

  currentReader.reset( new AsyncReader(
    acceptedSocket,
    //processor, openDelimiter, closeDelimiter,
    acceptor, activeReaderCount,
    terminationNotifier, terminationLock,
    errorStream, outputLock,
    shouldExit
  ));

  currentReader->start();

  if (shouldExit.load() != true)
  {
    doAccept();
  }
}
