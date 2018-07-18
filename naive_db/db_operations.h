// db_operations.h in Otus homework#13 project

#pragma once

#include "naive_db.h"

using OperationsResultType = std::map<int, std::pair<std::string, std::string>>;

OperationsResultType
tablesIntersection(const NaiveTable& tableA, const NaiveTable& tableB);

OperationsResultType
tablesSymmetricDifference(const NaiveTable& tableA, const NaiveTable& tableB);
