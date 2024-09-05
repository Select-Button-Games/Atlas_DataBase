// Query_Parser.h
#pragma once
#include "Database.h"
#include <string>
#include <regex>
#include <sstream>
#include <iostream> // Include for debugging

class QueryParser {
public:
    explicit QueryParser(DatabaseManager& dbManager) : dbManager(dbManager) {}

    bool executeCommand(const std::string& command);

private:
    DatabaseManager& dbManager;

    bool parseCreateDatabase(const std::string& command);
    bool parseUseDatabase(const std::string& command);
    bool parseAddTable(const std::string& command);
    bool parseInsertInto(const std::string& command);
};

bool QueryParser::executeCommand(const std::string& command) {
    std::smatch match;

    std::cout << "Executing command: " << command << std::endl; // Debugging

    try {
        if (std::regex_match(command, match, std::regex(R"(CREATE DATABASE (\w+))"))) {
            return parseCreateDatabase(command);
        }

        if (std::regex_match(command, match, std::regex(R"(USE (\w+))"))) {
            return parseUseDatabase(command);
        }

        if (std::regex_match(command, match, std::regex(R"(ADD TABLE (\w+) \(([^)]+)\))"))) {
            return parseAddTable(command);
        }

        if (std::regex_match(command, match, std::regex(R"(INSERT INTO (\w+) \(([^)]+)\) VALUES \(([^)]+)\))"))) {
            return parseInsertInto(command);
        }

        std::cout << "Command not recognized: " << command << std::endl; // Debugging
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error executing command: " << e.what() << std::endl;
        return false;
    }
}

bool QueryParser::parseCreateDatabase(const std::string& command) {
    std::smatch match;
    std::regex_match(command, match, std::regex(R"(CREATE DATABASE (\w+))"));
    std::string dbName = match[1];
    std::cout << "Creating database: " << dbName << std::endl; // Debugging
    dbManager.createDatabase(dbName);
    return true;
}

bool QueryParser::parseUseDatabase(const std::string& command) {
    std::smatch match;
    std::regex_match(command, match, std::regex(R"(USE (\w+))"));
    std::string dbName = match[1];
    std::cout << "Using database: " << dbName << std::endl; // Debugging
    return dbManager.selectDatabase(dbName);
}

bool QueryParser::parseAddTable(const std::string& command) {
    if (!dbManager.getCurrentDatabase()) {
        std::cout << "No database selected" << std::endl;
        return false;
    }

    std::smatch match;
    std::regex_match(command, match, std::regex(R"(ADD TABLE (\w+) \(([^)]+)\))"));
    std::string tableName = match[1];
    std::string columnDefs = match[2];
    std::cout << "Adding table: " << tableName << " with columns: " << columnDefs << std::endl;
    Table table(tableName);

    std::istringstream colStream(columnDefs);
    std::string colDef;
    while (std::getline(colStream, colDef, ',')) {
        std::istringstream colDefStream(colDef);
        std::string colName, colType;
        bool isPrimaryKey = false;

        colDefStream >> colName >> colType;
        if (colType == "PRIMARY_KEY") {
            colType = "INT"; // Assuming primary key is of type INT
            isPrimaryKey = true;
        }

        DataType type;
        if (colType == "INT") type = DataType::INT;
        else if (colType == "STRING") type = DataType::STRING;
        else if (colType == "BOOL") type = DataType::BOOL;
        else {
            std::cout << "Unknown column type: " << colType << std::endl;
            return false;
        }

        std::cout << "Adding column: " << colName << " of type: " << colType << std::endl;
        table.addColumn(Column(colName, type, isPrimaryKey));
    }

    dbManager.getCurrentDatabase()->addTable(table);
    return true;
}

bool QueryParser::parseInsertInto(const std::string& command) {
    if (!dbManager.getCurrentDatabase()) {
        std::cout << "No database selected" << std::endl; // Debugging
        return false;
    }

    std::smatch match;
    std::regex_match(command, match, std::regex(R"(INSERT INTO (\w+) \(([^)]+)\) VALUES \(([^)]+)\))"));
    std::string tableName = match[1];
    std::string columnNames = match[2];
    std::string values = match[3];
    std::cout << "Inserting into table: " << tableName << " columns: " << columnNames << " values: " << values << std::endl; // Debugging

    Table* table = dbManager.getCurrentDatabase()->getTable(tableName);
    if (!table) {
        std::cout << "Table not found: " << tableName << std::endl; // Debugging
        return false;
    }

    Row row;
    std::istringstream colStream(columnNames);
    std::istringstream valStream(values);
    std::string colName, value;

    while (std::getline(colStream, colName, ',') && std::getline(valStream, value, ',')) {
        colName.erase(0, colName.find_first_not_of(" \t")); // Trim leading spaces
        colName.erase(colName.find_last_not_of(" \t") + 1); // Trim trailing spaces
        value.erase(0, value.find_first_not_of(" \t")); // Trim leading spaces
        value.erase(value.find_last_not_of(" \t") + 1); // Trim trailing spaces

        auto* column = table->getColumn(colName);
        if (!column) {
            std::cout << "Column not found: " << colName << std::endl; // Debugging
            return false;
        }

        if (column->type == DataType::INT) {
            row.addData(colName, std::stoi(value));
        }
        else if (column->type == DataType::STRING) {
            row.addData(colName, value);
        }
        else if (column->type == DataType::BOOL) {
            row.addData(colName, value == "true");
        }
    }

    try {
        table->addRow(row);
    }
    catch (const std::runtime_error& e) {
        std::cout << "Error inserting row: " << e.what() << std::endl;
        return false;
    }

    return true;
}
