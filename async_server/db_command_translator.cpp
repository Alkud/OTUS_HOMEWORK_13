#include "db_command_translator.h"

std::map<std::string, std::pair<uint, uint>>
DbCommandTranslator::dbCommandCodes
{
  {"INSERT", {static_cast<uint>(DbCommands::INSERT), 3}},                             // code = 0, number of arguments = 3
  {"TRUNCATE", {static_cast<uint>(DbCommands::TRUNCATE), 1}},                         // code = 100, number of arguments = 1
  {"INTERSECTION", {static_cast<uint>(DbCommands::INTERSECTION), 0}},                 // code = 200, number of arguments = 0
  {"SYMMETRIC_DIFFERENCE", {static_cast<uint>(DbCommands::SYMMETRIC_DIFFERENCE), 0}}, // code = 300, number of arguments = 3
};

DbCommandReaction DbCommandTranslator::translate(const std::string& request, SharedSocket replySocket)
{
  std::istringstream tempStream{request};

  /* split received string into words */
  std::vector<std::string> splittedCommand{
    std::istream_iterator<std::string>{tempStream},
    std::istream_iterator<std::string>{}
  };

  auto commandName {splittedCommand[0]};                           // 1st word of the received string, treated as a command

  if (splittedCommand.size() == 0                                  // empty command
      || dbCommandCodes.find(commandName) == dbCommandCodes.end()  // unknown command
      || splittedCommand.size() != dbCommandCodes[commandName].second + 1) // wrong number of arguments
  {
    return reactError(request, replySocket);
  }

  auto commandCode {dbCommandCodes[commandName].first}; // numeric command code

  switch (commandCode)
  {
  case static_cast<uint>(DbCommands::INSERT):
    try /* make sure id field is convertible */
    {
      std::stoi(splittedCommand[2]);
    }
    catch(...)
    {
      return reactError(request, replySocket);
      break;
    }

    return reactInsert(splittedCommand[1],
      splittedCommand[2],
      splittedCommand[3],
      replySocket
    );
    break;

  case static_cast<uint>(DbCommands::INTERSECTION):
    return reactIntersection(
      "A", "B",
      replySocket
    );
    break;

  case static_cast<uint>(DbCommands::SYMMETRIC_DIFFERENCE):
    return reactSymmetricDifference(
      "A", "B",
      replySocket
    );
    break;

  case static_cast<uint>(DbCommands::TRUNCATE):
    return reactTruncate(splittedCommand[1],
      replySocket
    );
    break;

  default:
    return reactError(request, replySocket);
    break;
  }
}

std::string DbCommandTranslator::restore(const DbCommandReaction& reaction)
{
  auto commandCode {static_cast<uint>(std::get<0>(reaction))};
  auto arguments {std::get<1>(reaction)};

  std::stringstream resultStream{};

  switch (commandCode)
  {
  case static_cast<uint>(DbCommands::INSERT):
    resultStream << "INSERT ";
    break;

  case static_cast<uint>(DbCommands::INTERSECTION):
    resultStream << "INTERSECTION ";
    break;

  case static_cast<uint>(DbCommands::SYMMETRIC_DIFFERENCE):
    resultStream << "SYMMETRIC_DIFFERENCE ";
    break;

  case static_cast<uint>(DbCommands::TRUNCATE):
    resultStream << "TRUNCATE ";
    break;

  default:
    resultStream << "UNKNOWN ";
    break;
  }

  auto commandName{resultStream.str()};
  commandName = commandName.substr(0, commandName.length() - 1);

  for (size_t idx {0}; idx < dbCommandCodes[commandName].second; ++idx)
  {
    resultStream << arguments[idx] << " ";
  }

  auto result {resultStream.str()};

  /* cut als space symbol*/
  result = result.substr(0, result.length() - 1);

  return result;
}


/* ------------------------------------------------------------------------------------------------ */

DbCommandReaction
DbCommandTranslator::reactError(std::string request, SharedSocket socket)
{
  std::stringstream replyStream{};
  replyStream << "ERR bad_request " << "\'" << request << "\'\n";

  std::string reply{replyStream.str()};
  std::vector<std::string> arguments{};

  arguments.push_back(reply);

  DbCommands command {DbCommands::EMPTY};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

DbCommandReaction
DbCommandTranslator::reactInsert(
  const std::string& table,
  const std::string& id,
  const std::string& name,
  SharedSocket socket
)
{
  std::vector<std::string> arguments{};

  arguments.push_back(table);
  arguments.push_back(id);
  arguments.push_back(name);

  DbCommands command {DbCommands::INSERT};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

DbCommandReaction
DbCommandTranslator::reactIntersection(
  const std::string& tableA,
  const std::string& tableB,
  SharedSocket socket)
{
  std::vector<std::string> arguments{};

  arguments.push_back(tableA);
  arguments.push_back(tableB);

  DbCommands command {DbCommands::INTERSECTION};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

DbCommandReaction
DbCommandTranslator::reactSymmetricDifference(
  const std::string& tableA,
  const std::string& tableB,
  SharedSocket socket)
{
  std::vector<std::string> arguments{};
  arguments.push_back(tableA);
  arguments.push_back(tableB);

  DbCommands command {DbCommands::SYMMETRIC_DIFFERENCE};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

DbCommandReaction
DbCommandTranslator::reactTruncate(const std::string& table, SharedSocket socket)
{
  std::vector<std::string> arguments{};
  arguments.push_back(table);

  DbCommands command {DbCommands::TRUNCATE};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

