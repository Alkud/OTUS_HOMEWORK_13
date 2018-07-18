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
    BOOST_CHECK(testTable.getName(12) == "Bob");
    BOOST_CHECK(testTable.getName(22) == "Bill");

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
      BOOST_CHECK(copyTable.getName(idx) == std::to_string(idx));
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
        BOOST_CHECK(table.second->getName(idx) == std::to_string(idx));
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
      BOOST_CHECK(table.second->getDataSize() == 10);
    }

    sourceDB.clear();

    for (size_t tableIdx{100}; tableIdx < 110; ++tableIdx)
    {
      for (int idx{20}; idx < 30; ++idx)
      {
        BOOST_CHECK(copyDB[std::to_string(tableIdx)]->getName(idx) == std::to_string(idx));
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

    //    table A             table B
    //    id | name           id | name
    //     0 |  0              1 |  1
    //     2 |  2              3 |  3
    //     4 |  4              5 |  5
    //     6 |  6              7 |  7
    //     8 |  8              9 |  9
    //    10 | 10             11 | 11
    //    33 | 66             33 | 67
    //    34 | 68             34 | 69
    //    35 | 70             35 | 71

    auto intersectionResult(tablesIntersection(testTableA, testTableB));

    std::stringstream testStream{};
    for (const auto& record : *intersectionResult)
    {
      testStream << record.first << ","
                 << record.second.first << ","
                 << record.second.second << "\n";
    }

    //    id | name | name
    //    33 | 66   | 67
    //    34 | 68   | 69
    //    35 | 70   | 71

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

    //    table A             table B
    //    id | name           id | name
    //     0 |  0              0 |  1
    //     1 |  2              1 |  3
    //     2 |  4              2 |  5
    //     3 |  6              3 |  7
    //     4 |  8              4 |  9
    //     5 | 10              5 | 11
    //     6 | 12              6 | 13
    //     7 | 14              7 | 15
    //     8 | 16              8 | 17
    //     9 | 18              9 | 19
    //    66 | 66             67 | 67
    //    68 | 68             69 | 69
    //    70 | 70             71 | 71


    auto symmetricDifferenceResult(*tablesSymmetricDifference(testTableA, testTableB));

    std::stringstream testStream{};
    for (const auto& record : symmetricDifferenceResult)
    {
      testStream << record.first << ","
                 << record.second.first << ","
                 << record.second.second << "\n";
    }

    //    id | name | name
    //    66 | 66   |
    //    67 |      | 67
    //    68 | 68   |
    //    69 |      | 69
    //    70 | 70   |
    //    71 |      | 71

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
