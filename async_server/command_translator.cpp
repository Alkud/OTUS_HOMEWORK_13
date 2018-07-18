#include "command_translator.h"

CommandReaction
reactError (std::string request,
            SharedSocket socket);

CommandReaction
reactInsert (const std::string& table,
             const std::string& id,
             const std::string& name,
             SharedSocket socket);

CommandReaction
reactIntersection (SharedSocket socket);

CommandReaction
reactSymmetricDifference (SharedSocket socket);

CommandReaction
reactTruncate (const std::string& table,
               SharedSocket socket);

/* --------------------------------------------------------------------------------------------------------------------- */

CommandReaction CommandTranslator::translate(const std::string& request, SharedSocket replySocket)
{
  std::istringstream tempStream{command};

  /* split received string into words */
  std::vector<std::string> splittedCommand{
    std::istream_iterator<std::string>{tempStream},
    std::istream_iterator<std::string>{}
  };

  auto commandName {splittedCommand[0]};                           // 1st word of the received string, treated as a command

  if (splittedCommand.size() == 0                                  // empty command
      || dbCommandCodes.find(commandName) == dbCommandCodes.end()  // unknown command
      || command.size() != dbCommandCodes[commandName].second + 1) // wrong number of arguments
  {
    return reactError(command, replySocket);
  }

  auto commandCode {dbCommandCodes[commandName].first}; // numeric command code

  switch (commandCode)
  {
  case static_cast<uint>(DBCommands::INSERT):
    return reactInsert(splittedCommand[1],
      splittedCommand[2],
      splittedCommand[3],
      replySocket
    );
    break;

  case static_cast<uint>(DBCommands::INTERSECTION):
    return reactIntersection(
      replySocket
    );
    break;

  case static_cast<uint>(DBCommands::SYMMETRIC_DIFFERENCE):
    return reactSymmetricDifference(
      replySocket
    );
    break;

  case static_cast<uint>(DBCommands::TRUNCATE):
    return reactTruncate(splittedCommand[1],
      replySocket
    );
    break;

  default:
    return reactError(command, replySocket);
    break;
  }
}


/* ------------------------------------------------------------------------------------------------ */

CommandReaction reactError(std::string request, SharedSocket socket)
{
  std::stringstream replyStream{};
  replyStream << "ERR bad request: " << "\'" << request << "\'\n";

  std::string reply{replyStream.str()};
  std::vector<std::string> arguments{};

  arguments.push_back(reply);

  DBCommands command {DBCommands::EMPTY};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction reactInsert(const std::string& table,
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

CommandReaction reactIntersection(SharedSocket socket)
{
  std::vector<std::string> arguments{};

  DBCommands command {DBCommands::INTERSECTION};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction reactSymmetricDifference(SharedSocket socket)
{
  std::vector<std::string> arguments{};

  DBCommands command {DBCommands::INTERSECTION};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

CommandReaction reactTruncate(const std::string& table, SharedSocket socket)
{
  std::vector<std::string> arguments{};
  arguments.push_back(table);

  DBCommands command {DBCommands::TRUNCATE};

  auto result{std::make_tuple(command, arguments, socket)};

  return result;
};

