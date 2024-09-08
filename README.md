# Atlas Database

Atlas Database is an educational relational database management system (RDBMS) implemented in C++. It supports basic database operations such as creating databases, adding tables, inserting data, and more. This project is designed to help understand the core concepts of relational databases and their implementation.

## Features

- **User Management**: Secure user registration and login using encrypted passwords.
- **Database Operations**: Create databases, add tables, insert data, and remove rows.
- **Data Types**: Supports various data types including integers, strings, booleans, timestamps, floats, and blobs.
- **B-Tree Indexing**: Efficient data retrieval using B-Tree indexing for primary keys.
- **File Persistence**: Save and load databases from binary files.

## Getting Started

### Prerequisites

- C++17 or later
- OpenSSL library for encryption
- A C++ compiler (e.g., GCC, Clang, MSVC)

### Building the Project

1. Clone the repository:

```
git clone https://github.com/yourusername/atlas-database.git
cd atlas-database
```

2. Build the project using your preferred build system. For example, using `g++`:
    
```
g++ -o atlas Main.cpp UserManagement.cpp Database.cpp DataBaseFile.cpp Query_Parser.cpp CommandExecuter.cpp -lssl -lcrypto
```


### Running the Project

1. Run the executable:
    
```
./atlas
```

2. Follow the on-screen instructions to create a user profile and start using the database.

## Usage

### User Management

- **Register a User**: Create a new user profile.
- **Login**: Authenticate with an existing user profile.

### Database Commands

- **Create Database**: `CREATE DATABASE dbName`
- **Use Database**: `USE dbName`
- **Add Table**: `ADD TABLE tableName (column1 type1, column2 type2, ...)`
- **Insert Data**: `INSERT INTO tableName (column1, column2, ...) VALUES (value1, value2, ...)`
- **Remove Row**: `REMOVE FROM tableName WHERE column = value`

### Example

```
CREATE DATABASE testDB; USE testDB; 
ADD TABLE users (id INT, name STRING, age INT); 
INSERT INTO users (id, name, age) VALUES (1, 'Alice', 30); REMOVE FROM users WHERE id = 1;
```


## Project Structure

- **Main.cpp**: Entry point of the application.
- **UserManagement.h/cpp**: Handles user registration and login with encryption.
- **Database.h/cpp**: Core database classes including `Database`, `Table`, `Row`, and `Column`.
- **DataBaseFile.h/cpp**: Functions for saving and loading databases from files.
- **Query_Parser.h/cpp**: Parses and executes SQL-like commands.
- **CommandExecuter.h/cpp**: Executes commands from a file.
- **BTree.h**: Implementation of B-Tree for indexing.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any improvements or bug fixes.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by various educational resources on relational databases and C++ programming.
- Uses OpenSSL for encryption.

---

Feel free to customize this README to better fit your project's specifics and requirements.

