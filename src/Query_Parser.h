// Query_Parser.h
#pragma once
#include "Database.h"
#include <string>
#include <regex>
#include <sstream>
#include <iostream> // Include for debugging
#include <sstream>


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
    bool parseRemoveRow(const std::string& command);
    bool parseUpdateRow(const std::string& command);
};

bool QueryParser::executeCommand(const std::string& command) {
    std::istringstream commandStream(command);
    std::string singleCommand;
    bool allCommandsSuccessful = true;

    while (std::getline(commandStream, singleCommand, ';')) {
        std::smatch match;
        std::string trimmedCommand = std::regex_replace(singleCommand, std::regex("^ +| +$|( ) +"), "$1"); // Trim spaces

        std::cout << "Executing command: " << trimmedCommand << std::endl; // Debugging

        try {
            if (std::regex_match(trimmedCommand, match, std::regex(R"(CREATE DATABASE (\w+))"))) {
                std::cout << "Matched CREATE DATABASE command." << std::endl; // Debugging
                allCommandsSuccessful &= parseCreateDatabase(trimmedCommand);
            }
            else if (std::regex_match(trimmedCommand, match, std::regex(R"(USE (\w+))"))) {
                std::cout << "Matched USE command." << std::endl; // Debugging
                allCommandsSuccessful &= parseUseDatabase(trimmedCommand);
            }
            else if (std::regex_match(trimmedCommand, match, std::regex(R"(ADD TABLE (\w+) \((.*)\))"))) {
                std::cout << "Matched ADD TABLE command." << std::endl; // Debugging
                allCommandsSuccessful &= parseAddTable(trimmedCommand);
            }
            else if (std::regex_match(trimmedCommand, match, std::regex(R"(INSERT INTO (\w+) \(([^)]+)\) VALUES \(([^)]+)\))"))) {
                std::cout << "Matched INSERT INTO command." << std::endl; // Debugging
                allCommandsSuccessful &= parseInsertInto(trimmedCommand);
            }
            else if (std::regex_match(trimmedCommand, match, std::regex(R"(REMOVE FROM (\w+) WHERE (\w+) = (.+))"))) {
                std::cout << "Matched REMOVE FROM command." << std::endl; // Debugging
                allCommandsSuccessful &= parseRemoveRow(trimmedCommand);
            }
            else {
                std::cerr << "Command not recognized: " << trimmedCommand << std::endl; // Debugging
                allCommandsSuccessful = false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error executing command: " << e.what() << std::endl;
            allCommandsSuccessful = false;
        }
    }

    return allCommandsSuccessful;
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
    std::smatch match;
    // Regex to match the ADD TABLE command and capture table name and columns
    std::regex addTableRegex(R"(ADD TABLE (\w+) \((.*)\))");

    // Match the command against the regex
    if (!std::regex_match(command, match, addTableRegex)) {
        std::cerr << "Failed to parse ADD TABLE command: " << command << std::endl;
        return false;
    }

    // Extract table name and column definitions
    std::string tableName = match[1].str();
    std::string columnsStr = match[2].str();

    std::cout << "Adding table: " << tableName << " with columns: " << columnsStr << std::endl; // Debugging

    Table table(tableName);
    std::istringstream columnsStream(columnsStr);
    std::string columnDef;

    // Process each column definition
    while (std::getline(columnsStream, columnDef, ',')) {
        columnDef = std::regex_replace(columnDef, std::regex("^ +| +$|( ) +"), "$1"); // Trim spaces

        std::smatch columnMatch;
        // Regex to handle column definitions with possible attributes
        std::regex columnRegex(R"((\w+)\s+(\w+)(.*))");

        // Match the column definition to extract column name, type, and attributes
        if (std::regex_match(columnDef, columnMatch, columnRegex)) {
            std::string columnName = columnMatch[1].str();
            std::string columnTypeStr = columnMatch[2].str();
            std::string attributes = columnMatch[3].str();

            std::cout << "Column definition: " << columnDef << std::endl; // Debugging
            std::cout << "Column name: " << columnName << ", Type: " << columnTypeStr
                << ", Attributes: " << attributes << std::endl; // Debugging

            // Identify the column type
            DataType columnType;
            if (columnTypeStr == "INT") {
                columnType = DataType::INT;
            }
            else if (columnTypeStr == "STRING") {
                columnType = DataType::STRING;
            }
            else if (columnTypeStr == "BOOL") {
                columnType = DataType::BOOL;
            }
            else if (columnTypeStr == "TIMESTAMP") {
                columnType = DataType::TIMESTAMP;
            }
            else if (columnTypeStr == "FLOAT") {
                columnType = DataType::FLOAT;
            }
            else if (columnTypeStr == "BLOB") {
                columnType = DataType::BLOB;
            }
            else {
                std::cerr << "Unknown column type: " << columnTypeStr << std::endl;
                return false;
            }

            Column column(columnName, columnType);

            // Check for PRIMARY_KEY attribute
            if (attributes.find("PRIMARY_KEY") != std::string::npos) {
                column.setPrimaryKey(true);
                std::cout << "Column " << columnName << " is a primary key." << std::endl;

                // Initialize BTree for the primary key
                table.setPrimaryKeyBTree(new BTree<std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>>>(3));
            }

            // Check for REFERENCES attribute (foreign key)
            std::smatch referenceMatch;
            std::regex referenceRegex(R"(REFERENCES\s+(\w+)\((\w+)\))");
            if (std::regex_search(attributes, referenceMatch, referenceRegex)) {
                std::string referencedTable = referenceMatch[1].str();
                std::string referencedColumn = referenceMatch[2].str();
                column.setForeignKey(referencedTable, referencedColumn);
                std::cout << "Column " << columnName << " references "
                    << referencedTable << "(" << referencedColumn << ")." << std::endl;
            }

            table.addColumn(column); // Add column to the table
        }
        else {
            std::cerr << "Failed to parse column definition: " << columnDef << std::endl;
            return false;
        }
    }

    // Attempt to add the table to the database
    try {
        dbManager.getCurrentDatabase()->addTable(table);
        std::cout << "Table " << tableName << " successfully added to the database." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding table: " << e.what() << std::endl;
        return false;
    }

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
            std::cout << "Column not found: " << colName << std::endl;
            return false;
        }

        std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>> key;

        if (column->type == DataType::INT) {
            int intValue = std::stoi(value);
            row.addData(colName, intValue);
            key = intValue;
        }
        else if (column->type == DataType::STRING) {
            row.addData(colName, value);
            key = value;
        }
        else if (column->type == DataType::BOOL) {
            bool boolValue = (value == "true");
            row.addData(colName, boolValue);
            key = boolValue;
        }
        else if (column->type == DataType::TIMESTAMP) {
            time_t timestamp = std::stoll(value);
            row.addData(colName, timestamp);
            key = timestamp;
        }
        else if (column->type == DataType::FLOAT) {
            float floatValue = std::stof(value);
            row.addData(colName, floatValue);
            key = floatValue;
        }
        else if (column->type == DataType::BLOB) {
            std::vector<uint8_t> blob(value.begin(), value.end());
            row.addData(colName, blob);
            key = blob;
        }

        
        if (column->isPrimaryKey) {
            table->getPrimaryKeyBTree()->insert(key);
        }
    }

    try {
        table->addRow(row, dbManager);
    }
    catch (const std::runtime_error& e) {
        std::cout << "Error inserting row: " << e.what() << std::endl;
        return false;
    }

    return true;
}

//using Table::deleteRow
bool QueryParser::parseRemoveRow(const std::string& command) {
    if (!dbManager.getCurrentDatabase()) {
        std::cout << "No database selected" << std::endl; // Debugging
        return false;
    }

    std::smatch match;
    std::regex removeRegex(R"(REMOVE FROM (\w+) WHERE (\w+) = (.+))");
    if (!std::regex_match(command, match, removeRegex)) {
        std::cerr << "Failed to parse REMOVE command: " << command << std::endl;
        return false;
    }

    std::string tableName = match[1];
    std::string primaryKeyColumn = match[2];
    std::string value = match[3];

    Table* table = dbManager.getCurrentDatabase()->getTable(tableName);
    if (!table) {
        std::cerr << "Table not found: " << tableName << std::endl;
        return false;
    }

    const Column* column = table->getColumn(primaryKeyColumn);
    if (!column || !column->isPrimaryKey) {
        std::cerr << "Primary key column not found: " << primaryKeyColumn << std::endl;
        return false;
    }

    std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>> key;
    if (column->type == DataType::INT) {
        key = std::stoi(value);
    }
    else if (column->type == DataType::STRING) {
        key = value;
    }
    else if (column->type == DataType::BOOL) {
        key = (value == "true");
    }
    else if (column->type == DataType::TIMESTAMP) {
        key = std::stoll(value);
    }
    else if (column->type == DataType::FLOAT) {
        key = std::stof(value);
    }
    else if (column->type == DataType::BLOB) {
        key = std::vector<uint8_t>(value.begin(), value.end());
    }

    try {
        table->deleteRow(key);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error deleting row: " << e.what() << std::endl;
        return false;
    }

    return true;
}

//using Table::updateRow
bool QueryParser::parseUpdateRow(const std::string& command) {
    if (!dbManager.getCurrentDatabase()) {
        std::cout << "No database selected" << std::endl; // Debugging
        return false;
    }

    std::smatch match;
    std::regex updateRegex(R"(UPDATE (\w+) SET (.+) WHERE (\w+) = (.+))");
    if (!std::regex_match(command, match, updateRegex)) {
        std::cerr << "Failed to parse UPDATE command: " << command << std::endl;
        return false;
    }

    std::string tableName = match[1];
    std::string setClause = match[2];
    std::string primaryKeyColumn = match[3];
    std::string primaryKeyValue = match[4];

    Table* table = dbManager.getCurrentDatabase()->getTable(tableName);
    if (!table) {
        std::cerr << "Table not found: " << tableName << std::endl;
        return false;
    }

    const Column* column = table->getColumn(primaryKeyColumn);
    if (!column || !column->isPrimaryKey) {
        std::cerr << "Primary key column not found: " << primaryKeyColumn << std::endl;
        return false;
    }

    std::variant<int, std::string, bool, time_t, float, std::vector<uint8_t>> oldPrimaryKey;
    if (column->type == DataType::INT) {
        oldPrimaryKey = std::stoi(primaryKeyValue);
    }
    else if (column->type == DataType::STRING) {
        oldPrimaryKey = primaryKeyValue;
    }
    else if (column->type == DataType::BOOL) {
        oldPrimaryKey = (primaryKeyValue == "true");
    }
    else if (column->type == DataType::TIMESTAMP) {
        oldPrimaryKey = std::stoll(primaryKeyValue);
    }
    else if (column->type == DataType::FLOAT) {
        oldPrimaryKey = std::stof(primaryKeyValue);
    }
    else if (column->type == DataType::BLOB) {
        oldPrimaryKey = std::vector<uint8_t>(primaryKeyValue.begin(), primaryKeyValue.end());
    }

    Row newRow;
    std::istringstream setStream(setClause);
    std::string setPart;
    while (std::getline(setStream, setPart, ',')) {
        std::smatch setMatch;
        std::regex setRegex(R"((\w+) = (.+))");
        if (!std::regex_match(setPart, setMatch, setRegex)) {
            std::cerr << "Failed to parse SET clause: " << setPart << std::endl;
            return false;
        }

        std::string colName = setMatch[1];
        std::string value = setMatch[2];

        const Column* col = table->getColumn(colName);
        if (!col) {
            std::cerr << "Column not found: " << colName << std::endl;
            return false;
        }

        if (col->type == DataType::INT) {
            newRow.addData(colName, std::stoi(value));
        }
        else if (col->type == DataType::STRING) {
            newRow.addData(colName, value);
        }
        else if (col->type == DataType::BOOL) {
            newRow.addData(colName, (value == "true"));
        }
        else if (col->type == DataType::TIMESTAMP) {
            newRow.addData(colName, std::stoll(value));
        }
        else if (col->type == DataType::FLOAT) {
            newRow.addData(colName, std::stof(value));
        }
        else if (col->type == DataType::BLOB) {
            newRow.addData(colName, std::vector<uint8_t>(value.begin(), value.end()));
        }
    }

    try {
        table->updateRow(oldPrimaryKey, newRow);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error updating row: " << e.what() << std::endl;
        return false;
    }

    return true;
}
