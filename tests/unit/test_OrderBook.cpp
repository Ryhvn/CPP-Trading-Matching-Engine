#include <gtest/gtest.h>
#include "OrderBook.h"

using namespace me;

// test pour vérifier qu'un ordre LIMIT SELL est correctement apparié avec un ordre LIMIT BUY au même prix
TEST(OrderBook, SimpleLimitMatch) {
    OrderBook book;
    // ordre SELL au prix 100 qty 50
    Order sell = Order::makeLimit(1, 1, "XYZ", Side::SELL, 50, 100.0, Action::NEW);
    book.process(sell);
    // ordre BUY au même prix qty 30
    Order buy = Order::makeLimit(2, 2, "XYZ", Side::BUY, 30, 100.0, Action::NEW);
    auto fills = book.process(buy);
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).resting_order_id,   1u);
    EXPECT_EQ(fills.at(0).incoming_order_id,  2u);
    EXPECT_EQ(fills.at(0).executed_quantity, 30u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
}

// test pour vérifier qu'un ordre LIMIT BUY est correctement apparié avec un ordre LIMIT SELL au même prix
TEST(OrderBook, PartialAndResidual) {
    OrderBook book;
    book.process(Order::makeLimit(1,1,"ABC",Side::SELL, 50, 10.0, Action::NEW));
    auto fills = book.process(Order::makeLimit(2,2,"ABC",Side::BUY, 100, 10.0, Action::NEW));
    // on devrait avoir un fill de 50
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 50u);
    // et 50 unités résiduelles dans le book (implicitement testable via un nouveau SELL)
}

// Aucun match lorsque le prix BUY est inférieur au meilleur ASK
TEST(OrderBook, NoMatchWhenPriceNotMet) {
    OrderBook book;
    // ordre SELL passif : 50@100
    book.process(Order::makeLimit(1, 1, "XYZ", Side::SELL, 50, 100.0, Action::NEW));
    // ordre BUY agressif à 99 (en dessous) : pas de match
    auto fills = book.process(Order::makeLimit(2, 2, "XYZ", Side::BUY, 50,  99.0, Action::NEW));
    EXPECT_TRUE(fills.empty());
}

// Matching au prix du carnet même si BUY a un prix supérieur
TEST(OrderBook, BuyAboveAskMatchesAtAskPrice) {
    OrderBook book;
    // ordre SELL passif : 30@100
    book.process(Order::makeLimit(1, 1, "XYZ", Side::SELL, 30, 100.0, Action::NEW));
    // ordre BUY agressif : 20@110 → match 20@100
    auto fills = book.process(Order::makeLimit(2, 2, "XYZ", Side::BUY, 20, 110.0, Action::NEW));
    ASSERT_EQ(fills.size(), 1u);
    EXPECT_EQ(fills.at(0).resting_order_id,  1u);
    EXPECT_EQ(fills.at(0).incoming_order_id, 2u);
    EXPECT_EQ(fills.at(0).executed_quantity, 20u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
}

// Remplissage réparti sur plusieurs ordres passifs
TEST(OrderBook, MultipleFillsFromMultipleRestingOrders) {
    OrderBook book;
    // deux ordres SELL passifs : 10@100 (ID=1), 20@101 (ID=2)
    book.process(Order::makeLimit(1, 1, "XYZ", Side::SELL, 10, 100.0, Action::NEW));
    book.process(Order::makeLimit(2, 2, "XYZ", Side::SELL, 20, 101.0, Action::NEW));
    // ordre BUY agressif : 25@105 → doit matcher 10@100 puis 15@101
    auto fills = book.process(Order::makeLimit(3, 3, "XYZ", Side::BUY, 25, 105.0, Action::NEW));
    ASSERT_EQ(fills.size(), 2u);
    // premier fill sur ID=1
    EXPECT_EQ(fills.at(0).resting_order_id,   1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 10u);
    EXPECT_DOUBLE_EQ(fills.at(0).execution_price, 100.0);
    // second fill sur ID=2
    EXPECT_EQ(fills.at(1).resting_order_id,   2u);
    EXPECT_EQ(fills.at(1).executed_quantity, 15u);
    EXPECT_DOUBLE_EQ(fills.at(1).execution_price, 101.0);
}

// Respect de la priorité FIFO pour le même prix
TEST(OrderBook, FIFOAtSamePrice) {
    OrderBook book;
    // deux ordres SELL passifs au même prix 100 : ID=1 arrive avant ID=2
    book.process(Order::makeLimit(1, 1, "XYZ", Side::SELL, 10, 100.0, Action::NEW));
    book.process(Order::makeLimit(2, 2, "XYZ", Side::SELL, 10, 100.0, Action::NEW));
    // ordre BUY agressif : 15@100 → remplit 10@ID=1 puis 5@ID=2
    auto fills = book.process(Order::makeLimit(3, 3, "XYZ", Side::BUY, 15, 100.0, Action::NEW));
    ASSERT_EQ(fills.size(), 2u);
    EXPECT_EQ(fills.at(0).resting_order_id, 1u);
    EXPECT_EQ(fills.at(0).executed_quantity, 10u);
    EXPECT_EQ(fills.at(1).resting_order_id, 2u);
    EXPECT_EQ(fills.at(1).executed_quantity, 5u);
}

// Vérification de la persistance de la quantité résiduelle après un match partiel
TEST(OrderBook, ResidualOrderRemainsAfterPartialFill) {
    OrderBook book;
    // ordre SELL passif : 50@100
    book.process(Order::makeLimit(1, 1, "XYZ", Side::SELL, 50, 100.0, Action::NEW));
    // ordre BUY agressif : 20@100 → match 20, reste 30 en carnet
    auto fills1 = book.process(Order::makeLimit(2, 2, "XYZ", Side::BUY, 20, 100.0, Action::NEW));
    ASSERT_EQ(fills1.size(), 1u);
    EXPECT_EQ(fills1.at(0).executed_quantity, 20u);
    // un nouvel ordre BUY agressif pour 30@100 doit vider la quantité résiduelle
    auto fills2 = book.process(Order::makeLimit(3, 3, "XYZ", Side::BUY, 30, 100.0, Action::NEW));
    ASSERT_EQ(fills2.size(), 1u);
    EXPECT_EQ(fills2.at(0).executed_quantity, 30u);
    EXPECT_DOUBLE_EQ(fills2.at(0).execution_price, 100.0);
}