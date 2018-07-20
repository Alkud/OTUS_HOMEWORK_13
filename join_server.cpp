// join_server.cpp - main entry point in OTUS Homework 13 project

#include <iostream>

#include "server_process.h"

using namespace std;

int main(int argc, char* argv[])
{
  return serverRun(argc, argv, std::cout, std::cerr);
}
