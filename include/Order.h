// include/Order.h
#pragma once

#include <string>
#include <cstdint>
#include <ostream>

namespace me {

    // Les 3 enums
    enum class Side   { BUY, SELL };
    enum class Type   { LIMIT, MARKET };
    enum class Action { NEW, MODIFY, CANCEL };

    // Conversions enum ⇄ string
    std::string toString(Side);
    std::string toString(Type);
    std::string toString(Action);

    Side   sideFromString(const std::string&);
    Type   typeFromString(const std::string&);
    Action actionFromString(const std::string&);

    // Représentation d’un ordre
    struct Order {
        uint64_t   timestamp;
        uint64_t   order_id;
        std::string instrument;
        Side       side;
        Type       type;
        uint64_t   quantity;
        double     price;
        Action     action;

        // Affichage / debug
        [[nodiscard]] std::string toString() const;

        // Usines
        static Order makeLimit( uint64_t ts, uint64_t id,
                                std::string instr,
                                Side s, uint64_t qty, double p,
                                Action a );
        static Order makeMarket(uint64_t ts, uint64_t id,
                                std::string instr,
                                Side s, uint64_t qty,
                                Action a );

        // Vérifications internes
        void validate() const;
    };

    // Pour faire « cout << order ».
    std::ostream& operator<<(std::ostream& os, Order const& o);

} // namespace me
