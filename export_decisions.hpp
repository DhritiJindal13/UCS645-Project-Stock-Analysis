#ifndef EXPORT_DECISIONS_HPP
#define EXPORT_DECISIONS_HPP

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "decision_logic.hpp"  // for Decision and Signal

void exportDecisionCSV(const std::string& filename, const std::vector<Decision>& decisions, const std::vector<float>& closePrices) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << " for writing." << std::endl;
        return;
    }

    file << "date,close,signal\n";
    for (size_t i = 0; i < decisions.size(); ++i) {
        file << decisions[i].date << ","
             << closePrices[i] << ","
             << signalToString(decisions[i].signal) << "\n";
    }

    file.close();
    std::cout << "Exported decisions to " << filename << std::endl;
}

#endif // EXPORT_DECISIONS_HPP
