#include "include/InventoryManager.h"
#include "include/ProductTypes.h"
#include <iostream>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
//  main.cpp  –  Demo / entry point for StockSense backend.
//  The actual UI (console menu or frontend) calls InventoryManager APIs.
//  This file shows a full integration test of every backend feature.
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "       StockSense – Inventory Backend Demo      \n";
    std::cout << "═══════════════════════════════════════════════\n\n";

    // ── 1. Initialise manager (loads persisted data from data/) ──────────────
    InventoryManager mgr("data/");

    // ── 2. Login ─────────────────────────────────────────────────────────────
    mgr.login("admin", "admin123");

    // ── 3. Add Suppliers ──────────────────────────────────────────────────────
    Supplier s1("SUP001", "FreshFarm Co.",  "+91-9876543210",
                "12 Anna Nagar, Chennai", "orders@freshfarm.in");
    Supplier s2("SUP002", "TechWorld Pvt.", "+91-9123456780",
                "3rd Floor, Tidel Park, Chennai", "supply@techworld.in");
    Supplier s3("SUP003", "MediPlus Ltd.",  "+91-9988776655",
                "22 Mount Road, Chennai", "stock@mediplus.in");

    mgr.addSupplier(s1);
    mgr.addSupplier(s2);
    mgr.addSupplier(s3);

    // ── 4. Add Products (polymorphic types) ───────────────────────────────────
    // FoodProduct
    auto bread = std::make_shared<FoodProduct>(
        "FOOD001", "Whole Wheat Bread", 120, 18.00, 35.00, 20,
        "2025-06-30", true, "SUP001");

    auto rice = std::make_shared<FoodProduct>(
        "FOOD002", "Basmati Rice 5kg", 60, 250.00, 380.00, 15,
        "2026-12-31", false, "SUP001");

    // ElectronicProduct
    auto laptop = std::make_shared<ElectronicProduct>(
        "ELEC001", "Dell Inspiron 15", 10, 45000.00, 58000.00, 3,
        24, 65.0, "SUP002");

    auto charger = std::make_shared<ElectronicProduct>(
        "ELEC002", "USB-C Fast Charger", 80, 350.00, 750.00, 10,
        12, 25.0, "SUP002");

    // MedicineProduct
    auto paracetamol = std::make_shared<MedicineProduct>(
        "MED001", "Paracetamol 500mg", 500, 1.50, 3.00, 100,
        "BATCH-2024-A", "2026-09-30", false, "SUP003");

    auto insulin = std::make_shared<MedicineProduct>(
        "MED002", "Insulin Glargine 100U", 40, 850.00, 1100.00, 10,
        "BATCH-INS-5", "2025-12-31", true, "SUP003");

    mgr.addProduct(bread);
    mgr.addProduct(rice);
    mgr.addProduct(laptop);
    mgr.addProduct(charger);
    mgr.addProduct(paracetamol);
    mgr.addProduct(insulin);

    // ── 5. Polymorphism demo: displayDetails() called on base pointer ─────────
    std::cout << "\n── Polymorphic displayDetails() demo ──\n";
    std::vector<std::shared_ptr<Product>> demo = {bread, laptop, paracetamol};
    for (auto& p : demo) p->displayDetails();   // virtual dispatch in action

    // ── 6. Stock operations ───────────────────────────────────────────────────
    std::cout << "\n── Stock Operations ──\n";
    mgr.stockIn("FOOD001", 50, "Morning delivery");
    mgr.stockOut("ELEC001", 2, "Customer sale – invoice #INV-001");
    mgr.stockOut("MED001", 480, "Hospital supply");   // will trigger low-stock alert

    // ── 7. Low-stock alert ────────────────────────────────────────────────────
    mgr.displayLowStockAlert();

    // ── 8. Search demo ────────────────────────────────────────────────────────
    std::cout << "\n── Search by name: 'rice' ──\n";
    auto results = mgr.searchByName("rice");
    for (auto& r : results) std::cout << "  " << r->generateSummary() << "\n";

    std::cout << "\n── Search by category: 'Electronics' ──\n";
    results = mgr.searchByCategory("Electronics");
    for (auto& r : results) std::cout << "  " << r->generateSummary() << "\n";

    // ── 9. Sorting demo ───────────────────────────────────────────────────────
    std::cout << "\n── Sort by quantity (ascending) ──\n";
    mgr.sortByQuantity(true);
    for (const auto& p : mgr.getProducts())
        std::cout << "  " << p->generateSummary() << "\n";

    // ── 10. User management (admin-only) ──────────────────────────────────────
    std::cout << "\n── Adding staff user ──\n";
    mgr.addUser("staff01", "staff@123", Role::STAFF);

    // ── 11. Reports ───────────────────────────────────────────────────────────
    mgr.reportCurrentStock();
    mgr.reportLowStock();
    mgr.reportSupplierWise();
    mgr.reportProfitMargins();
    mgr.reportTransactionHistory();

    // ── 12. Update product field ──────────────────────────────────────────────
    std::cout << "\n── Update laptop selling price ──\n";
    mgr.updateProduct("ELEC001", "sellingPrice", "60000.00");

    // ── 13. Staff role permission test ────────────────────────────────────────
    mgr.logout();
    mgr.login("staff01", "staff@123");
    std::cout << "\n── Staff attempting DELETE (should fail) ──\n";
    mgr.deleteProduct("FOOD001");          // blocked – staff cannot delete
    std::cout << "\n── Staff doing stockIn (should succeed) ──\n";
    mgr.stockIn("FOOD002", 20, "Restocking");

    // ── 14. Persist everything ────────────────────────────────────────────────
    mgr.logout();
    mgr.login("admin", "admin123");
    mgr.saveAll();
    std::cout << "\n[DONE] All data persisted. Exiting demo.\n";
    return 0;
}
