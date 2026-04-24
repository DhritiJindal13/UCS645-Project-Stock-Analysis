#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "fetcher.hpp"
#include "indicators.hpp"
#include "decision_logic.hpp"
#include "backtester.hpp"
#include "export_decisions.hpp"
// #include <cuda_runtime.h>

int main()
{
    // Configuration
    std::string api_key = "JJBVkPtrkQflCzlf";
    std::string symbol = "TSLA";
    std::string interval = "5min"; // daily interval
    bool use_gpu = false;          // Set to false since GPU functions are commented out

    std::cout << "Starting stock analysis program for " << symbol << std::endl;
    std::cout << "GPU acceleration: " << (use_gpu ? "enabled" : "disabled") << std::endl;

    // Fetch data
    StockData data;
    bool success = fetchStockData(symbol, interval, api_key, data, use_gpu);
    if (!success)
    {
        std::cerr << "Error: Failed to fetch data." << std::endl;
        return 1;
    }

    std::cout << "Successfully fetched " << data.dates.size() << " data points." << std::endl;

    if (data.dates.size() > 0)
    {
        std::cout << "Date range: " << data.dates.back() << " to " << data.dates.front() << std::endl;
    }

    // === Individual Timing for Each Indicator ===
    auto start = std::chrono::high_resolution_clock::now();
    auto sma = computeSMA(data.close, 20);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "SMA Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto ema = computeEMA(data.close, 20);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "EMA Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto rsi = computeRSI(data.close, 14);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "RSI Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto bb = computeBollingerBands(data.close, 20);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Bollinger Bands Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto macd = computeMACD(data.close);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "MACD Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto stoch = computeStochasticOscillator(data.close, data.high, data.low);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Stochastic Oscillator Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    auto atr = computeATR(data.high, data.low, data.close, 14);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "ATR Calculation Time: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    // Start GPU timing
//     float gpu_time_ms = 0;
//     cudaEvent_t start, stop;
//     cudaEventCreate(&start);
//     cudaEventCreate(&stop);
//     cudaEventRecord(start);

//     // GPU version of indicators (if use_gpu is true)
// #ifdef __CUDACC__
//     auto sma_gpu = computeSMA_GPU(data.close, 20);
//     auto ema_gpu = computeEMA_GPU(data.close, 20);
//     auto rsi_gpu = computeRSI_GPU(data.close, 14);
//     auto bb_gpu = computeBollingerBands_GPU(data.close, 20);
//     auto macd_gpu = computeMACD_GPU(data.close);
//     auto stoch_gpu = computeStochasticOscillator_GPU(data.close, data.high, data.low);
//     auto atr_gpu = computeATR_GPU(data.high, data.low, data.close, 14);
// #endif

//     // Stop GPU timing
//     cudaEventRecord(stop);
//     cudaEventSynchronize(stop);
//     cudaEventElapsedTime(&gpu_time_ms, start, stop);
//     std::cout << "\nTotal GPU calculation time: " << gpu_time_ms << " ms" << std::endl;

    exportIndicatorsToCSV(
        "indicators.csv",
        data.dates,
        data.close,
        sma,
        ema,
        rsi,
        bb,
        macd,
        stoch,
        atr,
        20, // SMA period
        20, // EMA period
        14, // RSI period
        20, // BB period
        26, // MACD slow
        14, // ATR period
        14  // Stochastic %K period
    );

    // Generate decisions
    std::vector<Decision> decisions = generateDecisions(
        data.dates,
        data.close,
        data.high,
        data.low,
        data.volume,
        use_gpu);

    // save decision to csv
    exportDecisionCSV("signals.csv", decisions, data.close);

    Backtester bt(decisions, data.close, data.dates);
    bt.run(); // Simulate trading
    bt.exportBacktestCSV("backtest.csv");
    bt.report(); // Print summary

    // Output decisions
    std::cout << "Date       | Signal\n";
    std::cout << "-----------|--------\n";

    if (decisions.empty())
    {
        std::cout << "No trading signals generated." << std::endl;
    }
    else
    {
        // Print the most recent 10 decisions (or all if less than 10)
        size_t start_idx = (decisions.size() > 10) ? decisions.size() - 10 : 0;
        for (size_t i = start_idx; i < decisions.size(); ++i)
        {
            std::cout << decisions[i].date << " | " << signalToString(decisions[i].signal) << "\n";
        }

        // Count signals by type
        int buy_count = 0, sell_count = 0, hold_count = 0;
        for (const auto &d : decisions)
        {
            switch (d.signal)
            {
            case Signal::BUY:
                buy_count++;
                break;
            case Signal::SELL:
                sell_count++;
                break;
            case Signal::HOLD:
                hold_count++;
                break;
            }
        }

        std::cout << "\nSummary: " << decisions.size() << " total signals\n";
        std::cout << "BUY: " << buy_count << ", SELL: " << sell_count << ", HOLD: " << hold_count << std::endl;
    }

    return 0;
}