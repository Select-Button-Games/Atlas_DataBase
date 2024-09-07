#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <variant>
#include <optional>
#include <ctime> // Include for std::time_t

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

    void setPrimaryKey(bool isPrimaryKey) {
		this->isPrimaryKey = isPrimaryKey;
	}

    void setForeignKey(const std::string& refTable, const std::string& refColumn) {
        foreignKey = ForeignKey(refTable, refColumn);
    }

    Column(const std::string& name, DataType type, bool isPrimaryKey = false, std::optional<ForeignKey> foreignKey = std::nullopt)
        : name(name), type(type), isPrimaryKey(isPrimaryKey), foreignKey(foreignKey) {}
};

// Table class that will describe our tables within the database
class Table {
public:
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;

    // Default constructor
    Table() = default;

    // Parameterized constructor
    Table(const std::string& name) : name(name) {}

    void addColumn(const Column& column) {
        // Ensure only one primary key
        if (column.isPrimaryKey) {
            for (const auto& col : columns) {
                if (col.isPrimaryKey) {
                    throw std::runtime_error("Table can only have one primary key.");
                }
            }
        }
        columns.push_back(column);
    }

    void addRow(const Row& row, DatabaseManager& dbManager); // Updated signature

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
};

// Implementations of Database member functions
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

void Table::addRow(const Row& row, DatabaseManager& dbManager) { // Updated signature
    const Column* primaryKey = getPrimaryKey();
    if (primaryKey) {
        auto primaryKeyValue = row.getData(primaryKey->name);
        for (const auto& existingRow : rows) {
            if (existingRow.getData(primaryKey->name) == primaryKeyValue) {
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
}
