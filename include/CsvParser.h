#pragma once

#include "Order.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <optional>

namespace me {

    struct ParseError {
        size_t      line_number;
        std::string message;
        std::string raw_line;
    };

    class CsvParser {
    public:
        explicit CsvParser(std::string const& filename);
        std::optional<Order> next();
        std::vector<ParseError> const& getErrors() const { return errors_; }

    private:
        std::ifstream            in_;          // le flux
        size_t                   lineNumber_;  // index de la ligne courante
        std::vector<ParseError>  errors_;      // accumulateur d’erreurs

        // helper pour découper la ligne en champs
        static std::vector<std::string> split(std::string const& s, char delim);
    };

} // namespace me
