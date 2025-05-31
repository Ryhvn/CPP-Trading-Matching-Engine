#pragma once

#include "Order.h"
#include "MatchResult.h"
#include <vector>
#include <fstream>
#include <stdexcept>

namespace me {

    class CsvWriter {
    public:
        explicit CsvWriter(const std::string& filename);
        // Écrit un ordre avec ses résultats d'exécution
        void writeOrder(const Order& o,
            const std::vector<MatchResult>& results);

    private:
        std::ofstream out_;
        void writeHeader();
    };

} // namespace me
