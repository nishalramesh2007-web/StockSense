#include "../include/InventoryManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// ── Constructor ───────────────────────────────────────────────────────────────
InventoryManager::InventoryManager(const std::string& dataDir)
    : fileManager(dataDir), currentUser(nullptr) {
    loadAll();
    // Seed a default admin if no users exist
    if (users.empty()) {
        users.emplace_back("admin", "admin123", Role::ADMIN);
        persistUsers();
        std::cout << "[InventoryManager] Default admin created. "
                     "Change password after first login.\n";
    }
}

// ── Auth ──────────────────────────────────────────────────────────────────────
bool InventoryManager::login(const std::string& username,
                              const std::string& password) {
    int idx = findUserIndex(username);
    if (idx < 0) {
        std::cerr << "[AUTH] User not found: " << username << "\n";
        return false;
    }
    if (!users[idx].authenticate(password)) {
        std::cerr << "[AUTH] Incorrect password for: " << username << "\n";
        return false;
    }
    currentUser = &users[idx];
    std::cout << "[AUTH] Logged in as " << username
              << " (" << currentUser->getRoleString() << ")\n";
    return true;
}

void InventoryManager::logout() {
    if (currentUser)
        std::cout << "[AUTH] " << currentUser->getUsername() << " logged out.\n";
    currentUser = nullptr;
}
bool  InventoryManager::isLoggedIn()     const { return currentUser != nullptr; }
User* InventoryManager::getCurrentUser() const { return currentUser; }

// ── Load / Save ───────────────────────────────────────────────────────────────
void InventoryManager::loadAll() {
    products     = fileManager.loadProducts();
    suppliers    = fileManager.loadSuppliers();
    transactions = fileManager.loadTransactions();
    users        = fileManager.loadUsers();
    std::cout << "[InventoryManager] Loaded: "
              << products.size()     << " products, "
              << suppliers.size()    << " suppliers, "
              << transactions.size() << " transactions, "
              << users.size()        << " users.\n";
}
void InventoryManager::saveAll() {
    fileManager.saveProducts(products);
    fileManager.saveSuppliers(suppliers);
    fileManager.saveTransactions(transactions);
    fileManager.saveUsers(users);
}

// ── Internal helpers ──────────────────────────────────────────────────────────
int InventoryManager::findProductIndex(const std::string& pid) const {
    for (int i = 0; i < (int)products.size(); ++i)
        if (products[i]->getProductID() == pid) return i;
    return -1;
}
int InventoryManager::findSupplierIndex(const std::string& sid) const {
    for (int i = 0; i < (int)suppliers.size(); ++i)
        if (suppliers[i].getSupplierID() == sid) return i;
    return -1;
}
int InventoryManager::findUserIndex(const std::string& uname) const {
    for (int i = 0; i < (int)users.size(); ++i)
        if (users[i].getUsername() == uname) return i;
    return -1;
}
bool InventoryManager::isDuplicateProductID(const std::string& id) const {
    return findProductIndex(id) >= 0;
}
bool InventoryManager::isDuplicateSupplierID(const std::string& id) const {
    return findSupplierIndex(id) >= 0;
}
void InventoryManager::persistProducts()  { fileManager.saveProducts(products); }
void InventoryManager::persistSuppliers() { fileManager.saveSuppliers(suppliers); }
void InventoryManager::persistUsers()     { fileManager.saveUsers(users); }

// ── Product CRUD ──────────────────────────────────────────────────────────────
bool InventoryManager::addProduct(std::shared_ptr<Product> product) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission to add products.\n";
        return false;
    }
    if (!product->validate()) return false;
    if (isDuplicateProductID(product->getProductID())) {
        std::cerr << "[ERROR] Duplicate Product ID: " << product->getProductID() << "\n";
        return false;
    }
    // Link supplier
    if (!product->getSupplierID().empty()) {
        int sIdx = findSupplierIndex(product->getSupplierID());
        if (sIdx >= 0) suppliers[sIdx].linkProduct(product->getProductID());
    }
    products.push_back(product);
    persistProducts();
    std::cout << "[OK] Product added: " << product->getProductID() << "\n";
    return true;
}

bool InventoryManager::updateProduct(const std::string& productID,
                                      const std::string& field,
                                      const std::string& value) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    int idx = findProductIndex(productID);
    if (idx < 0) { std::cerr << "[ERROR] Product not found: " << productID << "\n"; return false; }

    auto& p = products[idx];
    bool ok = false;
    if      (field == "name")         ok = p->setName(value);
    else if (field == "category")     ok = p->setCategory(value);
    else if (field == "quantity")     ok = p->setQuantity(std::stoi(value));
    else if (field == "buyingPrice")  ok = p->setBuyingPrice(std::stod(value));
    else if (field == "sellingPrice") ok = p->setSellingPrice(std::stod(value));
    else if (field == "reorderLevel") ok = p->setReorderLevel(std::stoi(value));
    else if (field == "supplierID")   { p->setSupplierID(value); ok = true; }
    else { std::cerr << "[ERROR] Unknown field: " << field << "\n"; return false; }

    if (ok) { persistProducts(); std::cout << "[OK] Updated " << field << " for " << productID << "\n"; }
    return ok;
}

bool InventoryManager::deleteProduct(const std::string& productID) {
    if (!currentUser || !currentUser->canDelete()) {
        std::cerr << "[PERM] Only ADMIN can delete products.\n"; return false;
    }
    int idx = findProductIndex(productID);
    if (idx < 0) { std::cerr << "[ERROR] Product not found.\n"; return false; }

    // Unlink from supplier
    std::string supID = products[idx]->getSupplierID();
    if (!supID.empty()) {
        int sIdx = findSupplierIndex(supID);
        if (sIdx >= 0) suppliers[sIdx].unlinkProduct(productID);
        persistSuppliers();
    }
    products.erase(products.begin() + idx);
    persistProducts();
    std::cout << "[OK] Product deleted: " << productID << "\n";
    return true;
}

std::shared_ptr<Product> InventoryManager::getProduct(const std::string& pid) const {
    int idx = findProductIndex(pid);
    return (idx >= 0) ? products[idx] : nullptr;
}

// ── Stock operations ──────────────────────────────────────────────────────────
bool InventoryManager::stockIn(const std::string& productID,
                                int qty, const std::string& remarks) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    int idx = findProductIndex(productID);
    if (idx < 0) { std::cerr << "[ERROR] Product not found.\n"; return false; }
    if (!products[idx]->stockIn(qty)) return false;

    Transaction tx(Transaction::generateTxID(), productID,
                   Transaction::currentTimestamp(), qty,
                   ActionType::STOCK_IN, remarks,
                   currentUser->getUsername());
    transactions.push_back(tx);
    fileManager.appendTransaction(tx);
    persistProducts();
    std::cout << "[OK] Stock-in: +" << qty << " for " << productID << "\n";
    return true;
}

bool InventoryManager::stockOut(const std::string& productID,
                                 int qty, const std::string& remarks) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    int idx = findProductIndex(productID);
    if (idx < 0) { std::cerr << "[ERROR] Product not found.\n"; return false; }
    if (!products[idx]->stockOut(qty)) return false;

    Transaction tx(Transaction::generateTxID(), productID,
                   Transaction::currentTimestamp(), qty,
                   ActionType::STOCK_OUT, remarks,
                   currentUser->getUsername());
    transactions.push_back(tx);
    fileManager.appendTransaction(tx);
    persistProducts();

    if (products[idx]->isLowStock())
        std::cout << "[ALERT] Low stock on " << productID
                  << "! Qty: " << products[idx]->getQuantity()
                  << " (Reorder at: " << products[idx]->getReorderLevel() << ")\n";
    return true;
}

bool InventoryManager::adjustStock(const std::string& productID,
                                    int newQty, const std::string& remarks) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    int idx = findProductIndex(productID);
    if (idx < 0) { std::cerr << "[ERROR] Product not found.\n"; return false; }
    if (!products[idx]->setQuantity(newQty)) return false;

    Transaction tx(Transaction::generateTxID(), productID,
                   Transaction::currentTimestamp(), newQty,
                   ActionType::ADJUSTMENT, remarks,
                   currentUser->getUsername());
    transactions.push_back(tx);
    fileManager.appendTransaction(tx);
    persistProducts();
    std::cout << "[OK] Adjusted stock for " << productID << " to " << newQty << "\n";
    return true;
}

// ── Display ───────────────────────────────────────────────────────────────────
void InventoryManager::displayAllProducts() const {
    std::cout << "\n══════════════════════  INVENTORY  ══════════════════════\n";
    for (const auto& p : products) p->displayDetails();
    std::cout << "Total products: " << products.size() << "\n\n";
}

void InventoryManager::displayProduct(const std::string& pid) const {
    auto p = getProduct(pid);
    if (p) p->displayDetails();
    else std::cerr << "[ERROR] Product not found: " << pid << "\n";
}

void InventoryManager::displayAllSuppliers() const {
    std::cout << "\n══════════════════════  SUPPLIERS  ══════════════════════\n";
    for (const auto& s : suppliers) s.displayDetails();
}

void InventoryManager::displayTransactionHistory(const std::string& pid) const {
    std::cout << "\n══════════════════  TRANSACTION HISTORY  ═════════════════\n";
    for (const auto& t : transactions) {
        if (pid.empty() || t.getProductID() == pid)
            t.displayDetails();
    }
}

// ── Search ────────────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Product>>
InventoryManager::searchByName(const std::string& keyword) const {
    std::vector<std::shared_ptr<Product>> res;
    std::string kw = keyword;
    std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
    for (const auto& p : products) {
        std::string n = p->getName();
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        if (n.find(kw) != std::string::npos) res.push_back(p);
    }
    return res;
}

std::vector<std::shared_ptr<Product>>
InventoryManager::searchByCategory(const std::string& cat) const {
    std::vector<std::shared_ptr<Product>> res;
    for (const auto& p : products)
        if (p->getCategory() == cat) res.push_back(p);
    return res;
}

std::vector<std::shared_ptr<Product>>
InventoryManager::searchBySupplier(const std::string& supID) const {
    std::vector<std::shared_ptr<Product>> res;
    for (const auto& p : products)
        if (p->getSupplierID() == supID) res.push_back(p);
    return res;
}

std::vector<std::shared_ptr<Product>>
InventoryManager::searchByStockStatus(const std::string& status) const {
    std::vector<std::shared_ptr<Product>> res;
    for (const auto& p : products)
        if (p->calculateStockStatus() == status) res.push_back(p);
    return res;
}

// ── Sorting ───────────────────────────────────────────────────────────────────
void InventoryManager::sortByQuantity(bool ascending) {
    std::sort(products.begin(), products.end(),
        [ascending](const auto& a, const auto& b) {
            return ascending ? a->getQuantity() < b->getQuantity()
                             : a->getQuantity() > b->getQuantity();
        });
}
void InventoryManager::sortBySellingPrice(bool ascending) {
    std::sort(products.begin(), products.end(),
        [ascending](const auto& a, const auto& b) {
            return ascending ? a->getSellingPrice() < b->getSellingPrice()
                             : a->getSellingPrice() > b->getSellingPrice();
        });
}
void InventoryManager::sortByName(bool ascending) {
    std::sort(products.begin(), products.end(),
        [ascending](const auto& a, const auto& b) {
            return ascending ? a->getName() < b->getName()
                             : a->getName() > b->getName();
        });
}
void InventoryManager::sortByCategory(bool ascending) {
    std::sort(products.begin(), products.end(),
        [ascending](const auto& a, const auto& b) {
            return ascending ? a->getCategory() < b->getCategory()
                             : a->getCategory() > b->getCategory();
        });
}
void InventoryManager::sortLowStockFirst() {
    std::sort(products.begin(), products.end(),
        [](const auto& a, const auto& b) {
            return a->isLowStock() && !b->isLowStock();
        });
}

// ── Stock alerts ──────────────────────────────────────────────────────────────
std::vector<std::shared_ptr<Product>> InventoryManager::getLowStockProducts() const {
    std::vector<std::shared_ptr<Product>> res;
    for (const auto& p : products)
        if (p->isLowStock()) res.push_back(p);
    return res;
}

void InventoryManager::displayLowStockAlert() const {
    auto low = getLowStockProducts();
    if (low.empty()) { std::cout << "[OK] All products adequately stocked.\n"; return; }
    std::cout << "\n⚠️  LOW STOCK ALERT (" << low.size() << " products):\n";
    for (const auto& p : low)
        std::cout << "  " << p->generateSummary()
                  << "  [Reorder: " << p->getReorderLevel() << "]\n";
}

// ── Supplier management ───────────────────────────────────────────────────────
bool InventoryManager::addSupplier(const Supplier& supplier) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    if (!supplier.validate()) return false;
    if (isDuplicateSupplierID(supplier.getSupplierID())) {
        std::cerr << "[ERROR] Duplicate Supplier ID.\n"; return false;
    }
    suppliers.push_back(supplier);
    persistSuppliers();
    std::cout << "[OK] Supplier added: " << supplier.getSupplierID() << "\n";
    return true;
}

bool InventoryManager::updateSupplier(const std::string& supplierID,
                                       const std::string& field,
                                       const std::string& value) {
    if (!currentUser || !currentUser->canWrite()) {
        std::cerr << "[PERM] Insufficient permission.\n"; return false;
    }
    int idx = findSupplierIndex(supplierID);
    if (idx < 0) { std::cerr << "[ERROR] Supplier not found.\n"; return false; }
    auto& s = suppliers[idx];
    bool ok = false;
    if      (field == "name")    ok = s.setName(value);
    else if (field == "phone")   ok = s.setPhone(value);
    else if (field == "address") ok = s.setAddress(value);
    else if (field == "email")   ok = s.setEmail(value);
    else { std::cerr << "[ERROR] Unknown field: " << field << "\n"; return false; }
    if (ok) persistSuppliers();
    return ok;
}

bool InventoryManager::deleteSupplier(const std::string& supplierID) {
    if (!currentUser || !currentUser->canDelete()) {
        std::cerr << "[PERM] Only ADMIN can delete suppliers.\n"; return false;
    }
    int idx = findSupplierIndex(supplierID);
    if (idx < 0) { std::cerr << "[ERROR] Supplier not found.\n"; return false; }
    suppliers.erase(suppliers.begin() + idx);
    persistSuppliers();
    std::cout << "[OK] Supplier deleted: " << supplierID << "\n";
    return true;
}

Supplier* InventoryManager::getSupplier(const std::string& supplierID) {
    int idx = findSupplierIndex(supplierID);
    return (idx >= 0) ? &suppliers[idx] : nullptr;
}

// ── User management ───────────────────────────────────────────────────────────
bool InventoryManager::addUser(const std::string& username,
                                const std::string& password, Role role) {
    if (!currentUser || !currentUser->canDelete()) {
        std::cerr << "[PERM] Only ADMIN can add users.\n"; return false;
    }
    if (findUserIndex(username) >= 0) {
        std::cerr << "[ERROR] Username already exists.\n"; return false;
    }
    users.emplace_back(username, password, role);
    persistUsers();
    std::cout << "[OK] User created: " << username << "\n";
    return true;
}
bool InventoryManager::deleteUser(const std::string& username) {
    if (!currentUser || !currentUser->canDelete()) {
        std::cerr << "[PERM] Only ADMIN can delete users.\n"; return false;
    }
    int idx = findUserIndex(username);
    if (idx < 0) { std::cerr << "[ERROR] User not found.\n"; return false; }
    users.erase(users.begin() + idx);
    persistUsers();
    return true;
}
bool InventoryManager::setUserActive(const std::string& username, bool active) {
    if (!currentUser || !currentUser->canDelete()) {
        std::cerr << "[PERM] Only ADMIN can change user status.\n"; return false;
    }
    int idx = findUserIndex(username);
    if (idx < 0) return false;
    users[idx].setActive(active);
    persistUsers();
    return true;
}

// ── Reports ───────────────────────────────────────────────────────────────────
void InventoryManager::reportCurrentStock() const {
    if (!currentUser || !currentUser->canReport()) {
        std::cerr << "[PERM] Only ADMIN can generate reports.\n"; return;
    }
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║              CURRENT STOCK REPORT                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    std::cout << std::left
              << std::setw(12) << "ProductID"
              << std::setw(22) << "Name"
              << std::setw(14) << "Category"
              << std::setw(8)  << "Qty"
              << std::setw(12) << "SellPrice"
              << std::setw(16) << "Status" << "\n";
    std::cout << std::string(84, '-') << "\n";
    for (const auto& p : products) {
        std::cout << std::left
                  << std::setw(12) << p->getProductID()
                  << std::setw(22) << p->getName()
                  << std::setw(14) << p->getCategory()
                  << std::setw(8)  << p->getQuantity()
                  << std::setw(12) << std::fixed << std::setprecision(2) << p->getSellingPrice()
                  << std::setw(16) << p->calculateStockStatus() << "\n";
    }
    std::cout << std::string(84, '-') << "\n"
              << "Total SKUs: " << products.size() << "\n\n";
}

void InventoryManager::reportLowStock() const {
    if (!currentUser || !currentUser->canReport()) {
        std::cerr << "[PERM] Only ADMIN can generate reports.\n"; return;
    }
    auto low = getLowStockProducts();
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║                LOW STOCK REPORT                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    if (low.empty()) { std::cout << "  All products are adequately stocked.\n\n"; return; }
    for (const auto& p : low) {
        std::cout << "  " << p->generateSummary()
                  << "  ReorderAt:" << p->getReorderLevel()
                  << "  Status:" << p->calculateStockStatus() << "\n";
    }
    std::cout << "\n";
}

void InventoryManager::reportSupplierWise() const {
    if (!currentUser || !currentUser->canReport()) {
        std::cerr << "[PERM] Only ADMIN can generate reports.\n"; return;
    }
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║               SUPPLIER-WISE REPORT                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    for (const auto& s : suppliers) {
        std::cout << "\n  Supplier: " << s.getName()
                  << " [" << s.getSupplierID() << "] – " << s.getPhone() << "\n";
        auto plist = searchBySupplier(s.getSupplierID());
        if (plist.empty()) { std::cout << "    (No products linked)\n"; continue; }
        for (const auto& p : plist)
            std::cout << "    " << p->generateSummary() << "\n";
    }
    std::cout << "\n";
}

void InventoryManager::reportTransactionHistory(const std::string& pid) const {
    if (!currentUser || !currentUser->canReport()) {
        std::cerr << "[PERM] Only ADMIN can generate reports.\n"; return;
    }
    displayTransactionHistory(pid);
}

void InventoryManager::reportProfitMargins() const {
    if (!currentUser || !currentUser->canReport()) {
        std::cerr << "[PERM] Only ADMIN can generate reports.\n"; return;
    }
    std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║               PROFIT MARGIN REPORT                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    std::cout << std::left
              << std::setw(12) << "ProductID"
              << std::setw(22) << "Name"
              << std::setw(12) << "BuyPrice"
              << std::setw(12) << "SellPrice"
              << std::setw(12) << "Margin(%)" << "\n";
    std::cout << std::string(70, '-') << "\n";
    for (const auto& p : products) {
        double margin = 0.0;
        if (p->getBuyingPrice() > 0)
            margin = ((p->getSellingPrice() - p->getBuyingPrice()) /
                       p->getBuyingPrice()) * 100.0;
        std::cout << std::left
                  << std::setw(12) << p->getProductID()
                  << std::setw(22) << p->getName()
                  << std::setw(12) << std::fixed << std::setprecision(2) << p->getBuyingPrice()
                  << std::setw(12) << p->getSellingPrice()
                  << std::setw(12) << margin << "\n";
    }
    std::cout << "\n";
}

// ── Getters ───────────────────────────────────────────────────────────────────
const std::vector<std::shared_ptr<Product>>& InventoryManager::getProducts()     const { return products; }
const std::vector<Supplier>&                  InventoryManager::getSuppliers()    const { return suppliers; }
const std::vector<Transaction>&               InventoryManager::getTransactions() const { return transactions; }
