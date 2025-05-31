#pragma once

#include "Order.h"
#include "MatchResult.h"
#include <map>
#include <deque>
#include <vector>
#include <algorithm>
#include <functional>

namespace me {

    // OrderBook pour un seul instrument
    template<typename Cmp = std::less<double>>
    using PriceLevel = std::map<double, std::deque<Order>, Cmp>;

    class OrderBook {
    public:
        // Traite un ordre (NEW/MODIFY/CANCEL) et renvoie tous les fills générés
        std::vector<Execution> process(const Order& o);
        [[nodiscard]] bool empty() const {
            return buyBook_.empty() && sellBook_.empty();
        }

    private:
        PriceLevel<std::greater<>> buyBook_;   // BUY : prix décroissants
        PriceLevel<>               sellBook_;  // SELL: prix croissants

        // Helpers
        std::vector<Execution> matchLimit(const Order& o);
        std::vector<Execution> matchMarket(const Order& o);
        void addLimitOrder(const Order& o);
        void cancelOrder(const Order& o);
    };

} // namespace me
