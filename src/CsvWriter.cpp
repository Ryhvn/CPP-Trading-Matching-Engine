#include "CsvWriter.h"
#include "MatchResult.h"
#include <iomanip>

namespace me {

    CsvWriter::CsvWriter(const std::string& filename)
      : out_(filename, std::ios::trunc)
    {
        if (!out_.is_open())
            throw std::runtime_error("Impossible d'ouvrir « " + filename + " »");
        writeHeader();
    }

    void CsvWriter::writeHeader() {
        out_ << "timestamp,order_id,instrument,side,type,quantity,price,action";
        out_ << ",status,executed_quantity,execution_price,counterparty_id\n";
    }

    void CsvWriter::writeOrder(const Order& /*o_unused*/,
        const std::vector<MatchResult>& results)
    {
        for (auto const& r : results) {
            out_ << r.timestamp           << ','
                 << r.order_id            << ','
                 << r.instrument         << ','
                 << toString(r.side)     << ','
                 << toString(r.type)     << ','
                 << r.quantity           << ','
                 << r.price              << ','
                 << toString(r.action)   << ','
                 << toString(r.status)   << ','
                 << r.executed_quantity  << ','
                 << r.execution_price    << ','
                 << r.counterparty_id
                 << '\n';
        }
    }

} // namespace me
