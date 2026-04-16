#include "../include/User.h"
#include <sstream>
#include <iostream>
#include <functional>
#include <iomanip>

// ── Simple deterministic hash (not cryptographic – replace with bcrypt in prod) ─
std::string User::hashPassword(const std::string& raw) {
    size_t h = std::hash<std::string>{}(raw + "stocksense_salt");
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << h;
    return oss.str();
}

User::User()
    : username(""), passwordHash(""), role(Role::VIEWER), isActive(true) {}

User::User(const std::string& uname, const std::string& rawPass, Role r)
    : username(uname), passwordHash(hashPassword(rawPass)), role(r), isActive(true) {}

// ── Getters ───────────────────────────────────────────────────────────────────
std::string User::getUsername()   const { return username; }
Role        User::getRole()       const { return role; }
bool        User::getIsActive()   const { return isActive; }

std::string User::getRoleString() const {
    switch (role) {
        case Role::ADMIN:  return "ADMIN";
        case Role::STAFF:  return "STAFF";
        case Role::VIEWER: return "VIEWER";
        default:           return "UNKNOWN";
    }
}

// ── Auth ──────────────────────────────────────────────────────────────────────
bool User::authenticate(const std::string& rawPassword) const {
    return isActive && (hashPassword(rawPassword) == passwordHash);
}
void User::setPassword(const std::string& rawPassword) {
    passwordHash = hashPassword(rawPassword);
}
void User::setActive(bool active) { isActive = active; }

// ── Permission checks ─────────────────────────────────────────────────────────
bool User::canWrite()  const { return isActive && (role == Role::ADMIN || role == Role::STAFF); }
bool User::canDelete() const { return isActive && (role == Role::ADMIN); }
bool User::canReport() const { return isActive && (role == Role::ADMIN); }

// ── Persistence ───────────────────────────────────────────────────────────────
std::string User::toCSV() const {
    return username + "," + passwordHash + "," + getRoleString() + ","
           + (isActive ? "1" : "0");
}

User User::fromCSV(const std::string& line) {
    std::istringstream ss(line);
    std::string uname, hash, roleStr, active;
    std::getline(ss, uname,   ',');
    std::getline(ss, hash,    ',');
    std::getline(ss, roleStr, ',');
    std::getline(ss, active);

    Role r = Role::VIEWER;
    if (roleStr == "ADMIN") r = Role::ADMIN;
    else if (roleStr == "STAFF") r = Role::STAFF;

    User u;
    u.username     = uname;
    u.passwordHash = hash;
    u.role         = r;
    u.isActive     = (active == "1");
    return u;
}
