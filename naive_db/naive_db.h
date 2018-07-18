// naive_db.h in Otus homework#13 project

#pragma once

#include <memory>
#include <map>
#include <set>
#include <unordered_map>
#include <shared_mutex>

using SharedString = std::shared_ptr<std::string>;

const std::string WRONG_ID {"wrong_id"};

const std::string WRONG_ALIAS {"wrong_alias"};

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

 /// Copies actual data to destination table
 //void makeCopy(NaiveTable& destination);

 /// Removes all records
 void clear();

 /// Inserts new record
 bool insertName(const int id, const std::string& name);

 /// Returns a record if given id found
 std::string getName(const int id) const;
 /// Returns result of getName(id)
 std::string operator[](const int id) const;

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

  /// Inserts an existng table if given alias NOT found
  bool insertTable(const std::string& alias, const NaiveTable& table);

  /// Returns a table if given alias found
  NaiveTable getTable(const std::string& alias) const ;
  /// Returns result of getTable(alias)
  NaiveTable operator[](const std::string& alias) const;

  /// Gets pointer to a table if given alias found
  //const std::shared_ptr<NaiveTable> getTablePointer(const std::string& alias);

  /// Copies a table to destination if given alias found
  //void makeTableCopy(const std::string& alias, NaiveTable& destination) const;

  /// Inserta new record to a table if given alias found
  bool insertNameToTable(const std::string& alias, const int id, const std::string& name);

  /// Gets a record from a table if given alias found
  std::string getNameFromTable(const std::string& alias, const int id) const;

  /// Returns actual count of tables
  size_t getTablesSize() const;

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
  std::map<std::string, NaiveTable> tables;
};

