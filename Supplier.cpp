#include "../include/Supplier.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>

Supplier::Supplier()
    : supplierID(""), name(""), phone(""), address(""), email("") {}

Supplier::Supplier(const std::string& id, const std::string& n,
                   const std::string& ph, const std::string& addr,
                   const std::string& em)
    : supplierID(id), name(n), phone(ph), address(addr), email(em) {}

// ── InventoryItem interface ───────────────────────────────────────────────────
void Supplier::displayDetails() const {
    std::cout << "┌─────────────────────────────────────────┐\n"
              << "│  Supplier ID : " << supplierID << "\n"
              << "│  Name        : " << name       << "\n"
              << "│  Phone       : " << phone      << "\n"
              << "│  Email       : " << email      << "\n"
              << "│  Address     : " << address    << "\n"
              << "│  Products    : " << linkedProductIDs.size() << " linked\n"
              << "└─────────────────────────────────────────┘\n";
}

bool Supplier::validate() const {
    if (supplierID.empty()) { std::cerr << "[ERROR] Supplier ID empty.\n"; return false; }
    if (name.empty())       { std::cerr << "[ERROR] Supplier name empty.\n"; return false; }
    if (phone.empty())      { std::cerr << "[ERROR] Phone empty.\n"; return false; }
    if (email.find('@') == std::string::npos) {
        std::cerr << "[ERROR] Invalid email.\n"; return false;
    }
    return true;
}

std::string Supplier::toCSV() const {
    std::ostringstream oss;
    oss << supplierID << "," << name << "," << phone << ","
        << address << "," << email << "|";
    for (size_t i = 0; i < linkedProductIDs.size(); ++i) {
        if (i) oss << ";";
        oss << linkedProductIDs[i];
    }
    return oss.str();
}

std::string Supplier::getID()           const { return supplierID; }
std::string Supplier::generateSummary() const {
    return "[" + supplierID + "] " + name + " – " + phone;
}

// ── Getters ───────────────────────────────────────────────────────────────────
std::string              Supplier::getSupplierID()      const { return supplierID; }
std::string              Supplier::getName()             const { return name; }
std::string              Supplier::getPhone()            const { return phone; }
std::string              Supplier::getAddress()          const { return address; }
std::string              Supplier::getEmail()            const { return email; }
std::vector<std::string> Supplier::getLinkedProductIDs() const { return linkedProductIDs; }

// ── Setters ───────────────────────────────────────────────────────────────────
bool Supplier::setName(const std::string& n) {
    if (n.empty()) { std::cerr << "[ERROR] Name empty.\n"; return false; }
    name = n; return true;
}
bool Supplier::setPhone(const std::string& p) {
    if (p.empty()) { std::cerr << "[ERROR] Phone empty.\n"; return false; }
    phone = p; return true;
}
bool Supplier::setAddress(const std::string& a) {
    address = a; return true;
}
bool Supplier::setEmail(const std::string& e) {
    if (e.find('@') == std::string::npos) {
        std::cerr << "[ERROR] Invalid email.\n"; return false;
    }
    email = e; return true;
}

// ── Product link management ───────────────────────────────────────────────────
void Supplier::linkProduct(const std::string& pid) {
    if (!hasProduct(pid)) linkedProductIDs.push_back(pid);
}
void Supplier::unlinkProduct(const std::string& pid) {
    linkedProductIDs.erase(
        std::remove(linkedProductIDs.begin(), linkedProductIDs.end(), pid),
        linkedProductIDs.end());
}
bool Supplier::hasProduct(const std::string& pid) const {
    return std::find(linkedProductIDs.begin(), linkedProductIDs.end(), pid)
           != linkedProductIDs.end();
}

// ── CSV persistence ───────────────────────────────────────────────────────────
Supplier Supplier::fromCSV(const std::string& line) {
    // Format: id,name,phone,address,email|pid1;pid2;...
    size_t pipePos = line.find('|');
    std::string main = line.substr(0, pipePos);
    std::string prods = (pipePos != std::string::npos) ? line.substr(pipePos + 1) : "";

    std::istringstream ss(main);
    std::string id, name, phone, address, email;
    std::getline(ss, id,      ',');
    std::getline(ss, name,    ',');
    std::getline(ss, phone,   ',');
    std::getline(ss, address, ',');
    std::getline(ss, email);

    Supplier sup(id, name, phone, address, email);
    if (!prods.empty()) {
        std::istringstream ps(prods);
        std::string pid;
        while (std::getline(ps, pid, ';')) {
            if (!pid.empty()) sup.linkProduct(pid);
        }
    }
    return sup;
}
