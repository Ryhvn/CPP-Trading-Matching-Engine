#pragma once

#include "OrderBook.h"
#include "Order.h"
#include "MatchResult.h"
#include <vector>
#include <unordered_map>

namespace me {

    class MatchingEngine {
    public:
        // traite un ordre et renvoie une liste de MatchResult
        std::vector<MatchResult> process(const Order& o);

    private:
        // un carnet par instrument
        std::unordered_map<std::string, OrderBook> books_;

        // Pour chaque ordre ID : quantité originale (pour MODIFY) et restante
        std::unordered_map<uint64_t, uint64_t> originalQty_;
        std::unordered_map<uint64_t, uint64_t> remainingQty_;
    };

} // namespace me
