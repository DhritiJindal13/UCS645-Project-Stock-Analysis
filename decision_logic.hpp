#ifndef DECISION_LOGIC_HPP
#define DECISION_LOGIC_HPP

#include <vector>
#include <string>
#include <iostream>
#include "indicators.hpp"

enum class Signal {
    BUY,
    SELL,
    HOLD
};

struct Decision {
    std::string date;
    Signal signal;
};

std::string signalToString(Signal signal) {
    switch (signal) {
        case Signal::BUY: return "BUY";
        case Signal::SELL: return "SELL";
        default: return "HOLD";
    }
}

std::vector<Decision> generateDecisions(
    const std::vector<std::string>& dates,
    const std::vector<float>& close,
    const std::vector<float>& high,
    const std::vector<float>& low,
    const std::vector<float>& volume,
    bool use_gpu = false
) {
    std::vector<Decision> decisions;

   
    const int period = 20; 

    const int trend_period = 50;

    if (dates.size() <= trend_period || close.size() <= trend_period) {
        std::cerr << "Not enough data for trend analysis." << std::endl;
        return decisions;
    }

   
    std::vector<float> sma = computeSMA(close, period);
    std::vector<float> ema = computeEMA(close, period);
    std::vector<float> rsi = computeRSI(close, period);
    MACDResult macd = computeMACD(close, 12, 26, 9); 
    BollingerBands bb = computeBollingerBands(close, period, 2.0);
    StochasticResult stoch = computeStochasticOscillator(close, high, low, period, 3, 3);
    
   
    std::vector<float> long_ema = computeEMA(close, trend_period);
    std::vector<float> atr = computeATR(high, low, close, period); 

 
    size_t sma_offset = close.size() - sma.size();
    size_t ema_offset = close.size() - ema.size();
    size_t long_ema_offset = close.size() - long_ema.size();
    size_t rsi_offset = close.size() - rsi.size();
    size_t macd_offset = close.size() - macd.macdLine.size();
    size_t macd_signal_offset = close.size() - macd.signalLine.size();
    size_t bb_offset = close.size() - bb.middle.size();
    size_t stoch_k_offset = close.size() - stoch.k.size();
    size_t stoch_d_offset = close.size() - stoch.d.size();
    size_t atr_offset = close.size() - atr.size();
    
    
    size_t max_offset = std::max({sma_offset, ema_offset, long_ema_offset, rsi_offset, macd_offset, 
                                 macd_signal_offset, bb_offset, stoch_k_offset, stoch_d_offset, atr_offset});
    
   
    if (max_offset >= close.size()) {
        std::cerr << "Not enough aligned data for decision generation." << std::endl;
        return decisions;
    }

   
    size_t usable_start = max_offset;
    size_t usable_end = close.size();

    std::cout << "Starting decision generation from index " << usable_start 
              << " (data point: " << dates[usable_start] << ")" << std::endl;
    
   
    bool in_position = false;
    float entry_price = 0.0f;
    size_t entry_idx = 0;
    float stop_loss = 0.0f;
    
   
    float avg_vol = 0.0f;
    for (size_t i = usable_start; i < usable_start + 10 && i < usable_end; ++i) {
        size_t atr_idx = i - atr_offset;
        if (atr_idx < atr.size()) {
            avg_vol += atr[atr_idx];
        }
    }
    avg_vol /= 10.0f;
    
   
    const int buy_threshold = 4;  
    const int sell_threshold = -4;

    for (size_t i = usable_start; i < usable_end; ++i) {
        float c = close[i];
        float prev_c = close[i-1];
        
       
        size_t ema_idx = i - ema_offset;
        size_t prev_ema_idx = (ema_idx > 0) ? (ema_idx - 1) : 0;
        size_t long_ema_idx = i - long_ema_offset;
        size_t rsi_idx = i - rsi_offset;
        size_t macd_idx = i - macd_offset;
        size_t prev_macd_idx = (macd_idx > 0) ? (macd_idx - 1) : 0;
        size_t signal_idx = i - macd_signal_offset;
        size_t prev_signal_idx = (signal_idx > 0) ? (signal_idx - 1) : 0;
        size_t bb_idx = i - bb_offset;
        size_t stoch_k_idx = i - stoch_k_offset;
        size_t stoch_d_idx = i - stoch_d_offset;
        size_t atr_idx = i - atr_offset;
        
      
        if (ema_idx >= ema.size() || long_ema_idx >= long_ema.size() || 
            rsi_idx >= rsi.size() || macd_idx >= macd.macdLine.size() || 
            signal_idx >= macd.signalLine.size() || bb_idx >= bb.upper.size() || 
            stoch_k_idx >= stoch.k.size() || stoch_d_idx >= stoch.d.size() ||
            atr_idx >= atr.size()) {
            std::cerr << "Index out of bounds at date: " << dates[i] << std::endl;
            continue;
        }

        float e = ema[ema_idx];
        float prev_e = (prev_ema_idx < ema.size()) ? ema[prev_ema_idx] : e;
        float long_e = long_ema[long_ema_idx];
        float r = rsi[rsi_idx];
        float m = macd.macdLine[macd_idx];
        float prev_m = (prev_macd_idx < macd.macdLine.size()) ? macd.macdLine[prev_macd_idx] : m;
        float signal = macd.signalLine[signal_idx];
        float prev_signal = (prev_signal_idx < macd.signalLine.size()) ? macd.signalLine[prev_signal_idx] : signal;
        float b_upper = bb.upper[bb_idx];
        float b_middle = bb.middle[bb_idx];
        float b_lower = bb.lower[bb_idx];
        float k = stoch.k[stoch_k_idx];
        float d = stoch.d[stoch_d_idx];
        float current_atr = atr[atr_idx];
        
       
        bool high_volatility = (current_atr > avg_vol * 1.5);
        
       
        bool strong_uptrend = (c > long_e) && (e > long_e);
        bool strong_downtrend = (c < long_e) && (e < long_e);
        
       
        Signal signal_decision = Signal::HOLD;
        
        if (in_position && c <= stop_loss) {
          
            signal_decision = Signal::SELL;
            in_position = false;
            
            std::cout << "STOP LOSS @ " << dates[i] 
                      << "  Entry: " << entry_price 
                      << ", Exit: " << c
                      << ", Loss: " << ((c / entry_price - 1) * 100) << "%"
                      << std::endl;
        }
        else {
            int score = 0;

         
            if (!in_position && !high_volatility) {
              
                if (strong_uptrend) score += 2; 
                
             
                if (c > e && prev_c <= prev_e) score += 2;
                
             
                if (r < 30 && r > r - 5) score++; 
                
           
                if (m > signal && prev_m <= prev_signal) score += 2; 
                if (m < 0 && m > prev_m && m - prev_m > 0.1 * current_atr) score++; 
                
             
                if (c < b_lower) score++;
                if (c > b_lower && prev_c < b_lower) score++; 
                
            
                if (k < 20 && k > d && k > stoch.k[stoch_k_idx-1]) score += 2; 
                
                if (i > 0 && volume[i] > volume[i-1] * 1.2 && c > prev_c) score++;
                
             
                if (score >= buy_threshold) {
                    signal_decision = Signal::BUY;
                    in_position = true;
                    entry_price = c;
                    entry_idx = i;
                    
                    stop_loss = entry_price * (1.0 - (2.0 * current_atr / entry_price));
                }
            }
       
            else if (in_position || !high_volatility) {
              
                if (strong_downtrend) score -= 2;
                
               
                if (c < e && prev_c >= prev_e) score -= 2;
                
              
                if (r > 70 && r < r + 5) score--; 
                
              
                if (m < signal && prev_m >= prev_signal) score -= 2; 
                if (m > 0 && m < prev_m && prev_m - m > 0.1 * current_atr) score--; 
                
                
                if (c > b_upper) score--; 
                if (c < b_upper && prev_c > b_upper) score--; 
                
                if (k > 80 && k < d && k < stoch.k[stoch_k_idx-1]) score -= 2; 
                
              
                if (in_position) {
                  
                    if (c > entry_price * (1.0 + (3.0 * current_atr / entry_price))) {
                        score -= 4; 
                    }
                    
                 
                    if (i - entry_idx > 5) {
                        score--;
                    }
                }
                
          
                if (score <= sell_threshold) {
                    signal_decision = Signal::SELL;
                    in_position = false;
                }
            }
        }

        decisions.push_back({dates[i], signal_decision});

  
    }

    std::cout << "Generated " << decisions.size() << " decisions" << std::endl;
    return decisions;
}



#endif 
