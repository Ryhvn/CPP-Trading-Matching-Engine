#include "CsvParser.h"
#include "Logger.h"
#include <sstream>    // std::stringstream
#include <string>
#include <vector>
#include <stdexcept>
#include <utility>    // std::move
#include <optional>


namespace me {

CsvParser::CsvParser(std::string const& filename)
  : in_(filename), lineNumber_(0)
{
    if (!in_.is_open())
        throw std::runtime_error("Impossible d'ouvrir « " + filename + " »");

    // On saute l'en-tête (timestamp,order_id,…) pour que next() ne le parse jamais
    std::string header;
    if (std::getline(in_, header))
        ++lineNumber_;
}

std::vector<std::string> CsvParser::split(std::string const& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss{s};
    std::string item;
    while (std::getline(ss, item, delim))
        elems.push_back(std::move(item));
    return elems;
}

std::optional<Order> CsvParser::next() {
    std::string line;
    if (!std::getline(in_, line))
        return std::nullopt;

    ++lineNumber_;
    auto fields = split(line, ',');
    if (fields.size() != 8) {
        errors_.push_back({ lineNumber_, "Nombre de colonnes != 8", line });
        LOG_WARN("Nombre de colonnes invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 1) timestamp
    uint64_t ts;
    try {
        ts = std::stoull(fields.at(0));
    } catch (...) {
        errors_.push_back({ lineNumber_, "Timestamp invalide", line });
        LOG_WARN("Timestamp invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 2) order_id
    uint64_t id;
    try {
        id = std::stoull(fields.at(1));
    } catch (...) {
        errors_.push_back({ lineNumber_, "order_id invalide", line });
        LOG_WARN("order_id invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 3) instrument
    auto const& instr = fields.at(2);
    if (instr.empty()) {
        errors_.push_back({ lineNumber_, "Instrument vide", line });
        LOG_WARN("Instrument invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 4) side
    Side side;
    try {
         side = sideFromString(fields.at(3));
    } catch (...) {
        errors_.push_back({ lineNumber_, "Side invalide", line });
        LOG_WARN("Side invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 5) type
    Type type;
    try {
        type = typeFromString(fields.at(4));
    } catch (...) {
        errors_.push_back({ lineNumber_, "Type invalide", line });
        LOG_WARN("Type invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 8) action
    Action action;
    try {
        action = actionFromString(fields.at(7));
    } catch (...) {
        errors_.push_back({ lineNumber_, "Action invalide", line });
        LOG_WARN("Action invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 9) quantity
    auto const& qtyStr = fields.at(5);
    // si on avait un signe moins en tête, on rejette
    if (qtyStr.size() > 1 && qtyStr.front() == '-') {
        errors_.push_back({ lineNumber_, "Quantité négative", line });
        LOG_WARN("Quantité négative: \"" + line + "\"");
        return std::nullopt;
    }
    uint64_t qty;
    try {
        qty = std::stoull(qtyStr);
    } catch (...) {
        errors_.push_back({ lineNumber_, "Quantité invalide", line });
        LOG_WARN("Quantité invalide: \"" + line + "\"");
        return std::nullopt;
    }

    // 10) price
    double price = 0.0;
    if (type == Type::LIMIT) { // si c’est un ordre LIMIT, on attend un prix, pas pour un MARKET
        auto const& priceStr = fields.at(6);
        // négatif ?
        if (!priceStr.empty() && priceStr.front() == '-') {
            errors_.push_back({ lineNumber_, "Prix négatif", line });
            LOG_WARN("Prix négatif: \"" + line + "\"");
            return std::nullopt;
        }
        // conversion
        try {
            price = std::stod(priceStr);
        } catch (...) {
            errors_.push_back({ lineNumber_, "Prix invalide", line });
            LOG_WARN("Prix invalide: \"" + line + "\"");
            return std::nullopt;
        }
    }

    // 11) création de l’ordre (limit ou market)
    Order o = (type == Type::LIMIT)
      ? Order::makeLimit (ts, id, instr, side, qty, price, action)
      : Order::makeMarket(ts, id, instr, side, qty, action);

    return std::make_optional(std::move(o));
}

} // namespace me
