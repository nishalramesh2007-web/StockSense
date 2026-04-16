#include "../include/Transaction.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <atomic>

static std::atomic<int> txCounter{1};

Transaction::Transaction()
    : transactionID(""), productID(""), date(""), quantity(0),
      action(ActionType::STOCK_IN), remarks(""), performedBy("system") {}

Transaction::Transaction(const std::string& txID, const std::string& prodID,
                         const std::string& dt, int qty,
                         ActionType act, const std::string& rem,
                         const std::string& by)
    : transactionID(txID), productID(prodID), date(dt), quantity(qty),
      action(act), remarks(rem), performedBy(by) {}

// ── InventoryItem interface ───────────────────────────────────────────────────
void Transaction::displayDetails() const {
    std::cout << "  TX:" << transactionID
              << "  Prod:" << productID
              << "  Date:" << date
              << "  Action:" << actionToString(action)
              << "  Qty:" << quantity
              << "  By:" << performedBy
              << "  Rem:" << remarks << "\n";
}

bool Transaction::validate() const {
    if (transactionID.empty()) { std::cerr << "[ERROR] Transaction ID empty.\n"; return false; }
    if (productID.empty())     { std::cerr << "[ERROR] Product ID empty.\n";     return false; }
    if (quantity <= 0)         { std::cerr << "[ERROR] Quantity must be > 0.\n"; return false; }
    return true;
}

std::string Transaction::toCSV() const {
    std::ostringstream oss;
    oss << transactionID << "," << productID << "," << date << ","
        << quantity << "," << actionToString(action) << ","
        << remarks << "," << performedBy;
    return oss.str();
}

std::string Transaction::getID()           const { return transactionID; }
std::string Transaction::generateSummary() const {
    return "[" + transactionID + "] " + actionToString(action)
           + " " + std::to_string(quantity) + " of " + productID + " on " + date;
}

// ── Getters ───────────────────────────────────────────────────────────────────
std::string Transaction::getTransactionID() const { return transactionID; }
std::string Transaction::getProductID()     const { return productID; }
std::string Transaction::getDate()          const { return date; }
int         Transaction::getQuantity()      const { return quantity; }
ActionType  Transaction::getAction()        const { return action; }
std::string Transaction::getRemarks()       const { return remarks; }
std::string Transaction::getPerformedBy()   const { return performedBy; }

// ── Helpers ───────────────────────────────────────────────────────────────────
std::string Transaction::actionToString(ActionType a) {
    switch (a) {
        case ActionType::STOCK_IN:    return "STOCK_IN";
        case ActionType::STOCK_OUT:   return "STOCK_OUT";
        case ActionType::ADJUSTMENT:  return "ADJUSTMENT";
        default:                      return "UNKNOWN";
    }
}
ActionType Transaction::stringToAction(const std::string& s) {
    if (s == "STOCK_IN")   return ActionType::STOCK_IN;
    if (s == "STOCK_OUT")  return ActionType::STOCK_OUT;
    return ActionType::ADJUSTMENT;
}
std::string Transaction::currentTimestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}
std::string Transaction::generateTxID() {
    int n = txCounter++;
    std::ostringstream oss;
    oss << "TX" << std::setw(6) << std::setfill('0') << n;
    return oss.str();
}

// ── CSV re-hydration ─────────────────────────────────────────────────────────
Transaction Transaction::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string txID, prodID, date, qty, action, remarks, by;
    std::getline(ss, txID,   ',');
    std::getline(ss, prodID, ',');
    std::getline(ss, date,   ',');
    std::getline(ss, qty,    ',');
    std::getline(ss, action, ',');
    std::getline(ss, remarks,',');
    std::getline(ss, by);
    return Transaction(txID, prodID, date, std::stoi(qty),
                       stringToAction(action), remarks, by);
}
