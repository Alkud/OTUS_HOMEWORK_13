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

//void NaiveTable::makeCopy(NaiveTable& destination)
//{
//  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
//  std::unique_lock<std::shared_timed_mutex> lockDestinationData{destination.dataLock};

//  destination.data = data;
//}

void NaiveTable::clear()
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  data.clear();
}

bool NaiveTable::insertName(const int id, const std::string& name)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  return data.insert(std::make_pair(id, std::make_shared<std::string>(name))).second;
}

std::string NaiveTable::getName(const int id) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  if (data.find(id) == data.end())
  {
    throw std::out_of_range(WRONG_ID);
  }
  else
  {
    return *data.at(id);
  }
}

std::string NaiveTable::operator[](const int id) const
{
  return getName(id);
}

size_t NaiveTable::getDataSize() const
{
  std::unique_lock<std::shared_timed_mutex> lockSelfData{dataLock};
  return data.size();
}

std::set<int> NaiveTable::getIndices() const
{
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

//void NaiveDB::makeCopy(NaiveDB& destination) const
//{
//  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
//  std::unique_lock<std::shared_timed_mutex> lockDestinationTables{tablesLock};

//  destination.tables = tables;
//}

void NaiveDB::clear()
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  tables.clear();
}

bool NaiveDB::createTable(const std::string& alias)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  return tables.insert(std::make_pair(alias, NaiveTable{})).second;
}

bool NaiveDB::insertTable(const std::string& alias, const NaiveTable& table)
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  return tables.insert(std::make_pair(alias, table)).second;
}

NaiveTable NaiveDB::getTable(const std::string& alias) const
{
  std::shared_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  if (tables.find(alias) == tables.end())
  {
    throw std::out_of_range(WRONG_ALIAS);
  }
  else
  {
    return tables.at(alias);
  }
}

NaiveTable NaiveDB::operator[](const std::string& alias) const
{
  return getTable(alias);
}

bool NaiveDB::insertNameToTable(const std::string& alias, const int id, const std::string& name)
{
  if (tables.find(alias) == tables.end())
  {
    return false;
  }
  else
  {
    return tables[alias].insertName(id, name);
  }
}

std::string NaiveDB::getNameFromTable(const std::string& alias, const int id) const
{
  if (tables.find(alias) == tables.end())
  {
    throw std::out_of_range(WRONG_ALIAS);;
  }
  else
  {
    return tables.at(alias).getName(id);
  }
}

size_t NaiveDB::getTablesSize() const
{
  std::unique_lock<std::shared_timed_mutex> lockSelfTables{tablesLock};
  return tables.size();
}

std::set<std::string> NaiveDB::getAliases() const
{
  std::set<std::string> result;

  for (const auto& tbl : tables)
  {
    result.insert(tbl.first);
  }

  return result;
}

