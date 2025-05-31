#include <gtest/gtest.h>
#include <chrono>
#include "MatchingEngine.h"
#include "Order.h"
#include "Logger.h"

TEST(Performance, Throughput) {
    constexpr int N = 200000;
    // On désactive les logs pour ce test de perf
    me::setLoggingEnabled(false);

    me::MatchingEngine eng;
    std::vector<me::Order> orders;
    orders.reserve(N);
    for (int i = 0; i < N; ++i) {
        orders.push_back(
          me::Order::makeLimit(i, i, "AAPL", me::Side::BUY, 1, 100.0, me::Action::NEW)
        );
    }
    auto t0 = std::chrono::high_resolution_clock::now();
    for (auto const& o : orders) eng.process(o);
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();
    // Seuil réaliste sans logs : on passe à 0.5 s
    EXPECT_LT(secs, 0.5)
    << "200 000 ordres en " << secs << " s (trop lent)";
}
