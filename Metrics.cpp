#include "Metrics.h"
#include "Utils.h"
#include <cmath>

// No structured bindings; support for C++11+
LOBMetrics MetricsCalculator::calculate(const OrderBook &book, uint64_t raw_timestamp, int depth_levels, double decay_lambda)
{
    LOBMetrics metrics;
    metrics.timestamp_raw = raw_timestamp;
    metrics.timestamp_formatted = Utils::format_timestamp_ist(raw_timestamp);

    std::pair<int, int> best = book.get_best_bid_ask();
    int best_bid = best.first;
    int best_ask = best.second;

    // --- Classic metrics ---
    if (best_bid > 0 && best_ask > 0)
    {
        metrics.spread = best_ask - best_bid;
        metrics.mid_price = (static_cast<double>(best_bid) + best_ask) / 2.0;
    }

    // --- Collect levels ---
    std::vector<std::pair<int, int>> bid_levels = book.get_bids_depth(depth_levels);
    std::vector<std::pair<int, int>> ask_levels = book.get_asks_depth(depth_levels);
    metrics.depth_bids.resize(depth_levels, 0.0);
    metrics.depth_asks.resize(depth_levels, 0.0);

    for (size_t i = 0; i < bid_levels.size(); ++i)
        metrics.depth_bids[i] = static_cast<double>(bid_levels[i].second);
    for (size_t i = 0; i < ask_levels.size(); ++i)
        metrics.depth_asks[i] = static_cast<double>(ask_levels[i].second);

    // --- Top-level OFI (classic) ---
    double vol_bid = (bid_levels.empty() ? 0.0 : metrics.depth_bids[0]);
    double vol_ask = (ask_levels.empty() ? 0.0 : metrics.depth_asks[0]);
    if (vol_bid + vol_ask > 0)
        metrics.ofi_top = (vol_bid - vol_ask) / (vol_bid + vol_ask);

    // --- Advanced OFI: Multi-level, exponential decay weighting ---
    double weight_sum = 0.0;
    double weighted_bid = 0.0, weighted_ask = 0.0;
    double decay = 1.0;
    for (int i = 0; i < depth_levels; ++i)
    {
        if (metrics.depth_bids[i] > 0)
            weighted_bid += metrics.depth_bids[i] * decay;
        if (metrics.depth_asks[i] > 0)
            weighted_ask += metrics.depth_asks[i] * decay;
        weight_sum += decay;
        decay *= (1.0 - decay_lambda); // exponential decay
    }
    if (weight_sum > 0)
        metrics.ofi_depth = (weighted_bid - weighted_ask) / weight_sum;

    return metrics;
}
