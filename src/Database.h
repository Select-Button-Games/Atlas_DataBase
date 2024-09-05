#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <variant>

/////////////////////////////////////////////////////////////////////////////////
///////////RELATIONAL DATABASE EDUCATIONAL ONLY ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
    Coded by: Andrew "AJ"
    Date: 8/31/2024
    Desc of project:

    Relational Database similar to MySQL but connectionless.
    Simple CRUD(create, read, update, delete) operations.
*/

// Data type enum
enum class DataType {
    INT,
    STRING,
    BOOL,
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
    std::map<std::string, std::variant<int, std::string, bool>> data;

    void addData(const std::string& columnName, const std::variant<int, std::string, bool>& value) {
        data[columnName] = value;
    }

    std::variant<int, std::string, bool> getData(const std::string& columnName) const {
        auto it = data.find(columnName);
        if (it != data.end()) {
            return it->second;
        }
        return {};
    }
};

// Column class that will describe our Columns within the database
class Column {
public:
    std::string name;
    DataType type;

    Column(const std::string& name, DataType type) : name(name), type(type) {}
};

// Table class that will describe our tables within the database
class Table {
public:
    Table() {}
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;

    Table(const std::string& name) : name(name) {}

    void addColumn(const Column& column) {
        columns.push_back(column);
    }

    void addRow(const Row& row) {
        rows.push_back(row);
    }

    std::vector<Row> getRows() const {
        return rows;
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
