// server_process.h in OTUS Homework 13 project
#pragma once

#include <boost/asio.hpp>

int serverRun(int argc,
               char* argv[],
               std::ostream& outputStream,
               std::ostream& errorStream);
