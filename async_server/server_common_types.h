#pragma once

#include <boost/asio.hpp>
#include <chrono>

using namespace boost;
using namespace std::chrono_literals;



using SharedSocket = std::shared_ptr<asio::ip::tcp::socket>;
using SharedService = std::shared_ptr<asio::io_service>;
using UniqueWork = std::unique_ptr<asio::io_service::work>;

enum class DBCommands
{
  INSERT = 0,
  TRUNCATE = 100,
  INTERSECTION = 200,
  SYMMETRIC_DIFFERENCE = 300,
  EMPTY = 12345,
};

using CommandReaction = std::tuple<
  DBCommands,                                // DB command code
  std::vector<std::string>,                  // DB operation arguments to use
  SharedSocket                               // socket to write on termination
>;
