// db_operations.cpp in Otus homework#13 project

#include "db_operations.h"

#include <algorithm>
#include "naive_db.h"

SharedDbOperationsResultType
tablesIntersection(const NaiveTable& tableA, const NaiveTable& tableB)
{
  auto indicesA{tableA.getIndices()};
  auto indicesB{tableB.getIndices()};

  decltype(tableA.getIndices()) commonIndices{};

  std::set_intersection(indicesA.begin(), indicesA.end(),
                        indicesB.begin(), indicesB.end(),
                        std::inserter(commonIndices, commonIndices.begin()));

  SharedDbOperationsResultType result{ new DbOperationsResultType};

  for (const auto idx : commonIndices)
  {
    (*result)[idx] = std::make_pair(tableA.getName(idx), tableB.getName(idx));
  }

  return result;
}

SharedDbOperationsResultType
tablesSymmetricDifference(const NaiveTable& tableA, const NaiveTable& tableB)
{
  auto indicesA{tableA.getIndices()};
  auto indicesB{tableB.getIndices()};

  decltype(tableA.getIndices()) uniqueIndices{};

  std::set_symmetric_difference(
        indicesA.begin(), indicesA.end(),
        indicesB.begin(), indicesB.end(),
        std::inserter(uniqueIndices, uniqueIndices.begin())
  );

  decltype(tableA.getIndices()) missingIndicesA{};
  std::set_difference(uniqueIndices.begin(), uniqueIndices.end(),
                      indicesA.begin(), indicesA.end(),
                      std::inserter(missingIndicesA, missingIndicesA.begin()));

    decltype(tableB.getIndices()) missingIndicesB{};
  std::set_difference(uniqueIndices.begin(), uniqueIndices.end(),
                      indicesB.begin(), indicesB.end(),
                      std::inserter(missingIndicesB, missingIndicesB.begin()));

  SharedDbOperationsResultType result{ new DbOperationsResultType};

  for (const auto& idx : missingIndicesA)
  {
    (*result)[idx] = std::make_pair("", tableB.getName(idx));
  }

  for (const auto& idx : missingIndicesB)
  {
    (*result)[idx] = std::make_pair(tableA.getName(idx), "");
  }

  return result;
}
