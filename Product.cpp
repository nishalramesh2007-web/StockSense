#include "../include/Product.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ── Default constructor ───────────────────────────────────────────────────────
Product::Product()
    : productID(""), name(""), category(""), quantity(0),
      buyingPrice(0.0), sellingPrice(0.0), reorderLevel(5), supplierID("") {}

// ── Parameterised constructor ─────────────────────────────────────────────────
Product::Product(const std::string& id, const std::string& n,
                 const std::string& cat, int qty,
                 double buyPrice, double sellPrice,
                 int reorderLvl, const std::string& supID)
    : productID(id), name(n), category(cat), quantity(qty),
      buyingPrice(buyPrice), sellingPrice(sellPrice),
      reorderLevel(reorderLvl), supplierID(supID) {}

// ── InventoryItem interface ───────────────────────────────────────────────────
void Product::displayDetails() const {
    std::cout << std::left
              << "┌─────────────────────────────────────────┐\n"
              << "│  Product ID   : " << std::setw(24) << productID    << "│\n"
              << "│  Name         : " << std::setw(24) << name         << "│\n"
              << "│  Category     : " << std::setw(24) << category     << "│\n"
              << "│  Quantity     : " << std::setw(24) << quantity     << "│\n"
              << "│  Buy Price    : " << std::setw(24) << std::fixed << std::setprecision(2) << buyingPrice  << "│\n"
              << "│  Sell Price   : " << std::setw(24) << sellingPrice << "│\n"
              << "│  Reorder Lvl  : " << std::setw(24) << reorderLevel << "│\n"
              << "│  Supplier ID  : " << std::setw(24) << supplierID   << "│\n"
              << "│  Stock Status : " << std::setw(24) << calculateStockStatus() << "│\n"
              << "└─────────────────────────────────────────┘\n";
}

bool Product::validate() const {
    if (productID.empty())        { std::cerr << "[ERROR] Product ID cannot be empty.\n";      return false; }
    if (name.empty())             { std::cerr << "[ERROR] Name cannot be empty.\n";             return false; }
    if (category.empty())         { std::cerr << "[ERROR] Category cannot be empty.\n";         return false; }
    if (quantity < 0)             { std::cerr << "[ERROR] Quantity cannot be negative.\n";      return false; }
    if (buyingPrice < 0)          { std::cerr << "[ERROR] Buying price cannot be negative.\n";  return false; }
    if (sellingPrice < 0)         { std::cerr << "[ERROR] Selling price cannot be negative.\n"; return false; }
    if (sellingPrice < buyingPrice){ std::cerr << "[WARN]  Selling price is less than buying price.\n"; }
    if (reorderLevel < 0)         { std::cerr << "[ERROR] Reorder level cannot be negative.\n"; return false; }
    return true;
}

std::string Product::toCSV() const {
    std::ostringstream oss;
    oss << "GENERIC," << productID << "," << name << "," << category << ","
        << quantity << "," << std::fixed << std::setprecision(2)
        << buyingPrice << "," << sellingPrice << ","
        << reorderLevel << "," << supplierID;
    return oss.str();
}

std::string Product::getID()           const { return productID; }
std::string Product::generateSummary() const {
    return "[" + productID + "] " + name + " (" + category + ") – Qty:" + std::to_string(quantity);
}

// ── Polymorphic stock status ──────────────────────────────────────────────────
std::string Product::calculateStockStatus() const {
    if (quantity == 0)               return "OUT OF STOCK";
    if (quantity <= reorderLevel)    return "LOW STOCK";
    if (quantity <= reorderLevel * 2) return "MODERATE";
    return "ADEQUATE";
}

// ── Getters ───────────────────────────────────────────────────────────────────
std::string Product::getProductID()    const { return productID; }
std::string Product::getName()         const { return name; }
std::string Product::getCategory()     const { return category; }
int         Product::getQuantity()     const { return quantity; }
double      Product::getBuyingPrice()  const { return buyingPrice; }
double      Product::getSellingPrice() const { return sellingPrice; }
int         Product::getReorderLevel() const { return reorderLevel; }
std::string Product::getSupplierID()   const { return supplierID; }

// ── Setters ───────────────────────────────────────────────────────────────────
bool Product::setName(const std::string& n) {
    if (n.empty()) { std::cerr << "[ERROR] Name cannot be empty.\n"; return false; }
    name = n; return true;
}
bool Product::setCategory(const std::string& c) {
    if (c.empty()) { std::cerr << "[ERROR] Category cannot be empty.\n"; return false; }
    category = c; return true;
}
bool Product::setQuantity(int q) {
    if (q < 0) { std::cerr << "[ERROR] Quantity cannot be negative.\n"; return false; }
    quantity = q; return true;
}
bool Product::setBuyingPrice(double p) {
    if (p < 0) { std::cerr << "[ERROR] Buying price cannot be negative.\n"; return false; }
    buyingPrice = p; return true;
}
bool Product::setSellingPrice(double p) {
    if (p < 0) { std::cerr << "[ERROR] Selling price cannot be negative.\n"; return false; }
    sellingPrice = p; return true;
}
bool Product::setReorderLevel(int lvl) {
    if (lvl < 0) { std::cerr << "[ERROR] Reorder level cannot be negative.\n"; return false; }
    reorderLevel = lvl; return true;
}
void Product::setSupplierID(const std::string& sid) { supplierID = sid; }

// ── Stock helpers ─────────────────────────────────────────────────────────────
bool Product::stockIn(int qty) {
    if (qty <= 0) { std::cerr << "[ERROR] Stock-in quantity must be positive.\n"; return false; }
    quantity += qty; return true;
}
bool Product::stockOut(int qty) {
    if (qty <= 0)        { std::cerr << "[ERROR] Stock-out quantity must be positive.\n"; return false; }
    if (qty > quantity)  { std::cerr << "[ERROR] Insufficient stock. Available: " << quantity << "\n"; return false; }
    quantity -= qty; return true;
}
bool Product::isLowStock() const { return quantity <= reorderLevel; }

// ── CSV re-hydration ─────────────────────────────────────────────────────────
Product Product::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string type, id, name, cat, qty, buy, sell, reord, supID;
    std::getline(ss, type,  ',');
    std::getline(ss, id,    ',');
    std::getline(ss, name,  ',');
    std::getline(ss, cat,   ',');
    std::getline(ss, qty,   ',');
    std::getline(ss, buy,   ',');
    std::getline(ss, sell,  ',');
    std::getline(ss, reord, ',');
    std::getline(ss, supID);
    return Product(id, name, cat, std::stoi(qty),
                   std::stod(buy), std::stod(sell),
                   std::stoi(reord), supID);
}
