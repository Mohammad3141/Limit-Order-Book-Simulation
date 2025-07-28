#ifndef METRICS_H
#define METRICS_H

#include "OrderBook.h"
#include <vector>
#include <string>

// Metrics struct with human-readable and raw timestamp
struct LOBMetrics
{
    uint64_t timestamp_raw = 0;
    std::string timestamp_formatted;
    double mid_price = 0.0;
    int spread = 0;
    double ofi_top = 0.0;
    double ofi_depth = 0.0;
    std::vector<double> depth_bids;
    std::vector<double> depth_asks;
};

class MetricsCalculator
{
public:
    static LOBMetrics calculate(const OrderBook &book, uint64_t raw_timestamp, int depth_levels = 5, double decay_lambda = 0.5);
};

#endif // METRICS_H
