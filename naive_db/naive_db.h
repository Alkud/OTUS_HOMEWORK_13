// naive_db.h in Otus homework#13 project

#pragma once

#include <memory>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include "db_operations.h"

using SharedNaiveTable = std::shared_ptr<class NaiveTable>;
using SharedConstNaiveTable = std::shared_ptr<const class NaiveTable>;

using SharedNaiveDB = std::shared_ptr<class NaiveDB>;
using SharedConstNaiveDB = std::shared_ptr<const class NaiveDB>;

using SharedString = std::shared_ptr<std::string>;
using SharedConstString = std::shared_ptr<const std::string>;
using StringVector = std::vector<std::string>;
using SharedStringVector = std::shared_ptr<StringVector>;


const std::string WRONG_ID {"wrong_id"};

const std::string WRONG_ALIAS {"wrong_alias"};

enum class DbOpResult
{
  OK = 0,
  ID_NOT_FOUND = 1,
  ID_DUPLICATE = 2,
  ALIAS_NOT_FOUND = 3,
  ALIAS_DUPLICATE = 4,
};

/// Simple <key, value> table
class NaiveTable
{
public:
 /// Default constructor
 NaiveTable(){}
 /// Copy constructor
 NaiveTable(const NaiveTable& other);
 /// Move constructor
 NaiveTable(NaiveTable&& other);

 /// Copy assign operator
 NaiveTable& operator=(const NaiveTable& other);
 /// Move assign operator
 NaiveTable& operator=(NaiveTable&& other);

 /// Removes all records
 void clear();

 /// Inserts new record
 DbOpResult insertName(const int id, const std::string& name);

 /// Checks if given id is present
 bool containsId(const int id);

 /// Returns a record if given id found
 std::pair<std::string, DbOpResult> getName(const int id) const;
 /// Returns result of getName(id)
 SharedConstString operator[](const int id) const;

 /// Returns actual count of records
 size_t getDataSize() const;

 /// Returns data container's iterator to begin
 auto begin()
 { return data.begin();}

 /// Returns data container's const iterator to begin
 auto cbegin() const
 { return data.cbegin();}

 /// Returns data container's iterator to end
 auto end()
 { return data.end();}

 /// Returns data container's const iterator to end
 auto cend() const
 { return data.cend();}

 /// Returns ordered set of record indices
 std::set<int> getIndices() const;

private:
  /// Data access shared lock
  mutable std::shared_timed_mutex dataLock{};

  /// data member only stores id's and pointers to actual records
  std::unordered_map<int, SharedString> data{};
};

/// Simple database storing a set of NaiveTable's
class NaiveDB
{
public:
  /// Default constructor
  NaiveDB(){}
  /// Copy constructor
  NaiveDB(const NaiveDB& other);
  /// Move constructor
  NaiveDB(NaiveDB&& other);

  /// Copy assign operator
  NaiveDB& operator=(const NaiveDB& other);
  /// Move assign operator
  NaiveDB& operator=(NaiveDB&& other);

  /// Copies actual data to destination DB
  //void makeCopy(NaiveDB& destination) const;

  /// Removes all tables
  void clear();

  /// Creates a new empty table if given alias NOT found
  bool createTable(const std::string& alias);

  /// Inserts a copy of existng table if given alias NOT found
  DbOpResult insertTable(const std::string& alias, const NaiveTable& table);

  /// Checks if a table with given alias if present
  bool containsTable(const std::string& alias);

  /// Clear a table if given alias found
  bool clearTable(const std::string& alias);

  /// Returns a copy of table if given alias found
  std::pair<NaiveTable, DbOpResult> getTable(const std::string& alias) const ;
  /// Returns pointer to a table if given alias found
  SharedConstNaiveTable operator[](const std::string& alias) const;

  /// Inserta new record to a table if given alias found
  DbOpResult insertNameToTable(const std::string& alias, const int id, const std::string& name);

  /// Gets a record from a table if given alias found
  std::pair<std::string, DbOpResult> getNameFromTable(const std::string& alias, const int id) const;

  /// Gets result of intersection of two given tables
  SharedDbOperationsResultType getIntersection(const std::string& aliasA, const std::string& aliasB);

  /// Gets result of symmetric difference of two given tables
  SharedDbOperationsResultType getSymmetricDifference(const std::string& aliasA, const std::string& aliasB);

  /// Returns actual count of tables
  size_t getTablesSize() const;

  /// Returns total count of records in tables
  size_t getTotalDataSize() const;

  /// Returns tables container's iterator to begin
  auto begin()
  { return tables.begin();}

  /// Returns tables container's const iterator to begin
  auto cbegin() const
  { return tables.cbegin();}

  /// Returns tables container's iterator to end
  auto end()
  { return tables.end();}

  /// Returns tables container's const iterator to end
  auto cend() const
  { return tables.cend();}

  /// Returns ordered set of table aliases
  std::set<std::string> getAliases() const;

private:
  /// Tables access shared lock
  mutable std::shared_timed_mutex tablesLock{};
  /// Tables container
  std::map<std::string, SharedNaiveTable> tables;
};


