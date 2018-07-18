// db_operations_test.cpp in OTUS Homework 13 project

#define BOOST_TEST_MODULE OTUS_HW_13_TEST

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

#include "naive_db.h"
#include "db_operations.h"


BOOST_AUTO_TEST_SUITE(joinserver_db_operations_test)

BOOST_AUTO_TEST_CASE(create_table_test)
{
  try
  {
    NaiveTable testTable{};

    BOOST_CHECK(testTable.insertName(12, "Bob") == true);
    BOOST_CHECK(testTable.insertName(22, "Bill") == true);

    BOOST_CHECK(testTable.getDataSize() == 2);
    BOOST_CHECK(testTable[12] == "Bob");
    BOOST_CHECK(testTable[22] == "Bill");

    BOOST_CHECK(testTable.insertName(12, "John") == false);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "create_table_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(copy_table_test)
{
  try
  {
    NaiveTable sourceTable{};

    for (int idx{-10}; idx < 10; ++idx)
    {
      sourceTable.insertName(idx, std::to_string(idx));
    }

    BOOST_CHECK(sourceTable.getDataSize() == 20);

    auto copyTable{sourceTable};

    BOOST_CHECK(copyTable.getDataSize() == 20);

    sourceTable.clear();

    BOOST_CHECK(sourceTable.getDataSize() == 0);

    for (int idx{-10}; idx < 10; ++idx)
    {
      BOOST_CHECK(copyTable[idx] == std::to_string(idx));
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "copy_table_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(create_db_test)
{
  try
  {
    NaiveDB testDB{};

    for (size_t tableIdx{0}; tableIdx < 10; ++tableIdx)
    {
      NaiveTable nextTable{};
      for (int idx{1}; idx < 15; ++idx)
      {
        nextTable.insertName(idx, std::to_string(idx));
      }

      testDB.insertTable(std::to_string(tableIdx), nextTable);
    }

    BOOST_CHECK(testDB.getTablesSize() == 10);

    size_t tableIdx{};
    for (const auto& table : testDB)
    {
      BOOST_CHECK(table.first == std::to_string(tableIdx));
      ++tableIdx;
      for (int idx{1}; idx < 15; ++idx)
      {
        BOOST_CHECK(table.second[idx] == std::to_string(idx));
      }
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "create_db_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(copy_db_test)
{
  try
  {
    NaiveDB sourceDB{};

    for (size_t tableIdx{100}; tableIdx < 110; ++tableIdx)
    {
      NaiveTable nextTable{};
      for (int idx{20}; idx < 30; ++idx)
      {
        nextTable.insertName(idx, std::to_string(idx));
      }

      sourceDB.insertTable(std::to_string(tableIdx), nextTable);
    }

    BOOST_CHECK(sourceDB.getTablesSize() == 10);

    auto copyDB{sourceDB};

    BOOST_CHECK(copyDB.getTablesSize() == 10);

    for (const auto& table : copyDB)
    {
      BOOST_CHECK(table.second.getDataSize() == 10);
    }

    sourceDB.clear();

    for (size_t tableIdx{100}; tableIdx < 110; ++tableIdx)
    {
      for (int idx{20}; idx < 30; ++idx)
      {
        BOOST_CHECK(copyDB[std::to_string(tableIdx)][idx] == std::to_string(idx));
      }
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "copy_db_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(get_table_inices_test)
{
  try
  {
    NaiveTable testTable{};

    std::set<int> expectedIndices{};

    for (int idx{-10}; idx < 10; ++idx)
    {
      testTable.insertName(idx, std::to_string(idx));
      expectedIndices.insert(idx);
    }

    auto actualInices{testTable.getIndices()};

    BOOST_CHECK_EQUAL_COLLECTIONS(actualInices.begin(), actualInices.end(),
                                  expectedIndices.begin(), expectedIndices.end());
  }
  catch (const std::exception& ex)
  {
    std::cerr << "get_table_indices_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(get_db_aliases_test)
{
  try
  {
    NaiveDB testDB{};
    std::set<std::string> expectedAliases{};

    for (size_t tableIdx{0}; tableIdx < 10; ++tableIdx)
    {
      testDB.insertTable(std::to_string(tableIdx), NaiveTable{});
      expectedAliases.insert(std::to_string(tableIdx));
    }

    auto actualAliases{testDB.getAliases()};

    BOOST_CHECK_EQUAL_COLLECTIONS(actualAliases.begin(), actualAliases.end(),
                                  expectedAliases.begin(), expectedAliases.end());
  }
  catch (const std::exception& ex)
  {
    std::cerr << "getdb_aliases_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(intersect_tables_test)
{
  try
  {
    NaiveTable testTableA{};
    NaiveTable testTableB{};

    /* unique indices 0...18 and 1...19 */
    for (int idx{0}; idx < 10; ++idx)
    {
      testTableA.insertName(idx * 2, std::to_string(idx * 2));
      testTableB.insertName(idx * 2 + 1, std::to_string(idx * 2 + 1));
    }

    /* common indices 33...35 */
    for (int idx{33}; idx < 36; ++idx)
    {
      testTableA.insertName(idx, std::to_string(idx * 2));
      testTableB.insertName(idx, std::to_string(idx * 2 + 1));
    }

    auto intersectionResult(tablesIntersection(testTableA, testTableB));

    std::stringstream testStream{};
    for (const auto& record : intersectionResult)
    {
      testStream << record.first << ","
                 << record.second.first << ","
                 << record.second.second << "\n";
    }

    std::string testString {
      "33,66,67\n"
      "34,68,69\n"
      "35,70,71\n"
    };

    BOOST_CHECK(testStream.str() == testString);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "intersect_tables_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_CASE(tables_symmetric_difference_test)
{
  try
  {
    NaiveTable testTableA{};
    NaiveTable testTableB{};

    /* common indices 0...9 */
    for (int idx{0}; idx < 10; ++idx)
    {
      testTableA.insertName(idx, std::to_string(idx * 2));
      testTableB.insertName(idx, std::to_string(idx * 2 + 1));
    }

    /* unique indices 66,67,68,69,70,71 */
    for (int idx{33}; idx < 36; ++idx)
    {
      testTableA.insertName(idx * 2, std::to_string(idx * 2));
      testTableB.insertName(idx * 2 + 1, std::to_string(idx * 2 + 1));
    }

    auto symmetricDifferenceResult(tablesSymmetricDifference(testTableA, testTableB));

    std::stringstream testStream{};
    for (const auto& record : symmetricDifferenceResult)
    {
      testStream << record.first << ","
                 << record.second.first << ","
                 << record.second.second << "\n";
    }

    std::string testString {
      "66,66,\n"
      "67,,67\n"
      "68,68,\n"
      "69,,69\n"
      "70,70,\n"
      "71,,71\n"
    };

    BOOST_CHECK(testStream.str() == testString);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "tables_symmetric_difference_test failed. " << ex.what();
    BOOST_FAIL("");
  }
}

BOOST_AUTO_TEST_SUITE_END()
