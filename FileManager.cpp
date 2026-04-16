#include "../include/FileManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

FileManager::FileManager(const std::string& dataDir) {
    productFile     = dataDir + "products.csv";
    supplierFile    = dataDir + "suppliers.csv";
    transactionFile = dataDir + "transactions.csv";
    userFile        = dataDir + "users.csv";

    // Ensure data directory and files exist
    fs::create_directories(dataDir);
    ensureFileExists(productFile);
    ensureFileExists(supplierFile);
    ensureFileExists(transactionFile);
    ensureFileExists(userFile);
}

// ── Internal helpers ──────────────────────────────────────────────────────────
void FileManager::ensureFileExists(const std::string& filepath) const {
    std::ifstream f(filepath);
    if (!f.good()) {
        std::ofstream create(filepath);
        if (!create) {
            std::cerr << "[FileManager] Cannot create file: " << filepath << "\n";
        }
    }
}

std::vector<std::string> FileManager::readLines(const std::string& filepath) const {
    std::vector<std::string> lines;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[FileManager] Cannot open: " << filepath << "\n";
        return lines;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

bool FileManager::writeLines(const std::string& filepath,
                              const std::vector<std::string>& lines) const {
    std::ofstream file(filepath, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[FileManager] Cannot write: " << filepath << "\n";
        return false;
    }
    for (const auto& l : lines) file << l << "\n";
    return true;
}

// ── Products ──────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Product>> FileManager::loadProducts() const {
    std::vector<std::shared_ptr<Product>> products;
    auto lines = readLines(productFile);
    for (const auto& line : lines) {
        if (line.empty()) continue;
        try {
            std::string type = line.substr(0, line.find(','));
            if (type == "FOOD") {
                products.push_back(
                    std::make_shared<FoodProduct>(FoodProduct::fromCSV(line)));
            } else if (type == "ELECTRONIC") {
                products.push_back(
                    std::make_shared<ElectronicProduct>(ElectronicProduct::fromCSV(line)));
            } else if (type == "MEDICINE") {
                products.push_back(
                    std::make_shared<MedicineProduct>(MedicineProduct::fromCSV(line)));
            } else {
                products.push_back(
                    std::make_shared<Product>(Product::fromCSV(line)));
            }
        } catch (const std::exception& e) {
            std::cerr << "[FileManager] Skipping malformed product row: " << e.what() << "\n";
        }
    }
    return products;
}

bool FileManager::saveProducts(
    const std::vector<std::shared_ptr<Product>>& products) const {
    std::vector<std::string> lines;
    for (const auto& p : products) lines.push_back(p->toCSV());
    return writeLines(productFile, lines);
}

// ── Suppliers ─────────────────────────────────────────────────────────────────
std::vector<Supplier> FileManager::loadSuppliers() const {
    std::vector<Supplier> suppliers;
    auto lines = readLines(supplierFile);
    for (const auto& line : lines) {
        if (line.empty()) continue;
        try {
            suppliers.push_back(Supplier::fromCSV(line));
        } catch (...) {
            std::cerr << "[FileManager] Skipping malformed supplier row.\n";
        }
    }
    return suppliers;
}

bool FileManager::saveSuppliers(const std::vector<Supplier>& suppliers) const {
    std::vector<std::string> lines;
    for (const auto& s : suppliers) lines.push_back(s.toCSV());
    return writeLines(supplierFile, lines);
}

// ── Transactions ──────────────────────────────────────────────────────────────
std::vector<Transaction> FileManager::loadTransactions() const {
    std::vector<Transaction> txns;
    auto lines = readLines(transactionFile);
    for (const auto& line : lines) {
        if (line.empty()) continue;
        try {
            txns.push_back(Transaction::fromCSV(line));
        } catch (...) {
            std::cerr << "[FileManager] Skipping malformed transaction row.\n";
        }
    }
    return txns;
}

bool FileManager::saveTransactions(const std::vector<Transaction>& txns) const {
    std::vector<std::string> lines;
    for (const auto& t : txns) lines.push_back(t.toCSV());
    return writeLines(transactionFile, lines);
}

bool FileManager::appendTransaction(const Transaction& txn) const {
    std::ofstream file(transactionFile, std::ios::app);
    if (!file.is_open()) return false;
    file << txn.toCSV() << "\n";
    return true;
}

// ── Users ─────────────────────────────────────────────────────────────────────
std::vector<User> FileManager::loadUsers() const {
    std::vector<User> users;
    auto lines = readLines(userFile);
    for (const auto& line : lines) {
        if (line.empty()) continue;
        try {
            users.push_back(User::fromCSV(line));
        } catch (...) {
            std::cerr << "[FileManager] Skipping malformed user row.\n";
        }
    }
    return users;
}

bool FileManager::saveUsers(const std::vector<User>& users) const {
    std::vector<std::string> lines;
    for (const auto& u : users) lines.push_back(u.toCSV());
    return writeLines(userFile, lines);
}

// ── Utility ───────────────────────────────────────────────────────────────────
bool FileManager::backupData(const std::string& backupDir) const {
    try {
        fs::create_directories(backupDir);
        fs::copy(productFile,     backupDir + "products.csv",     fs::copy_options::overwrite_existing);
        fs::copy(supplierFile,    backupDir + "suppliers.csv",    fs::copy_options::overwrite_existing);
        fs::copy(transactionFile, backupDir + "transactions.csv", fs::copy_options::overwrite_existing);
        fs::copy(userFile,        backupDir + "users.csv",        fs::copy_options::overwrite_existing);
        std::cout << "[FileManager] Backup complete to: " << backupDir << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[FileManager] Backup failed: " << e.what() << "\n";
        return false;
    }
}

std::string FileManager::escapeCsv(const std::string& field) {
    if (field.find(',') == std::string::npos &&
        field.find('"') == std::string::npos &&
        field.find('\n') == std::string::npos)
        return field;
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') escaped += "\"\"";
        else escaped += c;
    }
    escaped += "\"";
    return escaped;
}

std::vector<std::string> FileManager::parseCsvLine(const std::string& line) {
    std::vector<std::string> result;
    std::string token;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                token += '"'; ++i;
            } else inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(token); token.clear();
        } else token += c;
    }
    result.push_back(token);
    return result;
}
