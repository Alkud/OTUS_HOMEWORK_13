#pragma once
#include <boost/asio.hpp>
#include <chrono>

using namespace boost;
using namespace std::chrono_literals;



namespace JoinServer {

using SharedSocket = std::shared_ptr<asio::ip::tcp::socket>;

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


std::map<std::string, std::pair<uint, uint>> dbCommandCodes
{
  {"INSERT", {static_cast<uint>(DBCommands::INSERT), 3}},                             // code = 0, number of arguments = 3
  {"TRUNCATE", {static_cast<uint>(DBCommands::TRUNCATE), 1}},                         // code = 100, number of arguments = 1
  {"INTERSECTION", {static_cast<uint>(DBCommands::INTERSECTION), 0}},                 // code = 200, number of arguments = 0
  {"SYMMETRIC_DIFFERENCE", {static_cast<uint>(DBCommands::SYMMETRIC_DIFFERENCE), 0}}, // code = 300, number of arguments = 3
};

};

using namespace JoinServer;
