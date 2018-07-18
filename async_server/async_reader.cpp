// async_reader.cpp in Otus homework#13 project

#include "async_reader.h"
#include <chrono>
#include <csignal>
#include <iterator>
#include "command_translator.h"

using namespace std::chrono_literals;

AsyncReader::AsyncReader(AsyncReader::SharedSocket newSocket,  
  asio::ip::tcp::acceptor& newAcceptor,
  std::atomic<size_t>& newReaderCounter,
  std::condition_variable& newTerminationNotifier,
  std::mutex& newTerminationLock,
  std::ostream& newErrorStream,
  std::mutex& newOutputLock,
  std::atomic<bool>& stopFlag
):
  socket{newSocket},
  readBuffer{}, receivedCommand{},
  acceptor{newAcceptor}, readerCounter{newReaderCounter},
  terminationNotifier{newTerminationNotifier},
  terminationLock{newTerminationLock},
  errorStream{newErrorStream}, outputLock{newOutputLock},
  sharedThis{},
  shouldExit{stopFlag},

  stopped{false}, controller{}, controllerNotifier{}
{
  ++readerCounter;
}

AsyncReader::~AsyncReader()
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- reader destructor\n";
  #endif

  if (controller.joinable())
  {
    controller.detach();
  }
}

void AsyncReader::start()
{
  if (nullptr == sharedThis)
  {
    sharedThis = shared_from_this();
  }
  else
  {
    return;
  }

  controller = std::thread{[this]()
  {
      while (shouldExit.load() != true && stopped.load() != true)
      {
        std::mutex dummyMutex{};
        std::unique_lock<std::mutex> dummyLock{dummyMutex};
        controllerNotifier.wait_for(dummyLock, 1s, [this]()
        {
          return shouldExit.load() == true || stopped.load() == true;
        });
      }

      if (shouldExit.load() == true
          && socket != nullptr
          && socket->is_open())
      {
        stop();
      }

      #ifdef NDEBUG
      #else
        //std::cout << "-- reader controller EXIT\n";
      #endif
  }};

  doRead();  
}

void AsyncReader::stop()
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- reader stop\n";
  #endif

  stopped.store(true);
  controllerNotifier.notify_one();

  if (socket != nullptr)
  {    
    if (socket->is_open())
    {
      #ifdef NDEBUG
      #else
       //std::cout << "-- reader socket shutdown\n";
      #endif

      socket->shutdown(asio::ip::tcp::socket::shutdown_both);

      #ifdef NDEBUG
      #else
        //std::cout << "-- reader socket close\n";
      #endif
    }

    if (readerCounter.load() != 0)
    {
      --readerCounter;
    }

    terminationNotifier.notify_all();

    sharedThis.reset();
  }
}

void AsyncReader::doRead()
{
  if (shouldExit.load() == true)
  {
    stop();
  }
  #ifdef NDEBUG
  #else
    //std::cout << "-- start doRead\n";
  #endif

  asio::async_read_until(*socket, readBuffer,
  '\n',
  [this](const system::error_code& error, std::size_t bytes_transferred)
  {
    if (shouldExit.load() != true && !error)
    {
      onReading(bytes_transferred, socket);

      #ifdef NDEBUG
      #else
        //std::cout << "-- continue doRead\n";
      #endif

      doRead();
    }
    else
    {
      stop();
    }
  });
}

void AsyncReader::onReading(std::size_t bytes_transferred, SharedSocket socket)
{
  #ifdef NDEBUG
  #else
    //std::cout << "-- start onReading\n";
  #endif

  std::istream tempStream{&readBuffer};
  std::string request{};

  std::copy(std::istream_iterator<std::string>(tempStream),
            std::istream_iterator<std::string>(),
            std::back_inserter(request));

  auto reaction(CommandTranslator::translate(request, socket));
}
