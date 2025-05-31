#include "OrderBook.h"
#include "Logger.h"
#include <stdexcept>

namespace me {

std::vector<Execution> OrderBook::process(const Order& o) {
    // --- cas spécial : premier NEW LIMIT sur ce carnet, rien à matcher ---
    if (o.action == Action::NEW
     && o.type   == Type::LIMIT
     && buyBook_.empty()
     && sellBook_.empty())
    {
        // on stocke l'ordre, sans jamais renvoyer de fills
        addLimitOrder(o);
        return {};
    }

    // CANCEL d’abord
    if (o.action == Action::CANCEL) {
        cancelOrder(o);
        return {};
    }
    // MODIFY : on annule, puis on retombe sur le NEW
    if (o.action == Action::MODIFY) {
        cancelOrder(o);
        // et on laisse tomber dans le NEW ci-dessous
    }
    // NEW (ou MODIFY après suppression)
    switch (o.type) {
        case Type::LIMIT:  return matchLimit(o);
        case Type::MARKET: return matchMarket(o);
        default:
            throw std::runtime_error("Type d'ordre inconnu");
    }
} // namespace me

void OrderBook::addLimitOrder(const Order& o) {
    if (o.side == Side::BUY)
        buyBook_[o.price].push_back(o);
    else
        sellBook_[o.price].push_back(o);
}

void OrderBook::cancelOrder(const Order& o) {
    if (o.side == Side::BUY) {
        // Annulation dans le book BUY
        auto it = buyBook_.find(o.price);
        if (it != buyBook_.end()) {
            auto& dq = it->second;
            dq.erase(std::remove_if(dq.begin(), dq.end(),
                     [&](auto const& ex){ return ex.order_id == o.order_id; }),
                     dq.end());
            if (dq.empty())
                buyBook_.erase(it);
        }
    }
    else {
        // Annulation dans le book SELL
        auto it = sellBook_.find(o.price);
        if (it != sellBook_.end()) {
            auto& dq = it->second;
            dq.erase(std::remove_if(dq.begin(), dq.end(),
                     [&](auto const& ex){ return ex.order_id == o.order_id; }),
                     dq.end());
            if (dq.empty())
                sellBook_.erase(it);
        }
    }
}

std::vector<Execution> OrderBook::matchLimit(const Order& o) {
    std::vector<Execution> fills;
    uint64_t remaining = o.quantity;

    if (o.side == Side::BUY) {
        // Croise contre le SELL book (prix croissants)
        for (auto it = sellBook_.begin(); it != sellBook_.end() && remaining > 0; ) {
            if (o.price < it->first) break;
            auto& dq = it->second;
            while (!dq.empty() && remaining > 0) {
                Order resting = dq.front();
                uint64_t traded = std::min(remaining, resting.quantity);
                fills.push_back({ resting.order_id, o.order_id, traded, it->first });
                remaining -= traded;
                resting.quantity -= traded;
                if (resting.quantity == 0)
                    dq.pop_front();
                else
                    dq.front() = resting;
            }
            it = dq.empty() ? sellBook_.erase(it) : std::next(it);
        }
    }
    else {
        // Croise contre le BUY book (prix décroissants)
        for (auto it = buyBook_.begin(); it != buyBook_.end() && remaining > 0; ) {
            if (o.price > it->first) break;
            auto& dq = it->second;
            while (!dq.empty() && remaining > 0) {
                Order resting = dq.front();
                uint64_t traded = std::min(remaining, resting.quantity);
                fills.push_back({ resting.order_id, o.order_id, traded, it->first });
                remaining -= traded;
                resting.quantity -= traded;
                if (resting.quantity == 0)
                    dq.pop_front();
                else
                    dq.front() = resting;
            }
            it = dq.empty() ? buyBook_.erase(it) : std::next(it);
        }
    }

    // Réinsertion du reliquat comme order LIMIT
    if (remaining > 0) {
        Order residual = o;
        residual.quantity = remaining;
        addLimitOrder(residual);
    }

    return fills;
}

std::vector<Execution> OrderBook::matchMarket(const Order& o) {
    std::vector<Execution> fills;
    uint64_t remaining = o.quantity;

    if (o.side == Side::BUY) {
        // BUY market: croise contre sellBook_, sans réinsertion
        for (auto it = sellBook_.begin(); it != sellBook_.end() && remaining > 0; ) {
            auto& dq = it->second;
            while (!dq.empty() && remaining > 0) {
                Order resting = dq.front();
                uint64_t traded = std::min(remaining, resting.quantity);
                fills.push_back({ resting.order_id, o.order_id, traded, it->first });
                remaining -= traded;
                resting.quantity -= traded;
                if (resting.quantity == 0)
                    dq.pop_front();
                else
                    dq.front() = resting;
            }
            it = dq.empty() ? sellBook_.erase(it) : std::next(it);
        }
    }
    else {
        // SELL market: croise contre buyBook_, sans réinsertion
        for (auto it = buyBook_.begin(); it != buyBook_.end() && remaining > 0; ) {
            auto& dq = it->second;
            while (!dq.empty() && remaining > 0) {
                Order resting = dq.front();
                uint64_t traded = std::min(remaining, resting.quantity);
                fills.push_back({ resting.order_id, o.order_id, traded, it->first });
                remaining -= traded;
                resting.quantity -= traded;
                if (resting.quantity == 0)
                    dq.pop_front();
                else
                    dq.front() = resting;
            }
            it = dq.empty() ? buyBook_.erase(it) : std::next(it);
        }
    }

    return fills;
}

} // namespace me
