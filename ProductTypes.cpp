#include "../include/ProductTypes.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// ═════════════════════════════════════════════════════════════════════════════
//  FOOD PRODUCT
// ═════════════════════════════════════════════════════════════════════════════

FoodProduct::FoodProduct()
    : Product(), expiryDate(""), isPerishable(true) {}

FoodProduct::FoodProduct(const std::string& id, const std::string& name,
                         int qty, double buyPrice, double sellPrice,
                         int reorderLvl, const std::string& expiry,
                         bool perishable, const std::string& supplierID)
    : Product(id, name, "Food", qty, buyPrice, sellPrice, reorderLvl, supplierID),
      expiryDate(expiry), isPerishable(perishable) {}

// POLYMORPHISM – overrides base displayDetails() to show food-specific fields
void FoodProduct::displayDetails() const {
    Product::displayDetails();
    std::cout << "│  [Food]  Expiry    : " << expiryDate << "\n"
              << "│  [Food]  Perishable: " << (isPerishable ? "Yes" : "No") << "\n"
              << "│  [Food]  Expired   : " << (isExpired() ? "YES ⚠️" : "No") << "\n"
              << "└─────────────────────────────────────────┘\n";
}

// POLYMORPHISM – overrides stock-status to add expiry concern
std::string FoodProduct::calculateStockStatus() const {
    if (isExpired())            return "EXPIRED ⚠️";
    if (getQuantity() == 0)     return "OUT OF STOCK";
    if (getQuantity() <= getReorderLevel()) return "LOW STOCK";
    return "ADEQUATE";
}

std::string FoodProduct::generateSummary() const {
    return "[FOOD] " + Product::generateSummary() + " Expiry:" + expiryDate;
}

std::string FoodProduct::toCSV() const {
    std::ostringstream oss;
    oss << "FOOD," << getProductID() << "," << getName() << ",Food,"
        << getQuantity() << "," << std::fixed << std::setprecision(2)
        << getBuyingPrice() << "," << getSellingPrice() << ","
        << getReorderLevel() << "," << getSupplierID() << ","
        << expiryDate << "," << (isPerishable ? "1" : "0");
    return oss.str();
}

bool FoodProduct::isExpired() const {
    if (expiryDate.empty()) return false;
    // Compare "YYYY-MM-DD" strings lexicographically against today
    time_t now = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return expiryDate < std::string(buf);
}

std::string FoodProduct::getExpiryDate()   const { return expiryDate; }
bool        FoodProduct::getIsPerishable() const { return isPerishable; }

FoodProduct FoodProduct::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string type, id, name, cat, qty, buy, sell, reord, supID, expiry, perishable;
    std::getline(ss, type, ','); std::getline(ss, id,    ',');
    std::getline(ss, name, ','); std::getline(ss, cat,   ',');
    std::getline(ss, qty,  ','); std::getline(ss, buy,   ',');
    std::getline(ss, sell, ','); std::getline(ss, reord, ',');
    std::getline(ss, supID,','); std::getline(ss, expiry,',');
    std::getline(ss, perishable);
    return FoodProduct(id, name, std::stoi(qty),
                       std::stod(buy), std::stod(sell), std::stoi(reord),
                       expiry, perishable == "1", supID);
}

// ═════════════════════════════════════════════════════════════════════════════
//  ELECTRONIC PRODUCT
// ═════════════════════════════════════════════════════════════════════════════

ElectronicProduct::ElectronicProduct()
    : Product(), warrantyMonths(0), wattage(0.0) {}

ElectronicProduct::ElectronicProduct(const std::string& id, const std::string& name,
                                     int qty, double buyPrice, double sellPrice,
                                     int reorderLvl, int warranty, double watt,
                                     const std::string& supplierID)
    : Product(id, name, "Electronics", qty, buyPrice, sellPrice, reorderLvl, supplierID),
      warrantyMonths(warranty), wattage(watt) {}

// POLYMORPHISM – overrides base displayDetails()
void ElectronicProduct::displayDetails() const {
    Product::displayDetails();
    std::cout << "│  [Elec] Warranty   : " << warrantyMonths << " months\n"
              << "│  [Elec] Wattage    : " << std::fixed << std::setprecision(1)
              << wattage << " W\n"
              << "└─────────────────────────────────────────┘\n";
}

std::string ElectronicProduct::calculateStockStatus() const {
    // Electronics reorder at 2x the base level (longer lead time)
    if (getQuantity() == 0)                   return "OUT OF STOCK";
    if (getQuantity() <= getReorderLevel())    return "CRITICAL - ORDER NOW";
    if (getQuantity() <= getReorderLevel() * 2) return "LOW STOCK";
    return "ADEQUATE";
}

std::string ElectronicProduct::generateSummary() const {
    return "[ELEC] " + Product::generateSummary() +
           " Warranty:" + std::to_string(warrantyMonths) + "mo";
}

std::string ElectronicProduct::toCSV() const {
    std::ostringstream oss;
    oss << "ELECTRONIC," << getProductID() << "," << getName() << ",Electronics,"
        << getQuantity() << "," << std::fixed << std::setprecision(2)
        << getBuyingPrice() << "," << getSellingPrice() << ","
        << getReorderLevel() << "," << getSupplierID() << ","
        << warrantyMonths << "," << wattage;
    return oss.str();
}

int    ElectronicProduct::getWarrantyMonths() const { return warrantyMonths; }
double ElectronicProduct::getWattage()        const { return wattage; }

ElectronicProduct ElectronicProduct::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string type, id, name, cat, qty, buy, sell, reord, supID, warranty, watt;
    std::getline(ss, type, ','); std::getline(ss, id,    ',');
    std::getline(ss, name, ','); std::getline(ss, cat,   ',');
    std::getline(ss, qty,  ','); std::getline(ss, buy,   ',');
    std::getline(ss, sell, ','); std::getline(ss, reord, ',');
    std::getline(ss, supID,','); std::getline(ss, warranty, ',');
    std::getline(ss, watt);
    return ElectronicProduct(id, name, std::stoi(qty),
                             std::stod(buy), std::stod(sell), std::stoi(reord),
                             std::stoi(warranty), std::stod(watt), supID);
}

// ═════════════════════════════════════════════════════════════════════════════
//  MEDICINE PRODUCT
// ═════════════════════════════════════════════════════════════════════════════

MedicineProduct::MedicineProduct()
    : Product(), batchNumber(""), expiryDate(""), requiresPrescription(false) {}

MedicineProduct::MedicineProduct(const std::string& id, const std::string& name,
                                 int qty, double buyPrice, double sellPrice,
                                 int reorderLvl, const std::string& batch,
                                 const std::string& expiry, bool prescription,
                                 const std::string& supplierID)
    : Product(id, name, "Medicine", qty, buyPrice, sellPrice, reorderLvl, supplierID),
      batchNumber(batch), expiryDate(expiry), requiresPrescription(prescription) {}

// POLYMORPHISM – overrides base displayDetails()
void MedicineProduct::displayDetails() const {
    Product::displayDetails();
    std::cout << "│  [Med]  Batch      : " << batchNumber << "\n"
              << "│  [Med]  Expiry     : " << expiryDate << "\n"
              << "│  [Med]  Expired    : " << (isExpired() ? "YES ⚠️" : "No") << "\n"
              << "│  [Med]  Rx Needed  : " << (requiresPrescription ? "Yes" : "No") << "\n"
              << "└─────────────────────────────────────────┘\n";
}

std::string MedicineProduct::calculateStockStatus() const {
    if (isExpired())              return "EXPIRED – REMOVE ⚠️";
    if (getQuantity() == 0)       return "OUT OF STOCK";
    if (getQuantity() <= getReorderLevel()) return "LOW STOCK – REORDER";
    return "ADEQUATE";
}

std::string MedicineProduct::generateSummary() const {
    return "[MED] " + Product::generateSummary() +
           " Batch:" + batchNumber + " Expiry:" + expiryDate;
}

std::string MedicineProduct::toCSV() const {
    std::ostringstream oss;
    oss << "MEDICINE," << getProductID() << "," << getName() << ",Medicine,"
        << getQuantity() << "," << std::fixed << std::setprecision(2)
        << getBuyingPrice() << "," << getSellingPrice() << ","
        << getReorderLevel() << "," << getSupplierID() << ","
        << batchNumber << "," << expiryDate << ","
        << (requiresPrescription ? "1" : "0");
    return oss.str();
}

bool MedicineProduct::isExpired() const {
    if (expiryDate.empty()) return false;
    time_t now = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return expiryDate < std::string(buf);
}

std::string MedicineProduct::getBatchNumber()          const { return batchNumber; }
std::string MedicineProduct::getExpiryDate()           const { return expiryDate; }
bool        MedicineProduct::getRequiresPrescription() const { return requiresPrescription; }

MedicineProduct MedicineProduct::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string type, id, name, cat, qty, buy, sell, reord, supID, batch, expiry, rx;
    std::getline(ss, type, ','); std::getline(ss, id,    ',');
    std::getline(ss, name, ','); std::getline(ss, cat,   ',');
    std::getline(ss, qty,  ','); std::getline(ss, buy,   ',');
    std::getline(ss, sell, ','); std::getline(ss, reord, ',');
    std::getline(ss, supID,','); std::getline(ss, batch, ',');
    std::getline(ss, expiry,',');std::getline(ss, rx);
    return MedicineProduct(id, name, std::stoi(qty),
                           std::stod(buy), std::stod(sell), std::stoi(reord),
                           batch, expiry, rx == "1", supID);
}
