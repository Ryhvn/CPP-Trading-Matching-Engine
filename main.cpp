#include "CsvParser.h"
#include "MatchingEngine.h"
#include "CsvWriter.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    try {
        // 1) On construit le chemin vers data/ via la macro DATA_DIR
        fs::path dataDir   = fs::path(DATA_DIR);
        fs::path inputPath = dataDir / "input.csv";
        fs::path outputPath= dataDir / "output.csv";

        if (!fs::exists(inputPath)) {
            throw std::runtime_error("Le fichier « " + inputPath.string() + " » est introuvable.");
        }

        // 2) Initialisation des composants
        me::CsvParser      parser(inputPath.string());
        me::CsvWriter      writer(outputPath.string());
        me::MatchingEngine engine;

        // 3) Boucle principale de matching
        while (auto maybe = parser.next()) {
            const auto& order   = *maybe;
            // process() renvoie désormais un vecteur de MatchResult
            auto        results = engine.process(order);
            // on écrit directement ces MatchResult dans le CSV
            writer.writeOrder(order, results);
        }

        // 4) Affichage des éventuelles erreurs de parsing
        const auto& errs = parser.getErrors();
        if (!errs.empty()) {
            std::cerr << "\n=== Erreurs de parsing (" << errs.size() << ") ===\n";
            for (auto const& e : errs) {
                std::cerr
                  << "Ligne " << e.line_number
                  << " : "       << e.message << "\n"
                  << "  -> \""    << e.raw_line << "\"\n";
            }
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Erreur fatale : " << e.what() << "\n";
        return 1;
    }
}
