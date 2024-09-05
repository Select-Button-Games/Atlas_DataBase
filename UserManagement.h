// UserManagement.h
#pragma once
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

class UserManagement {
public:
    UserManagement(const std::string& userFile);
    ~UserManagement();

    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);
    bool userDataExists() const;

private:
    std::unordered_map<std::string, std::string> users; // username -> encrypted password
    std::string userFile;

    bool saveUsers();
    bool loadUsers();
    std::string encrypt(const std::string& plaintext, const std::string& key);
    std::string decrypt(const std::string& ciphertext, const std::string& key);
    std::string generateKey(const std::string& password);
};


UserManagement::UserManagement(const std::string& userFile) : userFile(userFile) {
    loadUsers();
}

UserManagement::~UserManagement() {
    saveUsers();
}

bool UserManagement::registerUser(const std::string& username, const std::string& password) {
    if (users.find(username) != users.end()) {
        return false; // User already exists
    }
    std::string key = generateKey(password);
    std::string encryptedPassword = encrypt(password, key);
    users[username] = encryptedPassword;
    return saveUsers();
}

bool UserManagement::loginUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false; // User not found
    }
    std::string key = generateKey(password);
    std::string decryptedPassword = decrypt(it->second, key);
    return decryptedPassword == password;
}

bool UserManagement::saveUsers() {
    std::ofstream file(userFile, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& user : users) {
        size_t usernameSize = user.first.size();
        file.write(reinterpret_cast<const char*>(&usernameSize), sizeof(usernameSize));
        file.write(user.first.c_str(), usernameSize);

        size_t passwordSize = user.second.size();
        file.write(reinterpret_cast<const char*>(&passwordSize), sizeof(passwordSize));
        file.write(user.second.c_str(), passwordSize);
    }
    file.close();
    return true;
}

bool UserManagement::loadUsers() {
    std::ifstream file(userFile, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    while (file.peek() != EOF) {
        size_t usernameSize;
        file.read(reinterpret_cast<char*>(&usernameSize), sizeof(usernameSize));
        std::string username(usernameSize, '\0');
        file.read(&username[0], usernameSize);

        size_t passwordSize;
        file.read(reinterpret_cast<char*>(&passwordSize), sizeof(passwordSize));
        std::string password(passwordSize, '\0');
        file.read(&password[0], passwordSize);

        users[username] = password;
    }
    file.close();
    return true;
}

std::string UserManagement::encrypt(const std::string& plaintext, const std::string& key) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);

    std::string ciphertext;
    ciphertext.resize(plaintext.size() + AES_BLOCK_SIZE);

    int len;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv);
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&ciphertext[0]), &len, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size());
    int ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&ciphertext[0]) + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(ciphertext_len);
    return std::string(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE) + ciphertext;
}

std::string UserManagement::decrypt(const std::string& ciphertext, const std::string& key) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[AES_BLOCK_SIZE];
    std::copy(ciphertext.begin(), ciphertext.begin() + AES_BLOCK_SIZE, iv);

    std::string plaintext;
    plaintext.resize(ciphertext.size() - AES_BLOCK_SIZE);

    int len;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv);
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&plaintext[0]), &len, reinterpret_cast<const unsigned char*>(ciphertext.c_str()) + AES_BLOCK_SIZE, ciphertext.size() - AES_BLOCK_SIZE);
    int plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&plaintext[0]) + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(plaintext_len);
    return plaintext;
}

std::string UserManagement::generateKey(const std::string& password) {
    return std::string(password.begin(), password.end());
}

bool UserManagement::userDataExists() const {
    std::ifstream file(userFile, std::ios::binary);
    return file.is_open();
}
