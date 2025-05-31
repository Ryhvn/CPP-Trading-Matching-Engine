#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <string>
#include "CsvWriter.h"
#include "MatchResult.h"

using namespace me;

TEST(CsvWriter, WritesHeaderAndResult) {
    const std::string out = "tests/data/tmp_out.csv";
    {
        CsvWriter w(out);
        MatchResult r {
            1, 1, "XYZ", Side::BUY, Type::LIMIT,
            20, 50.0, Action::NEW, Status::PENDING,
            0,  0.0, 0
          };
        w.writeOrder(/* order */{/*…*/}, std::vector<MatchResult>{r});
    }
    std::ifstream in(out);
    std::string line;
    std::getline(in, line);
    EXPECT_EQ(line, "timestamp,order_id,instrument,side,type,quantity,price,action,status,executed_quantity,execution_price,counterparty_id");
    std::getline(in, line);
    // on vérifie qu’elle contient bien tous les champs de r
    EXPECT_NE(line.find("XYZ"), std::string::npos);
}

// Helper pour lire tout le fichier en vecteur de lignes (sans assertions)
static std::vector<std::string> readAllLines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);
    return lines;
}

// 1) Écriture de plusieurs résultats pour un même ordre
TEST(CsvWriter, WritesMultipleResultsForOneOrder) {
    const std::string out = "tests/data/tmp_multi.csv";
    {
        CsvWriter w(out);
        MatchResult r1{1, 1, "XYZ", Side::BUY,  Type::LIMIT, 10, 50.0, Action::NEW,    Status::EXECUTED,             10, 50.0, 2};
        MatchResult r2{1, 1, "XYZ", Side::BUY,  Type::LIMIT, 10, 50.0, Action::NEW,    Status::PARTIALLY_EXECUTED, 5,  50.0, 3};
        w.writeOrder(/* order */{/*…*/}, {r1, r2});
    }
    // Vérifie que le fichier a bien été créé
    std::ifstream in(out);
    ASSERT_TRUE(in.is_open());
    in.close();

    auto lines = readAllLines(out);
    // 1ère ligne = header
    ASSERT_GT(lines.size(), 2u);
    EXPECT_EQ(lines.at(0),
        "timestamp,order_id,instrument,side,type,quantity,price,action,"
        "status,executed_quantity,execution_price,counterparty_id");
    // Vérifier qu’on a bien deux lignes de résultats
    EXPECT_NE(lines.at(1).find("EXECUTED"), std::string::npos);
    EXPECT_NE(lines.at(2).find("PARTIALLY_EXECUTED"), std::string::npos);
}

// 2) Pas de résultat → seul l’en-tête
TEST(CsvWriter, WritesOnlyHeaderWhenNoResults) {
    const std::string out = "tests/data/tmp_empty.csv";
    {
        CsvWriter w(out);
        w.writeOrder(/* order */{/*…*/}, {});  // vecteur vide
    }
    std::ifstream in(out);
    ASSERT_TRUE(in.is_open());
    in.close();

    auto lines = readAllLines(out);
    // Seule l’en-tête doit être présente
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines.at(0),
        "timestamp,order_id,instrument,side,type,quantity,price,action,"
        "status,executed_quantity,execution_price,counterparty_id");
}

// 3) Écriture de plusieurs ordres (appel writeOrder plusieurs fois)
TEST(CsvWriter, AppendsResultsAcrossMultipleOrders) {
    const std::string out = "tests/data/tmp_append.csv";
    {
        CsvWriter w(out);
        MatchResult r1{1, 1, "AAA", Side::BUY,  Type::LIMIT, 5,  10.0, Action::NEW,    Status::PENDING, 0, 0.0, 0};
        MatchResult r2{2, 2, "BBB", Side::SELL, Type::LIMIT, 8,  20.5, Action::NEW,    Status::EXECUTED, 8, 20.5, 1};
        w.writeOrder(/* order1 */{/*…*/}, {r1});
        w.writeOrder(/* order2 */{/*…*/}, {r2});
    }
    std::ifstream in(out);
    ASSERT_TRUE(in.is_open());
    in.close();

    auto lines = readAllLines(out);
    // header + 2 lignes
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_NE(lines.at(1).find("AAA"), std::string::npos);
    EXPECT_NE(lines.at(2).find("BBB"), std::string::npos);
}

// 4) Vérifier le format numérique (double avec point décimal)
TEST(CsvWriter, FormatsDoublesCorrectly) {
    const std::string out = "tests/data/tmp_format.csv";
    {
        CsvWriter w(out);
        MatchResult r{3, 3, "CCC", Side::BUY, Type::LIMIT, 1,  0.1234, Action::NEW, Status::EXECUTED, 1, 0.1234, 4};
        w.writeOrder(/* order */{/*…*/}, {r});
    }
    std::ifstream in(out);
    ASSERT_TRUE(in.is_open());
    in.close();

    auto lines = readAllLines(out);
    // La ligne de résultat doit contenir "0.1234" (pas de virgule, pas d'exposant)
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_NE(lines.at(1).find("0.1234"), std::string::npos);
}

// 5) Statut et contrepartie pour CANCEL et REJECTED
TEST(CsvWriter, WritesCorrectStatusForCancelOrRejected) {
    const std::string out = "tests/data/tmp_status.csv";
    {
        CsvWriter w(out);
        MatchResult rC{4, 4, "DDD", Side::SELL, Type::LIMIT, 2,  30.0, Action::CANCEL, Status::CANCELED, 0, 0.0, 0};
        MatchResult rR{5, 5, "EEE", Side::BUY,  Type::LIMIT, 3,  40.0, Action::NEW,    Status::REJECTED, 0, 0.0, 0};
        w.writeOrder(/* orderC */{/*…*/}, {rC, rR});
    }
    std::ifstream in(out);
    ASSERT_TRUE(in.is_open());
    in.close();

    auto lines = readAllLines(out);
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_NE(lines.at(1).find("CANCELED"), std::string::npos);
    EXPECT_NE(lines.at(2).find("REJECTED"), std::string::npos);
}