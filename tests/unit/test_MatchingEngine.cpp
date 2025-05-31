#include <gtest/gtest.h>
#include "MatchingEngine.h"
#include "CsvParser.h"

using namespace me;

// Test pour s'assurer que deux ordres LIMIT de même instrument sont correctement appariés
TEST(MatchingEngine, TwoInstrumentsSeparated) {
    MatchingEngine eng;
    Order o1 = Order::makeLimit(1,1,"AAPL",Side::BUY, 10, 100.0, Action::NEW);
    Order o2 = Order::makeLimit(2,2,"GOOG",Side::SELL, 5, 1500.0, Action::NEW);

    auto f1 = eng.process(o1);
    auto f2 = eng.process(o2);

    // Chaque ordre non exécuté immédiatement génère un seul résultat PENDING
    ASSERT_EQ(f1.size(), 1u);
    EXPECT_EQ(f1.at(0).status, Status::PENDING);

    ASSERT_EQ(f2.size(), 1u);
    EXPECT_EQ(f2.at(0).status, Status::PENDING);
}

// Test qu'un ordre LIMIT inséré sans contrepartie ne génère pas de fill
TEST(MatchingEngine, FirstInsertionNoFill) {
    MatchingEngine eng;
    auto f = eng.process(
      Order::makeLimit(1,1,"AAPL",Side::BUY, 10,100.0,Action::NEW)
    );
    // On s'attend à recevoir un seul résultat PENDING
    ASSERT_EQ(f.size(), 1u);
    EXPECT_EQ(f.at(0).status, Status::PENDING);
}

// Test d’exécution d’un ordre limite agressif contre un ordre limite passif
TEST(MatchingEngine, SimpleLimitMatch) {
    MatchingEngine eng;
    // 1) Ordre SELL passif : 5@100
    eng.process(Order::makeLimit(1, 1, "AAPL", Side::SELL, 5, 100.0, Action::NEW));
    // 2) Ordre BUY agressif : 5@105 → doit matcher 5@100
    auto fills = eng.process(Order::makeLimit(2, 2, "AAPL", Side::BUY, 5, 105.0, Action::NEW));
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 5u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
    EXPECT_EQ(fills.at(0).status, Status::EXECUTED);
}

// Test de remplissage partiel réparti sur plusieurs ordres passifs
TEST(MatchingEngine, PartialFillAcrossMultipleOrders) {
    MatchingEngine eng;
    // Deux ordres SELL passifs : 5@100 puis 5@101
    eng.process(Order::makeLimit(1, 1, "AAPL", Side::SELL, 5, 100.0, Action::NEW));
    eng.process(Order::makeLimit(2, 2, "AAPL", Side::SELL, 5, 101.0, Action::NEW));
    // Ordre BUY agressif pour 12@101 → remplit 5@100 et 5@101, reste 2
    auto fills = eng.process(Order::makeLimit(3, 3, "AAPL", Side::BUY, 12, 101.0, Action::NEW));
    ASSERT_EQ(fills.size(), 2u);
    EXPECT_EQ(fills.at(0).executed_quantity, 5u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
    EXPECT_EQ(fills.at(1).executed_quantity, 5u);
    EXPECT_DOUBLE_EQ(fills.at(1).execution_price, 101.0);
    // Chaque fill est PARTIALLY_EXECUTED
    EXPECT_EQ(fills.at(0).status, Status::PARTIALLY_EXECUTED);
    EXPECT_EQ(fills.at(1).status, Status::PARTIALLY_EXECUTED);
}

// Test d’exécution d’un ordre Market contre le meilleur prix disponible
TEST(MatchingEngine, MarketOrderExecution) {
    MatchingEngine eng;
    // Ordre SELL passif à 200 pour 10
    eng.process(Order::makeLimit(1, 1, "GOOG", Side::SELL, 10, 200.0, Action::NEW));
    // Ordre Market BUY pour 8 → doit exécuter 8@200
    auto fills = eng.process(Order::makeMarket(2, 2, "GOOG", Side::BUY, 8, Action::NEW));
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 8u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 200.0);
    EXPECT_EQ(fills.at(0).status, Status::EXECUTED);
}

// Test de modification d’un ordre et impact sur le matching
TEST(MatchingEngine, ModifyOrderAffectsMatching) {
    MatchingEngine eng;
    // Ordre BUY passif initial : 10@100
    eng.process(Order::makeLimit(1, 1, "MSFT", Side::BUY, 10, 100.0, Action::NEW));
    // Modification à 5 → remainingQty devient 5
    eng.process(Order::makeLimit(2, 1, "MSFT", Side::BUY, 5, 100.0, Action::MODIFY));
    // Ordre SELL passif pour 8@100 → doit remplir seulement 5
    auto fills = eng.process(Order::makeLimit(3, 3, "MSFT", Side::SELL, 8, 100.0, Action::NEW));
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 5u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
    EXPECT_EQ(fills.at(0).status, Status::PARTIALLY_EXECUTED);
}

// Test d’annulation d’un ordre empêchant tout futur matching
TEST(MatchingEngine, CancelOrderPreventsMatching) {
    MatchingEngine eng;
    // Ordre BUY passif : 10@50
    eng.process(Order::makeLimit(1, 1, "TSLA", Side::BUY, 10, 50.0, Action::NEW));
    // Annulation de l’ordre
    eng.process(Order::makeLimit(2, 1, "TSLA", Side::BUY, 0, 50.0, Action::CANCEL));
    // Ordre SELL agressif pour 5@50 → ne doit pas matcher
    auto fills = eng.process(Order::makeLimit(3, 3, "TSLA", Side::SELL, 5, 50.0, Action::NEW));
    // Pas de fills → un résultat PENDING pour le SELL
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).status, Status::PENDING);
}