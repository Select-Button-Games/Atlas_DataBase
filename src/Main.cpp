// Main.cpp
#include <iostream>
#include <chrono>
#include <conio.h> // Include for getch
#include "Query_Parser.h"
#include "CommandExecuter.h"
#include "UserManagement.h"
#include "Database.h"
#include "DataBaseFile.h"
#include <iostream>

void printDatabase(const Database& db) {
    for (const auto& tablePair : db.tables) {
        const Table& table = tablePair.second;
        std::cout << "Table: " << table.name << std::endl;

        // Print column headers
        for (const auto& column : table.columns) {
            std::cout << column.name << "\t";
        }
        std::cout << std::endl;

        // Print rows
        for (const auto& row : table.rows) {
            for (const auto& column : table.columns) {
                const auto& value = row.getData(column.name);
                if (std::holds_alternative<int>(value)) {
                    std::cout << std::get<int>(value) << "\t";
                }
                else if (std::holds_alternative<std::string>(value)) {
                    std::cout << std::get<std::string>(value) << "\t";
                }
                else if (std::holds_alternative<bool>(value)) {
                    std::cout << (std::get<bool>(value) ? "true" : "false") << "\t";
                }
            }
            std::cout << std::endl;
        }
    }
}

std::string getPassword() {
    std::string password;
    char ch;
    while ((ch = _getch()) != '\r') { // '\r' is the Enter key
        if (ch == '\b') { // Handle backspace
            if (!password.empty()) {
                std::cout << "\b \b"; // Erase the last character from the console
                password.pop_back();
            }
        }
        else {
            password.push_back(ch);
            std::cout << '*'; // Print asterisk for each character
        }
    }
    std::cout << std::endl;
    return password;
}

void setupWizard(UserManagement& userManager) {
    std::string username, password;
    std::cout << "No user data found. Please create a new profile." << std::endl;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    password = getPassword();

    if (!userManager.registerUser(username, password)) {
        std::cerr << "Failed to create user. Exiting." << std::endl;
        exit(1);
    }
    std::cout << "User created successfully." << std::endl;
}

void login(UserManagement& userManager) {
    std::string username, password;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    password = getPassword();

    if (!userManager.loginUser(username, password)) {
        std::cerr << "Invalid username or password. Exiting." << std::endl;
        exit(1);
    }
    std::cout << "Login successful." << std::endl;
}

int main() {
    UserManagement userManager("users.dat");

    if (!userManager.userDataExists()) {
        setupWizard(userManager);
    }
    else {
        login(userManager);
    }

    DatabaseManager dbManager;
    QueryParser parser(dbManager);
    CommandExecutor executor(parser);

    // Load the database from a file if it exists
    const std::string dbFileName = "database.bin";
    if (std::filesystem::exists(dbFileName)) {
        dbManager.databases["TestDB"] = DataBaseFile::loadDatabase(dbFileName);
        dbManager.selectDatabase("TestDB");
    }

    // Execute commands from an external file
    const std::string commandsFileName = "commands.txt";
    if (!executor.executeCommandsFromFile(commandsFileName)) {
        std::cerr << "Failed to execute commands from file: " << commandsFileName << std::endl;
        return 1;
    }

    // Print the database contents
    if (dbManager.getCurrentDatabase()) {
        printDatabase(*dbManager.getCurrentDatabase());
    }

    // Save the database to a file after operations
    if (dbManager.getCurrentDatabase()) {
        DataBaseFile::saveDatabase(*dbManager.getCurrentDatabase(), dbFileName);
    }

    return 0;
}
