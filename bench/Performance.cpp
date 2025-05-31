#include <iostream>
#include <chrono>
#include <random>
#include "MatchingEngine.h"
#include "Order.h"
#include "Logger.h"

int main() {
    // ← ici on désactive tous les LOG_INFO / LOG_WARN / LOG_ERROR
    me::setLoggingEnabled(false);

    constexpr size_t N = 500000;
    me::MatchingEngine eng;

    // 1) Génération hors chrono
    std::mt19937_64 rng{42};
    std::uniform_int_distribution<uint64_t> qty{1,100};
    std::uniform_real_distribution<double> price{10.0,500.0};

    std::vector<me::Order> orders;
    orders.reserve(N);
    for (size_t i = 0; i < N; ++i) {
        orders.emplace_back(
          me::Order::makeLimit(
            i, i,
            "SYM" + std::to_string(i % 10),
            (i%2 ? me::Side::BUY : me::Side::SELL),
            qty(rng), price(rng),
            me::Action::NEW
          )
        );
    }

    // 2) Warm-up (pour peupler les carnets)
    for (auto const&  o : orders) {
        eng.process(o);
    }

    // 3) Mesure pure matching
    auto t0 = std::chrono::high_resolution_clock::now();
    for (auto const& o : orders) {
        eng.process(o);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "Processed " << N << " orders in "
              << secs << " s → " << (N/secs) << " ops/s\n";
    return 0;
}
