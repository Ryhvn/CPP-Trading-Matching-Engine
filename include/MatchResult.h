#pragma once

#include "Order.h"
#include <cstdint>
#include <string>

namespace me {

    // --- Résultat de crossing produit par l’OrderBook ---
    struct Execution {
        uint64_t resting_order_id;   // ID de l’ordre au book
        uint64_t incoming_order_id;  // ID de l’ordre entrant
        uint64_t executed_quantity;  // quantité appariée
        double   execution_price;    // prix d’exécution
    };

    // Statut à écrire en sortie CSV
    enum class Status {
        PENDING,
        EXECUTED,
        PARTIALLY_EXECUTED,
        CANCELED,
        REJECTED
    };

    inline std::string toString(Status s) {
        switch (s) {
            case Status::PENDING:            return "PENDING";
            case Status::EXECUTED:           return "EXECUTED";
            case Status::PARTIALLY_EXECUTED: return "PARTIALLY_EXECUTED";
            case Status::CANCELED:           return "CANCELED";
            case Status::REJECTED:           return "REJECTED";
        }
        return "";
    }

    // Ce qu’on renvoie au CsvWriter
    struct MatchResult {
        uint64_t    timestamp;          // timestamp de l’action
        uint64_t    order_id;           // id de l’ordre entrant
        std::string instrument;
        Side        side;
        Type        type;
        uint64_t    quantity;           // quantité restante
        double      price;
        Action      action;
        Status      status;             // statut calculé
        uint64_t    executed_quantity;  // qty exécutée sur cet event
        double      execution_price;    // prix d’exécution
        uint64_t    counterparty_id;    // resting_order_id
    };

} // namespace me
