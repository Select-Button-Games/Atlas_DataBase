#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <variant>
#include <optional>
#include <ctime> // Include for std::time_t
#include "BTree.h"
class DatabaseManager; // Forward declaration

/////////////////////////////////////////////////////////////////////////////////
///////////RELATIONAL DATABASE EDUCATIONAL ONLY ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Data type enum
enum class DataType {
    INT,
    STRING,
    BOOL,
    TIMESTAMP,
    FLOAT,
    BLOB
};

// Forward declare the Table class
class Table;

// Our core database class that will encapsulate the entire database itself 
class Database {
public:
    std::map<std::string, Table> tables;

    void addTable(const Table& table);
    Table* getTable(const std::string& tableName);
    void clear();
};

// Row class that will describe our rows inside the database
class Row {
public:
    std::map<std::string, std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>> data;

    void addData(const std::string& columnName, const std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>& value) {
        data[columnName] = value;
    }

    std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>> getData(const std::string& columnName) const {
        auto it = data.find(columnName);
        if (it != data.end()) {
            return it->second;
        }
        return {};
    }
};

class ForeignKey {
public:
    std::string referencedTable;
    std::string referencedColumn;

    ForeignKey(const std::string& refTable, const std::string& refColumn)
        : referencedTable(refTable), referencedColumn(refColumn) {}
};

// Column class that will describe our Columns within the database
class Column {
public:
    std::string name;
    DataType type;
    bool isPrimaryKey;
    std::optional<ForeignKey> foreignKey;
    std::unique_ptr<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>> index;

 
    Column() = default;

    Column(const std::string& name, DataType type, bool isPrimaryKey = false, std::optional<ForeignKey> foreignKey = std::nullopt)
        : name(name), type(type), isPrimaryKey(isPrimaryKey), foreignKey(foreignKey) {
        if (isPrimaryKey) {
            index = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(3); 
        }
    }

   
    Column(const Column& other)
        : name(other.name), type(other.type), isPrimaryKey(other.isPrimaryKey), foreignKey(other.foreignKey) {
        if (other.index) {
            index = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(*other.index);
        }
    }

   
    Column& operator=(const Column& other) {
        if (this == &other) {
            return *this;
        }
        name = other.name;
        type = other.type;
        isPrimaryKey = other.isPrimaryKey;
        foreignKey = other.foreignKey;
        if (other.index) {
            index = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(*other.index);
        }
        else {
            index.reset();
        }
        return *this;
    }

 
    Column(Column&& other) noexcept = default;

  
    Column& operator=(Column&& other) noexcept = default;

    void setPrimaryKey(bool isPrimaryKey) {
        this->isPrimaryKey = isPrimaryKey;
    }

    void setForeignKey(const std::string& refTable, const std::string& refColumn) {
        foreignKey = ForeignKey(refTable, refColumn);
    }

    void addToIndex(const std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>& value) {
        if (index) {
            index->insert(value);
        }
    }
};



class Table {
public:
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::unique_ptr<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>> primaryKeyBTree;

  
    Table() = default;

    
    Table(const std::string& name) : name(name) {}

    Table(const Table& other)
        : name(other.name), columns(other.columns), rows(other.rows) {
        if (other.primaryKeyBTree) {
            primaryKeyBTree = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(other.primaryKeyBTree->getDegree());
            other.primaryKeyBTree->copyTo(*primaryKeyBTree);
        }
    }

  
    Table& operator=(const Table& other) {
        if (this == &other) {
            return *this;
        }
        name = other.name;
        columns = other.columns;
        rows = other.rows;
        if (other.primaryKeyBTree) {
            primaryKeyBTree = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(other.primaryKeyBTree->getDegree());
            other.primaryKeyBTree->copyTo(*primaryKeyBTree);
        }
        else {
            primaryKeyBTree.reset();
        }
        return *this;
    }

    // Move constructor
    Table(Table&& other) noexcept = default;

    // Move assignment operator
    Table& operator=(Table&& other) noexcept = default;

    void addColumn(const Column& column) {
        // Ensure only one primary key
        if (column.isPrimaryKey) {
            for (const auto& col : columns) {
                if (col.isPrimaryKey) {
                    throw std::runtime_error("Table can only have one primary key.");
                }
            }
          
            primaryKeyBTree = std::make_unique<BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>>(3); 
        }
        columns.push_back(column);
    }

    void addRow(const Row& row, DatabaseManager& dbManager);

    const Column* getPrimaryKey() const {
        for (const auto& column : columns) {
            if (column.isPrimaryKey) {
                return &column;
            }
        }
        return nullptr;
    }

    const Column* getColumn(const std::string& columnName) const {
        for (const auto& column : columns) {
            if (column.name == columnName) {
                return &column;
            }
        }
        return nullptr; 
    }
    void deleteRow(const std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>>& primaryKey);
    void updateRow(const std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>>& oldPrimaryKey, const Row& newRow);
  
    void setPrimaryKeyBTree(BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>* btree) {
        primaryKeyBTree.reset(btree);
    }

    BTree<std::variant<int, std::string, bool, std::time_t, float, std::vector<uint8_t>>>* getPrimaryKeyBTree() const {
        return primaryKeyBTree.get();
    }
};


void Database::addTable(const Table& table) {
    tables[table.name] = table;
}

Table* Database::getTable(const std::string& tableName) {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return &(it->second);
    }
    return nullptr;
}

void Database::clear() {
    tables.clear();
}

class DatabaseManager {
public:
    std::map<std::string, Database> databases;
    Database* currentDatabase = nullptr;

    void createDatabase(const std::string& dbName) {
        databases[dbName] = Database();
    }

    bool selectDatabase(const std::string& dbName) {
        auto it = databases.find(dbName);
        if (it != databases.end()) {
            currentDatabase = &(it->second);
            return true;
        }
        return false;
    }

    Database* getCurrentDatabase() {
        return currentDatabase;
    }
};



void Table::addRow(const Row& row, DatabaseManager& dbManager) {
    const Column* primaryKey = getPrimaryKey();
    if (primaryKey) {
        auto primaryKeyValue = row.getData(primaryKey->name);
        if (primaryKey->index) {
            if (primaryKey->index->search(primaryKeyValue)) {
                throw std::runtime_error("Duplicate primary key value.");
            }
        }
    }

    for (const auto& column : columns) {
        if (column.foreignKey) {
            const auto& fk = column.foreignKey.value();
            Database* db = dbManager.getCurrentDatabase();
            Table* refTable = db->getTable(fk.referencedTable);
            if (!refTable) {
                throw std::runtime_error("Referenced table not found.");
            }
            const Column* refColumn = refTable->getColumn(fk.referencedColumn);
            if (!refColumn) {
                throw std::runtime_error("Referenced column not found.");
            }
            auto refValue = row.getData(column.name);
            bool found = false;
            for (const auto& refRow : refTable->rows) {
                if (refRow.getData(fk.referencedColumn) == refValue) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Foreign key constraint violation.");
            }
        }
    }

    rows.push_back(row);
    for (auto& column : columns) { 
        auto value = row.getData(column.name);
        if (column.index) {
            column.addToIndex(value);
        }
    }
}
void Table::deleteRow(const std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>>& primaryKey) {
    const Column* primaryKeyColumn = getPrimaryKey();
    if (!primaryKeyColumn) {
        throw std::runtime_error("Primary key column not found.");
    }

    auto it = std::find_if(rows.begin(), rows.end(), [&](const Row& row) {
        return row.getData(primaryKeyColumn->name) == primaryKey;
        });

    if (it != rows.end()) {
        primaryKeyBTree->remove(primaryKey);
        rows.erase(it);
    }
    else {
        throw std::runtime_error("Row with the given primary key not found");
    }
}

void Table::updateRow(const std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>>& oldPrimaryKey, const Row& newRow) {
    const Column* primaryKeyColumn = getPrimaryKey();
    if (!primaryKeyColumn) {
        throw std::runtime_error("Primary key column not found.");
    }

    auto it = std::find_if(rows.begin(), rows.end(), [&](const Row& row) {
        return row.getData(primaryKeyColumn->name) == oldPrimaryKey;
        });

    if (it != rows.end()) {
        primaryKeyBTree->remove(oldPrimaryKey);
        *it = newRow;
        primaryKeyBTree->insert(newRow.getData(primaryKeyColumn->name));
    }
    else {
        throw std::runtime_error("Row with the given primary key not found");
    }
}
