#ifndef BACKTESTER_HPP
#define BACKTESTER_HPP

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "decision_logic.hpp"  // for Decision and Signal

class Backtester {
public:
    Backtester(const std::vector<Decision>& decisions, const std::vector<float>& prices, const std::vector<std::string>& dates)
        : decisions(decisions), prices(prices), dates(dates) {}

    void run(float initialCapital = 10000.0f) {
        if (decisions.empty() || prices.empty()) {
            std::cerr << "No decisions or prices available for backtesting!" << std::endl;
            return;
        }

        cash = initialCapital;
        shares = 0;
        portfolioValues.clear();

        for (size_t i = 0; i < decisions.size(); ++i) {
            float price = prices[i];
            const Decision& decision = decisions[i];

            switch (decision.signal) {
                case Signal::BUY:
                    if (cash > 0) {
                        shares = cash / price;
                        cash = 0;
                        buyCount++;
                        tradeLog.push_back({decision.date, "BUY", price});
                    }
                    break;
                case Signal::SELL:
                    if (shares > 0) {
                        cash = shares * price;
                        shares = 0;
                        sellCount++;
                        tradeLog.push_back({decision.date, "SELL", price});
                    }
                    break;
                case Signal::HOLD:
                    break;
            }

            float totalValue = cash + shares * price;
            portfolioValues.push_back(totalValue);
        }

        // Final equity value
        finalValue = cash + shares * prices.back();
    }

    void report() const {
        std::cout << "\n======= Backtest Report =======\n";
        std::cout << "Initial capital : $10,000.00\n";
        std::cout << "Final equity    : $" << std::fixed << std::setprecision(2) << finalValue << "\n";
        std::cout << "Total trades    : " << buyCount + sellCount << " (BUYs: " << buyCount << ", SELLs: " << sellCount << ")\n";
        std::cout << "Return          : " << ((finalValue - 10000.0f) / 10000.0f) * 100.0f << "%\n";
        std::cout << "================================\n";

        if (!tradeLog.empty()) {
            std::cout << "\nTrade Log:\n";
            for (const auto& log : tradeLog) {
                std::cout << log.date << " | " << log.action << " @ $" << std::fixed << std::setprecision(2) << log.price << "\n";
            }
        }
    }

    void exportBacktestCSV(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << filename << " for writing." << std::endl;
            return;
        }
    
        file << "date,portfolio_value,action,price\n";
        for (size_t i = 0; i < decisions.size(); ++i) {
            std::string action = "HOLD";
            float price = 0.0f;
    
            if (i < tradeLog.size() && tradeLog[i].date == decisions[i].date) {
                action = tradeLog[i].action;
                price = tradeLog[i].price;
            }
    
            file << dates[i] << ","
                 << portfolioValues[i] << ","
                 << action << ","
                 << price << "\n";
        }
    
        file.close();
        std::cout << "Exported backtest results to " << filename << std::endl;
    }

private:
    float cash = 0.0f;
    float shares = 0.0f;
    float finalValue = 0.0f;

    int buyCount = 0;
    int sellCount = 0;

    std::vector<float> prices;
    std::vector<std::string> dates;
    std::vector<Decision> decisions;
    std::vector<float> portfolioValues;

    struct Trade {
        std::string date;
        std::string action;
        float price;
    };
    std::vector<Trade> tradeLog;
};

#endif // BACKTESTER_HPP
