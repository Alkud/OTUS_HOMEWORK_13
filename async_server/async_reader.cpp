// async_reader.cpp in Otus homework#13 project

#include "async_reader.h"
#include <chrono>
#include <csignal>
#include <iterator>
#include <iostream>
#include "db_command_translator.h"
#include "db_manager.h"

using namespace std::chrono_literals;

AsyncReader::AsyncReader(AsyncReader::SharedSocket newSocket,
  asio::ip::tcp::acceptor& newAcceptor,
  std::atomic<size_t>& newReaderCounter,
  const SharedService& newRequestService,
  ServerRequestCallback newRequestCallback,
  std::condition_variable& newTerminationNotifier,
  std::mutex& newTerminationLock,
  std::ostream& newOutputStream,
  std::ostream& newErrorStream,
  std::mutex& newOutputLock,
  std::atomic<bool>& stopFlag
):
  socket{newSocket},

  readBuffer{}, requestBuffer{},

  acceptor{newAcceptor}, readerCounter{newReaderCounter},

  requestService{newRequestService},
  requestCallback{newRequestCallback},

  terminationNotifier{newTerminationNotifier},
  terminationLock{newTerminationLock},

  outputStream{newOutputStream},
  errorStream{newErrorStream},
  outputLock{newOutputLock},

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
    std::cout << "-- reader destructor\n";
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
        controllerNotifier.wait_for(dummyLock, 100ms, [this]()
        {
          return shouldExit.load() == true
                 || stopped.load() == true
                 || socket->is_open() != true;
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

  asio::async_read_until(*socket, readBuffer, '\n',
  [this](const system::error_code& error, std::size_t bytes_transferred)
  {
    if (shouldExit.load() != true && !error)
    {
      onReading(bytes_transferred, socket);
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

  std::string fullRequest{
    std::istreambuf_iterator<char>(&readBuffer),
    std::istreambuf_iterator<char>()
  };

  requestBuffer << fullRequest;

  std::string request{};

  for(;;)
  {
    std::getline(requestBuffer, request);

    /* ignore empty commands */
    if (request.empty() == true)
    {
      continue;
    }

    requestBuffer.peek();
    if (requestBuffer.good() != true)
    {
      if (fullRequest.back() != '\n')
      {
        requestBuffer.clear();
        requestBuffer << request;
        break;
      }
      else
      {
        requestBuffer.clear();
        processRequest(request);
        break;
      }
    }
    processRequest(request);
  }
}

void AsyncReader::processRequest(const std::string& request)
{
  auto reaction(DbCommandTranslator::translate(request, socket));

  if (std::get<0>(reaction) == DbCommands::EMPTY)
  {
    std::lock_guard<std::mutex> lockOutput{outputLock};
    outputStream << "> " << request << "\n";

    onBadRequest(std::get<1>(reaction));

    return;
  }

  auto sharedReaction {std::make_shared<DbCommandReaction>(reaction)};

  requestService->post([callback = requestCallback, sharedReaction]()
  {
    callback(sharedReaction);
  });
}

void AsyncReader::onBadRequest(std::vector<std::string> arguments)
{
  std::string message{arguments[0]};  

  asio::async_write(*socket, asio::buffer(message),
  [&lock = outputLock, &stream = outputStream,
   message]
  (const system::error_code& error, std::size_t bytes_transferred)
  {
    if (!error)
    {
      std::lock_guard<std::mutex> lockOutput{lock};
      stream << "< " << message;
    }    
  });
}
