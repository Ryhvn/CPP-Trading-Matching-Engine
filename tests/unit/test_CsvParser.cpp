#include <gtest/gtest.h>
#include "CsvParser.h"

using namespace me;

// vérifie qu’un fichier CSV correct est bien parsé en un objet Order complet et sans aucune erreur.
TEST(CsvParser, ParsesValidLine) {
    CsvParser p("tests/data/input_valid.csv");
    auto o = p.next();
    ASSERT_TRUE(o.has_value());
    EXPECT_EQ(o->order_id, 1u);
    EXPECT_EQ(o->instrument, "AAPL");
    EXPECT_EQ(o->side, Side::BUY);
    EXPECT_EQ(o->type, Type::LIMIT);
    EXPECT_EQ(o->quantity, 100u);
    EXPECT_DOUBLE_EQ(o->price, 150.25);
    EXPECT_EQ(o->action, Action::NEW);
    EXPECT_TRUE(p.getErrors().empty());
}

// vérifie que le parser relève bien les erreurs de formatage dans le CSV, en particulier pour les lignes invalides.
TEST(CsvParser, SkipsHeaderAndReportsAllInvalidLines) {
    CsvParser p("tests/data/input_invalid.csv");

    // 2) 1ʳᵉ ligne invalide (bad timestamp)
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().back().message, "Timestamp invalide");

    // 3) 2ᵉ ligne invalide (bad order_id)
    o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 2u);
    EXPECT_EQ(p.getErrors().at(1).message, "order_id invalide");
}

// Ligne avec trop peu de colonnes
TEST(CsvParser, ReportsTooFewColumns) {
    // tests/data/input_few_columns.csv :
    // timestamp,order_id,instrument,side      <- seulement 4 colonnes
    // 1610000000,1,AAPL,BUY
    CsvParser p("tests/data/input_few_columns.csv");
    auto o = p.next();                // skip en-tête
    EXPECT_FALSE(o.has_value());
    o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().front().message, "Nombre de colonnes != 8");
}

// Instrument vide
TEST(CsvParser, ReportsEmptyInstrument) {
    // tests/data/input_empty_instrument.csv :
    // ... en-tête
    // 1610000000,1,,BUY,LIMIT,100,150.25,NEW
    CsvParser p("tests/data/input_empty_instrument.csv");
    p.next();                         // skip header
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().front().message, "Instrument vide");
}

// Side invalide
TEST(CsvParser, ReportsInvalidSide) {
    // line: 1610000000,1,AAPL,WRONG,LIMIT,100,150.25,NEW
    CsvParser p("tests/data/input_bad_side.csv");
    p.next();
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().front().message, "Side invalide");  // ou le message de sideFromString
}

// Type invalide
TEST(CsvParser, ReportsInvalidType) {
    // line: 1610000000,1,AAPL,BUY,WRONG,100,150.25,NEW
    CsvParser p("tests/data/input_bad_type.csv");
    p.next();
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().front().message, "Type invalide");  // ou le message de typeFromString
}

// Action invalide
TEST(CsvParser, ReportsInvalidAction) {
    // line: 1610000000,1,AAPL,BUY,LIMIT,100,150.25,WRONG
    CsvParser p("tests/data/input_bad_action.csv");
    p.next();
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    ASSERT_EQ(p.getErrors().size(), 1u);
    EXPECT_EQ(p.getErrors().front().message, "Action invalide"); // message de actionFromString
}

// Quantité négative et non numérique
TEST(CsvParser, ReportsAllQuantityAndPriceErrors) {
    CsvParser p("tests/data/input_all_errors.csv");
    p.next();  // skip header

    // On lit 4 lignes invalides successives
    for (int i = 0; i < 4; ++i) {
        auto o = p.next();
        EXPECT_FALSE(o.has_value());
    }

    // Vérification des 4 erreurs dans l'ordre
    const auto& errs = p.getErrors();
    ASSERT_EQ(errs.size(), 4u);
    EXPECT_EQ(errs.at(0).message, "Quantité négative");
    EXPECT_EQ(errs.at(1).message, "Quantité invalide");
    EXPECT_EQ(errs.at(2).message, "Prix négatif");
    EXPECT_EQ(errs.at(3).message, "Prix invalide");
}

// Plusieurs lignes valides successives
TEST(CsvParser, ParsesMultipleValidLines) {
    CsvParser p("tests/data/input_two_valid.csv");
    // tests/data/input_two_valid.csv :
    // header...
    // 1610000000,1,AAPL,BUY,LIMIT,100,150.25,NEW
    // 1610000100,2,GOOG,SELL,MARKET,50,,CANCEL

    auto o1 = p.next();
    ASSERT_TRUE(o1.has_value());
    EXPECT_EQ(o1->order_id, 1u);

    auto o2 = p.next();
    ASSERT_TRUE(o2.has_value());
    EXPECT_EQ(o2->order_id, 2u);
    EXPECT_EQ(o2->type, Type::MARKET);

    // plus de lignes
    EXPECT_FALSE(p.next().has_value());
    EXPECT_TRUE(p.getErrors().empty());
}

// Fichier vide ou sans ligne de données
TEST(CsvParser, EmptyFileOrOnlyHeader) {
    CsvParser p("tests/data/input_only_header.csv");
    auto o = p.next();
    EXPECT_FALSE(o.has_value());
    // il n’y a pas de deuxième ligne
    EXPECT_FALSE(p.next().has_value());
    EXPECT_TRUE(p.getErrors().empty());
}
