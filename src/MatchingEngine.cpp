#include "MatchingEngine.h"
#include "Logger.h"
#include <stdexcept>

namespace me {

std::vector<MatchResult> MatchingEngine::process(const Order& o) {
    // Log de l'ordre reçu
    LOG_INFO("→ process Order{"
             "id=" + std::to_string(o.order_id) +
             ", instr=" + o.instrument +
             ", side=" + toString(o.side) +
             ", type=" + toString(o.type) +
             ", qty=" + std::to_string(o.quantity) +
             ", price=" + std::to_string(o.price) +
             ", action=" + toString(o.action) +
             "}");

    // 1) bookkeeping des quantités
    if (o.action == Action::NEW) {
        originalQty_[o.order_id]  = o.quantity;
        remainingQty_[o.order_id] = o.quantity;
    }

    else if (o.action == Action::MODIFY) {
        auto it = originalQty_.find(o.order_id);
        if (it == originalQty_.end())
            throw std::runtime_error("MODIFY sur ordre inconnu: " + std::to_string(o.order_id));
        uint64_t prevRem = remainingQty_[o.order_id];

        // recalcul du remaining selon la coquille signalée
        int64_t  deltaOriginal = static_cast<int64_t>(o.quantity) - static_cast<int64_t>(it->second);
        int64_t  newRem = static_cast<int64_t>(prevRem) + deltaOriginal;
        remainingQty_[o.order_id] = newRem < 0 ? 0 : static_cast<uint64_t>(newRem);
        // on ne change pas originalQty_ : c'est la quantité d'origine
    }
    else if (o.action == Action::CANCEL) {
        remainingQty_[o.order_id] = 0;
    }

    // 2) délégation au carnet
    auto& book = books_[o.instrument];
    auto  fills = book.process(o);

    std::vector<MatchResult> results;

    // 3) pas d’execution => PENDING ou CANCELED
    if (fills.empty()) {
        Status st = (o.action == Action::CANCEL) ? Status::CANCELED : Status::PENDING;
        results.push_back({
            o.timestamp,
            o.order_id,
            o.instrument,
            o.side,
            o.type,
            remainingQty_[o.order_id],
            o.price,
            o.action,
            st,
            0,    // executed_quantity
            0.0,  // execution_price
            0     // counterparty_id
        });
        return results;
    }

    // 4) pour chaque crossing, on met à jour remaining et on renvoie un MatchResult
    for (auto const& f : fills) {
        remainingQty_[o.order_id] -= f.executed_quantity;
        Status st = (remainingQty_[o.order_id] == 0)
                    ? Status::EXECUTED
                    : Status::PARTIALLY_EXECUTED;
        results.push_back({
            o.timestamp,
            o.order_id,
            o.instrument,
            o.side,
            o.type,
            remainingQty_[o.order_id],
            o.price,
            o.action,
            st,
            f.executed_quantity,
            f.execution_price,
            f.resting_order_id
        });
        LOG_INFO(
          "Matching Result order="   + std::to_string(o.order_id)
        + " counterparty="            + std::to_string(f.resting_order_id)
        + " qty_exe="             + std::to_string(f.executed_quantity)
        + " price="               + std::to_string(f.execution_price)
        + " status="              + toString(st)
        );
    }

    return results;
}

} // namespace me
