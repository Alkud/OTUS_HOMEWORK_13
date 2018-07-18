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
};
