#include "Order.h"
#include <stdexcept>

namespace me {

// --- conversion enum → string ---
std::string toString(Side s) {
    switch (s) {
        case Side::BUY:  return "BUY";
        case Side::SELL: return "SELL";
    }
    throw std::runtime_error("Side invalide");
}

std::string toString(Type t) {
    switch (t) {
        case Type::LIMIT:  return "LIMIT";
        case Type::MARKET: return "MARKET";
    }
    throw std::runtime_error("Type invalide");
}

std::string toString(Action a) {
    switch (a) {
        case Action::NEW:    return "NEW";
        case Action::MODIFY: return "MODIFY";
        case Action::CANCEL: return "CANCEL";
    }
    throw std::runtime_error("Action invalide");
}

// --- parsing string → enum ---
Side sideFromString(const std::string& s) {
    if (s == "BUY")  return Side::BUY;
    if (s == "SELL") return Side::SELL;
    throw std::runtime_error("Side invalide: " + s);
}

Type typeFromString(const std::string& s) {
    if (s == "LIMIT")  return Type::LIMIT;
    if (s == "MARKET") return Type::MARKET;
    throw std::runtime_error("Type invalide: " + s);
}

Action actionFromString(const std::string& s) {
    if (s == "NEW")    return Action::NEW;
    if (s == "MODIFY") return Action::MODIFY;
    if (s == "CANCEL") return Action::CANCEL;
    throw std::runtime_error("Action invalide: " + s);
}

// --- méthode membre Order ---
std::string Order::toString() const {
    return std::to_string(timestamp) + " | "
    + std::to_string(order_id)   + " | "
    + instrument                  + " | "
    + me::toString(side)          + " | "
    + me::toString(type)          + " | "
    + std::to_string(quantity)    + " @ "
    + std::to_string(price)       + " | "
    + me::toString(action);
}

// --- usines statiques pour construire un Order proprement ---
Order Order::makeLimit(uint64_t ts, uint64_t id,
                       std::string instr,
                       Side s, uint64_t qty, double p,
                       Action a) {
    Order o{ts, id, std::move(instr), s, Type::LIMIT, qty, p, a};
    o.validate();
    return o;
}

Order Order::makeMarket(uint64_t ts, uint64_t id,
                        std::string instr,
                        Side s, uint64_t qty,
                        Action a) {
    Order o{ts, id, std::move(instr), s, Type::MARKET, qty, 0.0, a};
    o.validate();
    return o;
}

// --- validation interne ---
void Order::validate() const {
    // Pour NEW ou MODIFY, on exige quantité > 0
    if ((action == Action::NEW || action == Action::MODIFY) && quantity == 0)
        throw std::runtime_error("NEW/MODIFY avec quantité = 0");
    // Pour les LIMIT NEW/MODIFY, le prix doit être > 0
    if ((action == Action::NEW || action == Action::MODIFY)
         && type == Type::LIMIT
         && price <= 0.0)
        throw std::runtime_error("LIMIT avec prix non strictement positif");
    // CANCEL passe toujours
}

// --- opérateur de flux pour cout << order ---
std::ostream& operator<<(std::ostream& os, Order const& o) {
    return os << o.toString();
}

} // namespace me
