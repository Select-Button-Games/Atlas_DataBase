#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Database.h"

namespace fs = std::filesystem;

class DataBaseFile {
public:
    // Save the database to a binary file
    static void saveDatabase(const Database& db, const std::string& fileName) {
        fs::path path = fs::current_path();
        path /= fileName;
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
                    }
                }
            }
            file.close();
        }
    }

    // Load the database from a binary file
    static Database loadDatabase(const std::string& fileName) {
        Database db;
        fs::path path = fs::current_path();
        path /= fileName;
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
                    }
                    table.addRow(row);
                }

                db.addTable(table);
            }
            file.close();
        }
        return db;
    }

};
