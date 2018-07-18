// server_process.cpp in OTUS Homework 13 project

#include "server_process.h"
#include <utility>
#include <csignal>
#include <atomic>
#include <condition_variable>

#include "async_server.h"

std::atomic<bool> shouldExit{false};

std::condition_variable terminationNotifier{};
std::mutex terminationLock{};

void terminationHandler(int)
{
  shouldExit.store(true);
  terminationNotifier.notify_all();
}

int serverRun(int argc, char* argv[], std::ostream& outputStream,
               std::ostream& errorStream)
{
  std::signal(SIGINT, terminationHandler);
  std::signal(SIGTERM, terminationHandler);

  if (argc != 2 || std::stoi(std::string{argv[1]}) < 1)
  {
    errorStream << "usage: join_server <port>" << std::endl;
    return 1;
  }

  uint16_t portNumber{static_cast<uint16_t>(std::stoull(std::string{argv[1]}))};

  AsyncJoinServer<2> server{
    asio::ip::address_v4::any(), portNumber,    
    outputStream, errorStream
  };

  std::thread mainThread{[&server]()
   {
      server.start();

      while (shouldExit.load() != true)
      {
        std::unique_lock<std::mutex> lockTermination{terminationLock};
        terminationNotifier.wait_for(lockTermination, 100ms, []()
        {
          return shouldExit.load() == true;
        });
        lockTermination.unlock();
      }

      server.stop();
   }};


  mainThread.join();

  return 0;
}
