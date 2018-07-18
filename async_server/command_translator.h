#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "server_common_types.h"

class CommandTranslator
{
public:
  CommandTranslator(){}

  static CommandReaction translate(const std::string& request, SharedSocket replySocket);

  static std::map<std::string, std::pair<uint, uint>> dbCommandCodes;

  static CommandReaction
  reactError (std::string request,
              SharedSocket socket);

  static CommandReaction
  reactInsert (const std::string& table,
               const std::string& id,
               const std::string& name,
               SharedSocket socket);

  static CommandReaction
  reactIntersection (SharedSocket socket);

  static CommandReaction
  reactSymmetricDifference (SharedSocket socket);

  static CommandReaction
  reactTruncate (const std::string& table,
                 SharedSocket socket);
};
