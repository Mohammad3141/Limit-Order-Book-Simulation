#include "OrderBook.h"
#include "Metrics.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

#if __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#endif

void print_depth(const std::vector<std::pair<int, int>> &bids, const std::vector<std::pair<int, int>> &asks)
{
    std::cout << "BIDS (Price/Qty)           | ASKS (Price/Qty)\n";
    std::cout << "-------------------------- | -------------------------\n";
    for (int i = 0; i < 10; ++i)
    {
        if (i < bids.size())
        {
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << bids[i].first / 100.0
                      << "/" << std::setw(12) << bids[i].second << " | ";
        }
        else
        {
            std::cout << std::string(26, ' ') << "| ";
        }
        if (i < asks.size())
        {
            std::cout << std::fixed << std::setprecision(2) << std::setw(10) << asks[i].first / 100.0
                      << "/" << std::setw(12) << asks[i].second;
        }
        std::cout << "\n";
    }
}

int main()
{
    // Updated file paths for your folder structure
    std::string order_filepath = "Data/nse_orders_data.csv";
    std::string trade_filepath = "Data/nse_trades_data.csv";
    std::string metrics_filepath = "Output/metrics_output.csv";

#if __cplusplus >= 201703L
    // Create the Output directory if it does not exist (C++17 only)
    if (!fs::exists("Output"))
        fs::create_directory("Output");
#endif

    std::vector<Event> all_events;
    std::string line;

    // Read order events
    std::ifstream order_file(order_filepath);
    if (!order_file.is_open())
    {
        std::cerr << "FATAL ERROR: Could not open order file at " << order_filepath << std::endl;
        return 1;
    }
    std::getline(order_file, line); // Skip header
    while (std::getline(order_file, line))
    {
        Event e = Utils::parse_line(line, false);
        if (e.timestamp > 0)
            all_events.push_back(e);
    }
    order_file.close();

    // Read trade events
    std::ifstream trade_file(trade_filepath);
    if (!trade_file.is_open())
    {
        std::cerr << "FATAL ERROR: Could not open trade file at " << trade_filepath << std::endl;
        return 1;
    }
    std::getline(trade_file, line); // Skip header
    while (std::getline(trade_file, line))
    {
        Event e = Utils::parse_line(line, true);
        if (e.timestamp > 0)
            all_events.push_back(e);
    }
    trade_file.close();

    std::cout << "Loaded " << all_events.size() << " total events. Sorting..." << std::endl;
    std::sort(all_events.begin(), all_events.end(), [](const Event &a, const Event &b)
              { return a.timestamp < b.timestamp; });

    std::cout << "Processing events and collecting metrics...\n";

    OrderBook book;
    std::ofstream metrics_out(metrics_filepath);
    metrics_out << "Timestamp,TimestampRaw,MidPrice,Spread,OFI_Top,OFI_Depth";
    for (int lvl = 1; lvl <= 5; ++lvl)
    {
        metrics_out << ",BidLvl" << lvl << ",AskLvl" << lvl;
    }
    metrics_out << "\n";

    const int SNAPSHOT_FREQ = 1000;
    for (size_t i = 0; i < all_events.size(); ++i)
    {
        const auto &event = all_events[i];
        book.process_event(event);

        LOBMetrics metrics = MetricsCalculator::calculate(book, event.timestamp, 5, 0.5);

        if (i % SNAPSHOT_FREQ == 0)
        {
            std::cout << "\n\n--- Event at " << metrics.timestamp_formatted << " ---\n";
            print_depth(book.get_bids_depth(10), book.get_asks_depth(10));
            std::cout << "\n--- Metrics ---\n";
            std::cout << std::fixed << std::setprecision(4)
                      << "Mid-Price: " << metrics.mid_price / 100.0
                      << " | Spread: " << metrics.spread / 100.0
                      << " | OFI_Top: " << metrics.ofi_top
                      << " | OFI_Depth: " << metrics.ofi_depth << "\n";
        }

        metrics_out << metrics.timestamp_formatted << ","
                    << metrics.timestamp_raw << ","
                    << metrics.mid_price << ","
                    << metrics.spread << ","
                    << metrics.ofi_top << ","
                    << metrics.ofi_depth;

        for (size_t lvl = 0; lvl < 5; ++lvl)
        {
            metrics_out << ",";
            metrics_out << ((lvl < metrics.depth_bids.size()) ? metrics.depth_bids[lvl] : 0);
            metrics_out << ",";
            metrics_out << ((lvl < metrics.depth_asks.size()) ? metrics.depth_asks[lvl] : 0);
        }
        metrics_out << "\n";

        if (i % SNAPSHOT_FREQ == 0)
        {
            book.take_snapshot(event.timestamp);
        }
    }

    metrics_out.close();
    std::cout << "\nSimulation finished. Metrics data saved to " << metrics_filepath << std::endl;
    std::cout << "Book snapshots retained (latest " << book.get_snapshots().size() << ")" << std::endl;

    return 0;
}
