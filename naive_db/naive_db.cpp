#include "naive_db.h"

/* -------------- Table class implementation ---------------------------------------------*/

NaiveTable::NaiveTable(const NaiveTable& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherData{other.dataLock};

  data = other.data;
}

NaiveTable::NaiveTable(NaiveTable&& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherData{other.dataLock};

  data.swap(other.data);
}

NaiveTable& NaiveTable::operator=(const NaiveTable& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherData{other.dataLock};

  data = other.data;

  return *this;
}

NaiveTable& NaiveTable::operator=(NaiveTable&& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherData{other.dataLock};

  data.swap(other.data);

  return *this;
}

void NaiveTable::clear()
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  data.clear();
}

DbOpResult NaiveTable::insertName(const int id, const std::string& name)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  return (data.insert(std::make_pair(
    id, std::make_shared<std::string>(name)
  )).second ? DbOpResult::OK : DbOpResult::ID_DUPLICATE);
}

bool NaiveTable::containsId(const int id)
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  return data.find(id) != data.end();
}

std::pair<std::string, DbOpResult> NaiveTable::getName(const int id) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  if (data.find(id) == data.end())
  {
    return {"", DbOpResult::ID_NOT_FOUND};
  }
  else
  {
    return {*data.at(id), DbOpResult::OK};
  }
}

SharedConstString NaiveTable::operator[](const int id) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  if (data.find(id) == data.end())
  {
    throw std::out_of_range(WRONG_ID);
  }
  else
  {
    return std::const_pointer_cast<const std::string>(data.at(id));
  }
}

size_t NaiveTable::getDataSize() const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  return data.size();
}

std::set<int> NaiveTable::getIndices() const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};

  std::set<int> result;

  for (const auto& record : data)
  {
    result.insert(record.first);
  }

  return result;
}



/* -------------- Database class implementation ---------------------------------------------*/

NaiveDB::NaiveDB(const NaiveDB& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherTables{other.tablesLock};

  tables = other.tables;
}

NaiveDB::NaiveDB(NaiveDB&& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherTables{other.tablesLock};

  tables.swap(other.tables);
}

NaiveDB& NaiveDB::operator=(const NaiveDB& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherTables{other.tablesLock};

  tables = other.tables;

  return *this;
}

NaiveDB& NaiveDB::operator=(NaiveDB&& other)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  std::unique_lock<std::shared_timed_mutex> lockOtherTables{other.tablesLock};

  tables.swap(other.tables);

  return *this;
}

void NaiveDB::clear()
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  tables.clear();
}

bool NaiveDB::createTable(const std::string& alias)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  return tables.insert(std::make_pair(alias, std::make_shared<NaiveTable>())).second;
}

DbOpResult NaiveDB::insertTable(const std::string& alias, const NaiveTable& table)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  return (tables.insert(std::make_pair(
    alias, std::make_shared<NaiveTable>(table)
  )).second ? DbOpResult::OK : DbOpResult::ALIAS_DUPLICATE);
}

bool NaiveDB::containsTable(const std::string& alias)
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  return tables.find(alias) != tables.end();
}

bool NaiveDB::clearTable(const std::string& alias)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(alias) == tables.end())
  {
    return false;
  }
  else
  {
    tables[alias]->clear();
    return true;
  }
}

std::pair<NaiveTable, DbOpResult> NaiveDB::getTable(const std::string& alias) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(alias) == tables.end())
  {
    return {NaiveTable{}, DbOpResult::ALIAS_NOT_FOUND};
  }
  else
  {
    return {*tables.at(alias), DbOpResult::OK};
  }
}

SharedConstNaiveTable NaiveDB::operator[](const std::string& alias) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(alias) == tables.end())
  {
    throw std::out_of_range(WRONG_ALIAS);
  }
  else
  {
    return std::const_pointer_cast<const NaiveTable>(tables.at(alias));
  }
}

DbOpResult NaiveDB::insertNameToTable(const std::string& alias, const int id, const std::string& name)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(alias) == tables.end())
  {
    return DbOpResult::ALIAS_NOT_FOUND;
  }
  else
  {
    return tables[alias]->insertName(id, name);
  }
}

std::pair<std::string, DbOpResult> NaiveDB::getNameFromTable(const std::string& alias, const int id) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(alias) == tables.end())
  {
    return {"", DbOpResult::ALIAS_NOT_FOUND};
  }
  else
  {
    return tables.at(alias)->getName(id);
  }
}

SharedDbOperationsResultType NaiveDB::getIntersection(const std::string& aliasA, const std::string& aliasB)
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(aliasA) == tables.end()
      || tables.find(aliasB) == tables.end())
  {
    throw std::out_of_range(WRONG_ALIAS);
  }
  else
  {
    auto copyTableA{*tables[aliasA]};
    auto copyTableB{*tables[aliasB]};

    lockSelfTables.unlock();

    return tablesIntersection(copyTableA, copyTableB);
  }
}

SharedDbOperationsResultType NaiveDB::getSymmetricDifference(const std::string& aliasA, const std::string& aliasB)
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  if (tables.find(aliasA) == tables.end()
      || tables.find(aliasB) == tables.end())
  {
    throw std::out_of_range(WRONG_ALIAS);
  }
  else
  {
    auto copyTableA{*tables[aliasA]};
    auto copyTableB{*tables[aliasB]};

    lockSelfTables.unlock();

    return tablesSymmetricDifference(copyTableA, copyTableB);
  }
}

size_t NaiveDB::getTablesSize() const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  return tables.size();
}

size_t NaiveDB::getTotalDataSize() const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  size_t result{};

  for (const auto tbl: tables)
  {
    result += tbl.second->getDataSize();
  }

  return  result;
}

std::set<std::string> NaiveDB::getAliases() const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};

  std::set<std::string> result;

  for (const auto& tbl : tables)
  {
    result.insert(tbl.first);
  }

  return result;
}

