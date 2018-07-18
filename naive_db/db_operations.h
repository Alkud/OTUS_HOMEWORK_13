// db_operations.h in Otus homework#13 project

#pragma once

#include <map>
#include <memory>

using DbOperationsResultType = std::map<
  int,
  std::pair<std::string, std::string>
>;

using SharedDbOperationsResultType = std::shared_ptr<
  DbOperationsResultType
>;

SharedDbOperationsResultType
tablesIntersection(const class NaiveTable& tableA, const class NaiveTable& tableB);

SharedDbOperationsResultType
tablesSymmetricDifference(const class NaiveTable& tableA, const class NaiveTable& tableB);
