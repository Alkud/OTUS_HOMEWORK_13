#include "command_translator.h"

std::map<std::string, std::pair<uint, uint>>
CommandTranslator::dbCommandCodes
{
  {"INSERT", {static_cast<uint>(DBCommands::INSERT), 3}},                             // code = 0, number of arguments = 3
  {"TRUNCATE", {static_cast<uint>(DBCommands::TRUNCATE), 1}},                         // code = 100, number of arguments = 1
  {"INTERSECTION", {static_cast<uint>(DBCommands::INTERSECTION), 0}},                 // code = 200, number of arguments = 0
  {"SYMMETRIC_DIFFERENCE", {static_cast<uint>(DBCommands::SYMMETRIC_DIFFERENCE), 0}}, // code = 300, number of arguments = 3
};

CommandReaction CommandTranslator::translate(const std::string& request, SharedSocket replySocket)
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
  case static_cast<uint>(DBCommands::INSERT):
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

  case static_cast<uint>(DBCommands::INTERSECTION):
    return reactIntersection(
      "A", "B",
      replySocket
    );
    break;

  case static_cast<uint>(DBCommands::SYMMETRIC_DIFFERENCE):
    return reactSymmetricDifference(
      "A", "B",
      replySocket
    );
    break;

  case static_cast<uint>(DBCommands::TRUNCATE):
    return reactTruncate(splittedCommand[1],
      replySocket
    );
    break;

  default:
    return reactError(request, replySocket);
    break;
  }
}


/* ------------------------------------------------------------------------------------------------ */

CommandReaction
CommandTranslator::reactError(std::string request, SharedSocket socket)
{
  std::stringstream replyStream{};
  replyStream << "ERR bad_request: " << "\'" << request << "\'\n";

  std::string reply{replyStream.str()};
  std::vector<std::string> arguments{};

  arguments.push_back(reply);

  DBCommands command {DBCommands::EMPTY};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction
CommandTranslator::reactInsert(
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

  DBCommands command {DBCommands::INSERT};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction
CommandTranslator::reactIntersection(
  const std::string& tableA,
  const std::string& tableB,
  SharedSocket socket)
{
  std::vector<std::string> arguments{};

  arguments.push_back(tableA);
  arguments.push_back(tableB);

  DBCommands command {DBCommands::INTERSECTION};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction
CommandTranslator::reactSymmetricDifference(
  const std::string& tableA,
  const std::string& tableB,
  SharedSocket socket)
{
  std::vector<std::string> arguments{};
  arguments.push_back(tableA);
  arguments.push_back(tableB);

  DBCommands command {DBCommands::SYMMETRIC_DIFFERENCE};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction
CommandTranslator::reactTruncate(const std::string& table, SharedSocket socket)
{
  std::vector<std::string> arguments{};
  arguments.push_back(table);

  DBCommands command {DBCommands::TRUNCATE};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

