#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "server_common_types.h"

class DbCommandTranslator
{
public:
  DbCommandTranslator(){}

  static DbCommandReaction translate(const std::string& request, SharedSocket replySocket);

  static std::string restore(const DbCommandReaction& reaction);

  static std::map<std::string, std::pair<uint, uint>> dbCommandCodes;

  static DbCommandReaction
  reactError (std::string request,
              SharedSocket socket);

  static DbCommandReaction
  reactInsert (const std::string& table,
               const std::string& id,
               const std::string& name,
               SharedSocket socket);

  static DbCommandReaction
  reactIntersection (const std::string& tableA,
                     const std::string& tableB,
                     SharedSocket socket);

  static DbCommandReaction
  reactSymmetricDifference (const std::string& tableA,
                            const std::string& tableB,
                            SharedSocket socket);

  static DbCommandReaction
  reactTruncate (const std::string& table,
                 SharedSocket socket);
};
