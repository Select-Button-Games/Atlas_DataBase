// DataBaseFile.h
#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Database.h"

namespace fs = std::filesystem;

class DataBaseFile {
public:
    // Save the database to a binary file
    static void saveDatabase(const Database& db, const std::string& dbName, DatabaseManager& dbManager) {
        fs::path path = fs::current_path();
        path /= dbName + ".db"; // Use the database name as the file name
        std::ofstream file(path, std::ios::binary);
        if (file.is_open()) {
            // Write the number of tables
            size_t numTables = db.tables.size();
            file.write(reinterpret_cast<const char*>(&numTables), sizeof(numTables));

            for (const auto& tablePair : db.tables) {
                const Table& table = tablePair.second;

                // Write the table name size and name
                size_t tableNameSize = table.name.size();
                file.write(reinterpret_cast<const char*>(&tableNameSize), sizeof(tableNameSize));
                file.write(table.name.c_str(), tableNameSize);

                // Write the number of columns
                size_t numColumns = table.columns.size();
                file.write(reinterpret_cast<const char*>(&numColumns), sizeof(numColumns));

                // Write each column's name and type
                for (const auto& column : table.columns) {
                    size_t columnNameSize = column.name.size();
                    file.write(reinterpret_cast<const char*>(&columnNameSize), sizeof(columnNameSize));
                    file.write(column.name.c_str(), columnNameSize);

                    DataType columnType = column.type;
                    file.write(reinterpret_cast<const char*>(&columnType), sizeof(columnType));

                    bool isPrimaryKey = column.isPrimaryKey;
                    file.write(reinterpret_cast<const char*>(&isPrimaryKey), sizeof(isPrimaryKey));
                }

                // Write the number of rows
                size_t numRows = table.rows.size();
                file.write(reinterpret_cast<const char*>(&numRows), sizeof(numRows));

                // Write each row's data
                for (const auto& row : table.rows) {
                    for (const auto& column : table.columns) {
                        const std::string& columnName = column.name;
                        const auto& value = row.getData(columnName);

                        if (std::holds_alternative<int>(value)) {
                            int intValue = std::get<int>(value);
                            file.write(reinterpret_cast<const char*>(&intValue), sizeof(intValue));
                        }
                        else if (std::holds_alternative<std::string>(value)) {
                            const std::string& strValue = std::get<std::string>(value);
                            size_t valueSize = strValue.size();
                            file.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));
                            file.write(strValue.c_str(), valueSize);
                        }
                        else if (std::holds_alternative<bool>(value)) {
                            bool boolValue = std::get<bool>(value);
                            file.write(reinterpret_cast<const char*>(&boolValue), sizeof(boolValue));
                        }
                        else if (std::holds_alternative<std::time_t>(value)) {
                            std::time_t timestampValue = std::get<std::time_t>(value);
                            file.write(reinterpret_cast<const char*>(&timestampValue), sizeof(timestampValue));
                        }
                        else if (std::holds_alternative<float>(value)) {
                            float floatValue = std::get<float>(value);
                            file.write(reinterpret_cast<const char*>(&floatValue), sizeof(floatValue));
                        }
                        else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
                            const std::vector<uint8_t>& blobValue = std::get<std::vector<uint8_t>>(value);
                            size_t blobSize = blobValue.size();
                            file.write(reinterpret_cast<const char*>(&blobSize), sizeof(blobSize));
                            file.write(reinterpret_cast<const char*>(blobValue.data()), blobSize);
                        }
                    }
                }
            }
            file.close();
        }
    }

    static Database loadDatabase(const std::string& dbName, DatabaseManager& dbManager) {
        Database db;
        fs::path path = fs::current_path();
        path /= dbName + ".db"; // Use the database name as the file name
        std::ifstream file(path, std::ios::binary);
        if (file.is_open()) {
            size_t numTables = 0;
            file.read(reinterpret_cast<char*>(&numTables), sizeof(numTables));
            std::cout << "Number of tables: " << numTables << std::endl;

            for (size_t i = 0; i < numTables; ++i) {
                // Read the table name
                size_t tableNameSize = 0;
                file.read(reinterpret_cast<char*>(&tableNameSize), sizeof(tableNameSize));
                if (tableNameSize > 1000) { // Arbitrary large value check
                    throw std::runtime_error("Invalid table name size.");
                }
                std::string tableName(tableNameSize, '\0');
                file.read(&tableName[0], tableNameSize);
                std::cout << "Table name: " << tableName << std::endl;

                Table table(tableName);

                // Read the number of columns
                size_t numColumns = 0;
                file.read(reinterpret_cast<char*>(&numColumns), sizeof(numColumns));
                std::cout << "Number of columns: " << numColumns << std::endl;

                for (size_t j = 0; j < numColumns; ++j) {
                    // Read each column's name
                    size_t columnNameSize = 0;
                    file.read(reinterpret_cast<char*>(&columnNameSize), sizeof(columnNameSize));
                    if (columnNameSize > 1000) { // Arbitrary large value check
                        throw std::runtime_error("Invalid column name size.");
                    }
                    std::string columnName(columnNameSize, '\0');
                    file.read(&columnName[0], columnNameSize);
                    std::cout << "Column name: " << columnName << std::endl;

                    // Read each column's type
                    DataType columnType;
                    file.read(reinterpret_cast<char*>(&columnType), sizeof(columnType));
                    std::cout << "Column type: " << static_cast<int>(columnType) << std::endl;

                    // Read primary key flag
                    bool isPrimaryKey;
                    file.read(reinterpret_cast<char*>(&isPrimaryKey), sizeof(isPrimaryKey));
                    std::cout << "Is primary key: " << isPrimaryKey << std::endl;

                    table.addColumn(Column(columnName, columnType, isPrimaryKey));
                }

                // Read the number of rows
                size_t numRows = 0;
                file.read(reinterpret_cast<char*>(&numRows), sizeof(numRows));
                std::cout << "Number of rows: " << numRows << std::endl;

                for (size_t k = 0; k < numRows; ++k) {
                    Row row;
                    for (const auto& column : table.columns) {
                        if (column.type == DataType::INT) {
                            int intValue;
                            file.read(reinterpret_cast<char*>(&intValue), sizeof(intValue));
                            row.addData(column.name, intValue);
                        }
                        else if (column.type == DataType::STRING) {
                            size_t valueSize = 0;
                            file.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
                            if (valueSize > 1000000) { // Arbitrary large value check
                                throw std::runtime_error("Invalid string value size.");
                            }
                            std::string strValue(valueSize, '\0');
                            file.read(&strValue[0], valueSize);
                            row.addData(column.name, strValue);
                        }
                        else if (column.type == DataType::BOOL) {
                            bool boolValue;
                            file.read(reinterpret_cast<char*>(&boolValue), sizeof(boolValue));
                            row.addData(column.name, boolValue);
                        }
                        else if (column.type == DataType::TIMESTAMP) {
                            std::time_t timestampValue;
                            file.read(reinterpret_cast<char*>(&timestampValue), sizeof(timestampValue));
                            row.addData(column.name, timestampValue);
                        }
                        else if (column.type == DataType::FLOAT) {
                            float floatValue;
                            file.read(reinterpret_cast<char*>(&floatValue), sizeof(floatValue));
                            row.addData(column.name, floatValue);
                        }
                        else if (column.type == DataType::BLOB) {
                            size_t blobSize = 0;
                            file.read(reinterpret_cast<char*>(&blobSize), sizeof(blobSize));
                            if (blobSize > 1000000) { // Arbitrary large value check
                                throw std::runtime_error("Invalid blob value size.");
                            }
                            std::vector<uint8_t> blobValue(blobSize);
                            file.read(reinterpret_cast<char*>(blobValue.data()), blobSize);
                            row.addData(column.name, blobValue);
                        }
                    }
                    table.addRow(row, dbManager); // Pass the DatabaseManager reference here
                }

                db.addTable(table);
            }
            file.close();
        }
        return db;
    }

};
