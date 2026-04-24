#ifndef INDICATORS_HPP
#define INDICATORS_HPP

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip> 
// #include <cuda_runtime.h>

// -------------------- CPU VERSIONS -------------------- //

std::vector<float> computeSMA(const std::vector<float>& prices, int period) {
    std::vector<float> sma;
    
    // Input validation
    if (prices.size() < period) {
        std::cerr << "Warning: Not enough data points for SMA calculation" << std::endl;
        return sma;  // Return empty vector
    }
    
    std::cout << "Computing SMA with period " << period << " on " << prices.size() << " prices" << std::endl;
    
    for (size_t i = period - 1; i < prices.size(); ++i) {
        float sum = 0;
        for (int j = 0; j < period; ++j)
            sum += prices[i - j];
        sma.push_back(sum / period);
    }
    
    std::cout << "SMA calculation complete. Output size: " << sma.size() << std::endl;
    return sma;
}

std::vector<float> computeEMA(const std::vector<float>& prices, int period) {
    std::vector<float> ema;
    
    // Input validation
    if (prices.size() < period) {
        std::cerr << "Warning: Not enough data points for EMA calculation" << std::endl;
        return ema;  // Return empty vector
    }
    
    std::cout << "Computing EMA with period " << period << " on " << prices.size() << " prices" << std::endl;
    
    // Calculate initial SMA as the first EMA value
    float firstEma = 0;
    for (int i = 0; i < period; ++i) {
        firstEma += prices[i];
    }
    firstEma /= period;
    
    ema.push_back(firstEma);
    
    // Calculate subsequent EMA values
    float multiplier = 2.0f / (period + 1);
    
    for (size_t i = period; i < prices.size(); ++i) {
        float value = (prices[i] - ema.back()) * multiplier + ema.back();
        ema.push_back(value);
    }
    
    std::cout << "EMA calculation complete. Output size: " << ema.size() << std::endl;
    return ema;
}

std::vector<float> computeRSI(const std::vector<float>& prices, int period = 14) {
    std::vector<float> rsi;
    
    // Input validation
    if (prices.size() <= period) {
        std::cerr << "Warning: Not enough data points for RSI calculation" << std::endl;
        return rsi;
    }
    
    std::cout << "Computing RSI with period " << period << " on " << prices.size() << " prices" << std::endl;
    
    // Calculate the initial average gain and loss
    float avgGain = 0, avgLoss = 0;
    
    for (int i = 1; i <= period; ++i) {
        float change = prices[i] - prices[i-1];
        if (change >= 0) {
            avgGain += change;
        } else {
            avgLoss -= change;  // Make positive
        }
    }
    
    avgGain /= period;
    avgLoss /= period;
    
    // Calculate the first RSI
    float rs = (avgLoss == 0) ? 100 : avgGain / avgLoss;
    rsi.push_back(100 - (100 / (1 + rs)));
    
    // Calculate subsequent RSI values
    for (size_t i = period + 1; i < prices.size(); ++i) {
        float change = prices[i] - prices[i-1];
        float gain = (change >= 0) ? change : 0;
        float loss = (change < 0) ? -change : 0;
        
        avgGain = ((period - 1) * avgGain + gain) / period;
        avgLoss = ((period - 1) * avgLoss + loss) / period;
        
        rs = (avgLoss == 0) ? 100 : avgGain / avgLoss;
        rsi.push_back(100 - (100 / (1 + rs)));
    }
    
    std::cout << "RSI calculation complete. Output size: " << rsi.size() << std::endl;
    return rsi;
}

struct BollingerBands {
    std::vector<float> upper;
    std::vector<float> middle;
    std::vector<float> lower;
};

BollingerBands computeBollingerBands(const std::vector<float>& prices, int period = 20, float numStdDev = 2.0f) {
    BollingerBands bb;
    
    // Input validation
    if (prices.size() < period) {
        std::cerr << "Warning: Not enough data points for Bollinger Bands calculation" << std::endl;
        return bb;
    }
    
    std::cout << "Computing Bollinger Bands with period=" << period << ", stdDev=" << numStdDev 
              << " on " << prices.size() << " prices" << std::endl;
    
    for (size_t i = period - 1; i < prices.size(); ++i) {
        float sum = 0;
        for (int j = 0; j < period; ++j)
            sum += prices[i - j];
        float mean = sum / period;

        float variance = 0;
        for (int j = 0; j < period; ++j)
            variance += pow(prices[i - j] - mean, 2);
        float stddev = sqrt(variance / period);

        bb.middle.push_back(mean);
        bb.upper.push_back(mean + numStdDev * stddev);
        bb.lower.push_back(mean - numStdDev * stddev);
    }
    
    std::cout << "Bollinger Bands calculation complete. Size: " << bb.middle.size() << std::endl;
    return bb;
}

//MACD and Stochastic Oscillator claude version
// Corrected MACD implementation with signal line
struct MACDResult {
    std::vector<float> macdLine;     // MACD line (difference between fast and slow EMAs)
    std::vector<float> signalLine;   // Signal line (EMA of MACD line)
    std::vector<float> histogram;    // Histogram (MACD line - signal line)
};

MACDResult computeMACD(const std::vector<float>& prices, int fast = 12, int slow = 26, int signal = 9) {
    MACDResult result;
    
    // Input validation
    if (prices.size() <= slow) {
        std::cerr << "Warning: Not enough data points for MACD calculation" << std::endl;
        return result;
    }
    
    std::cout << "Computing MACD with fast=" << fast << ", slow=" << slow 
              << ", signal=" << signal << " on " << prices.size() << " prices" << std::endl;
    
    std::vector<float> fastEMA = computeEMA(prices, fast);
    std::vector<float> slowEMA = computeEMA(prices, slow);
    
    // Calculate MACD line (fast EMA - slow EMA)
    // We need to align the EMAs since they have different starting points
    size_t fastOffset = prices.size() - fastEMA.size();
    size_t slowOffset = prices.size() - slowEMA.size();
    size_t macdSize = prices.size() - std::max(fastOffset, slowOffset);
    
    for (size_t i = 0; i < macdSize; ++i) {
        size_t fastIndex = i + fastEMA.size() - macdSize;
        size_t slowIndex = i + slowEMA.size() - macdSize;
        result.macdLine.push_back(fastEMA[fastIndex] - slowEMA[slowIndex]);
    }
    
    // Calculate signal line (EMA of MACD line)
    if (result.macdLine.size() >= signal) {
        // Calculate initial SMA for signal line
        float signalSMA = 0;
        for (int i = 0; i < signal; ++i) {
            signalSMA += result.macdLine[i];
        }
        signalSMA /= signal;
        result.signalLine.push_back(signalSMA);
        
        // Calculate EMA of MACD line
        float multiplier = 2.0f / (signal + 1);
        for (size_t i = signal; i < result.macdLine.size(); ++i) {
            float value = (result.macdLine[i] - result.signalLine.back()) * multiplier + result.signalLine.back();
            result.signalLine.push_back(value);
        }
        
        // Calculate histogram (MACD line - signal line)
        size_t histStart = result.macdLine.size() - result.signalLine.size();
        for (size_t i = 0; i < result.signalLine.size(); ++i) {
            result.histogram.push_back(result.macdLine[i + histStart] - result.signalLine[i]);
        }
    }
    
    std::cout << "MACD calculation complete. MACD size: " << result.macdLine.size() 
              << ", Signal size: " << result.signalLine.size() 
              << ", Histogram size: " << result.histogram.size() << std::endl;
    
    return result;
}

// Corrected Stochastic Oscillator implementation with %K and %D
struct StochasticResult {
    std::vector<float> k;  // %K line (raw stochastic)
    std::vector<float> d;  // %D line (moving average of %K)
};

StochasticResult computeStochasticOscillator(
    const std::vector<float>& close, 
    const std::vector<float>& high, 
    const std::vector<float>& low, 
    int periodK = 14,     // Period for %K calculation
    int periodD = 3,      // Period for %D calculation (SMA of %K)
    int smoothK = 1       // Smoothing for %K (1 = no smoothing)
) {
    StochasticResult result;
    
    // Input validation
    size_t minSize = std::min({close.size(), high.size(), low.size()});
    if (minSize < periodK) {
        std::cerr << "Warning: Not enough data points for Stochastic Oscillator calculation" << std::endl;
        return result;
    }
    
    std::cout << "Computing Stochastic Oscillator with periodK=" << periodK 
              << ", smoothK=" << smoothK << ", periodD=" << periodD
              << " on " << minSize << " data points" << std::endl;
    
    // Calculate raw %K values
    std::vector<float> rawK;
    for (size_t i = periodK - 1; i < minSize; ++i) {
        float highest = *std::max_element(high.begin() + i - periodK + 1, high.begin() + i + 1);
        float lowest = *std::min_element(low.begin() + i - periodK + 1, low.begin() + i + 1);
        
        // Prevent division by zero
        if (highest == lowest) {
            rawK.push_back(50.0f);  // Middle value if no range
        } else {
            float value = 100 * ((close[i] - lowest) / (highest - lowest));
            rawK.push_back(value);
        }
    }
    
    // Apply smoothing to %K if requested (smoothK > 1)
    if (smoothK > 1 && rawK.size() >= smoothK) {
        for (size_t i = smoothK - 1; i < rawK.size(); ++i) {
            float sum = 0;
            for (int j = 0; j < smoothK; ++j) {
                sum += rawK[i - j];
            }
            result.k.push_back(sum / smoothK);
        }
    } else {
        // No smoothing required
        result.k = rawK;
    }
    
    // Calculate %D (SMA of %K)
    if (result.k.size() >= periodD) {
        for (size_t i = periodD - 1; i < result.k.size(); ++i) {
            float sum = 0;
            for (int j = 0; j < periodD; ++j) {
                sum += result.k[i - j];
            }
            result.d.push_back(sum / periodD);
        }
    }
    
    std::cout << "Stochastic Oscillator calculation complete. %K size: " << result.k.size() 
              << ", %D size: " << result.d.size() << std::endl;
    
    return result;
}


// ATR (Average True Range) calculation - add this function if not already present
std::vector<float> computeATR(const std::vector<float>& high, const std::vector<float>& low, 
    const std::vector<float>& close, int period = 14) {
std::vector<float> atr;

// Input validation
if (high.size() < period + 1 || low.size() < period + 1 || close.size() < period + 1) {
std::cerr << "Warning: Not enough data points for ATR calculation" << std::endl;
return atr;
}

std::cout << "Computing ATR with period " << period << " on " << high.size() << " data points" << std::endl;

std::vector<float> trueRange;

// Calculate True Range for each bar
for (size_t i = 1; i < high.size(); ++i) {
float highLowRange = high[i] - low[i];
float highClosePrevRange = std::abs(high[i] - close[i-1]);
float lowClosePrevRange = std::abs(low[i] - close[i-1]);

float tr = std::max({highLowRange, highClosePrevRange, lowClosePrevRange});
trueRange.push_back(tr);
}

// Calculate initial ATR (simple average of TR for first 'period' values)
float sum = 0;
for (int i = 0; i < period; ++i) {
sum += trueRange[i];
}
float firstATR = sum / period;
atr.push_back(firstATR);

// Calculate subsequent ATR values using EMA formula
for (size_t i = period; i < trueRange.size(); ++i) {
float currentATR = (atr.back() * (period - 1) + trueRange[i]) / period;
atr.push_back(currentATR);
}

std::cout << "ATR calculation complete. Output size: " << atr.size() << std::endl;
return atr;
}


// -------------------- GPU VERSION -------------------- //

// __global__ void smaKernel(float* prices, float* output, int n, int period) {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
//     // Map thread index to output position (0 to sizeOut-1)
//     if (idx < n - period + 1) {
//         float sum = 0.0f;
//         // Calculate sum for the current window
//         // For output[0], we need prices[period-1] down to prices[0]
//         // For output[1], we need prices[period] down to prices[1]
//         // So for output[idx], we need prices[idx+period-1] down to prices[idx]
//         for (int j = 0; j < period; ++j) {
//             sum += prices[idx + period - 1 - j];
//         }
//         output[idx] = sum / period;
//     }
// }

// void computeSMA_GPU(const std::vector<float>& prices, std::vector<float>& result, int period) {
//     int n = prices.size();
    
//     // Input validation
//     if (n < period) {
//         std::cerr << "Warning: Not enough data points for SMA calculation" << std::endl;
//         result.clear();  // Return empty vector
//         return;
//     }
    
//     int sizeOut = n - period + 1;
//     std::cout << "Computing GPU SMA with period " << period << " on " << n << " prices" << std::endl;

//     float *d_prices, *d_output;
//     cudaMalloc(&d_prices, n * sizeof(float));
//     cudaMalloc(&d_output, sizeOut * sizeof(float));

//     cudaMemcpy(d_prices, prices.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     int blockSize = 256;
//     int numBlocks = (sizeOut + blockSize - 1) / blockSize; // Calculate blocks based on output size
//     smaKernel<<<numBlocks, blockSize>>>(d_prices, d_output, n, period);
    
//     // Check for kernel launch errors
//     cudaError_t err = cudaGetLastError();
//     if (err != cudaSuccess) {
//         std::cerr << "CUDA Kernel Error: " << cudaGetErrorString(err) << std::endl;
//     }
    
//     cudaDeviceSynchronize();

//     result.resize(sizeOut);
//     cudaMemcpy(result.data(), d_output, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);

//     std::cout << "GPU SMA calculation complete. Output size: " << result.size() << std::endl;

//     cudaFree(d_prices);
//     cudaFree(d_output);
// }

// __global__ void emaKernel(float* prices, float* output, int n, int period, float multiplier) {
//     // Only one thread does the computation
//     if (threadIdx.x == 0 && blockIdx.x == 0) {
//         // Compute initial SMA as first EMA value
//         float sum = 0.0f;
//         for (int i = 0; i < period; ++i) {
//             sum += prices[i];
//         }
//         output[0] = sum / period;

//         // Compute EMA from index `period`
//         for (int i = period; i < n; ++i) {
//             output[i - period + 1] = (prices[i] - output[i - period]) * multiplier + output[i - period];
//         }
//     }
// }


// void computeEMA_GPU(const std::vector<float>& prices, std::vector<float>& result, int period) {
//     int n = prices.size();

//     if (n < period) {
//         std::cerr << "Warning: Not enough data points for EMA calculation" << std::endl;
//         result.clear();
//         return;
//     }

//     float multiplier = 2.0f / (period + 1);
//     int sizeOut = n - period + 1;

//     float *d_prices, *d_output;
//     cudaMalloc(&d_prices, n * sizeof(float));
//     cudaMalloc(&d_output, sizeOut * sizeof(float));

//     cudaMemcpy(d_prices, prices.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     // Launch with 1 thread (sequential logic)
//     emaKernel<<<1, 1>>>(d_prices, d_output, n, period, multiplier);
//     cudaDeviceSynchronize();

//     result.resize(sizeOut);
//     cudaMemcpy(result.data(), d_output, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);

//     cudaFree(d_prices);
//     cudaFree(d_output);

//     std::cout << "GPU EMA calculation complete. Output size: " << result.size() << std::endl;
// }

// __global__ void rsiKernel(float* prices, float* output, int n, int period) {
//     if (threadIdx.x == 0 && blockIdx.x == 0) {
//         if (n <= period) return;

//         float avgGain = 0.0f, avgLoss = 0.0f;

//         // Initial average gain and loss
//         for (int i = 1; i <= period; ++i) {
//             float change = prices[i] - prices[i - 1];
//             if (change >= 0)
//                 avgGain += change;
//             else
//                 avgLoss -= change;
//         }

//         avgGain /= period;
//         avgLoss /= period;

//         float rs = (avgLoss == 0.0f) ? 100.0f : avgGain / avgLoss;
//         output[0] = 100.0f - (100.0f / (1.0f + rs));

//         // Rolling averages
//         int outIndex = 1;
//         for (int i = period + 1; i < n; ++i) {
//             float change = prices[i] - prices[i - 1];
//             float gain = (change >= 0) ? change : 0.0f;
//             float loss = (change < 0) ? -change : 0.0f;

//             avgGain = ((period - 1) * avgGain + gain) / period;
//             avgLoss = ((period - 1) * avgLoss + loss) / period;

//             rs = (avgLoss == 0.0f) ? 100.0f : avgGain / avgLoss;
//             output[outIndex++] = 100.0f - (100.0f / (1.0f + rs));
//         }
//     }
// }

// void computeRSI_GPU(const std::vector<float>& prices, std::vector<float>& rsi, int period = 14) {
//     int n = prices.size();

//     if (n <= period) {
//         std::cerr << "Warning: Not enough data points for RSI calculation" << std::endl;
//         rsi.clear();
//         return;
//     }

//     std::cout << "Computing GPU RSI with period " << period << " on " << n << " prices" << std::endl;

//     int sizeOut = n - period;  // Matches CPU version

//     float *d_prices, *d_output;
//     cudaMalloc(&d_prices, n * sizeof(float));
//     cudaMalloc(&d_output, sizeOut * sizeof(float));

//     cudaMemcpy(d_prices, prices.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     rsiKernel<<<1, 1>>>(d_prices, d_output, n, period);
//     cudaDeviceSynchronize();

//     rsi.resize(sizeOut);
//     cudaMemcpy(rsi.data(), d_output, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);

//     cudaFree(d_prices);
//     cudaFree(d_output);

//     std::cout << "GPU RSI calculation complete. Output size: " << rsi.size() << std::endl;
// }

// __global__ void bollingerKernel(
//     const float* prices, float* middle, float* upper, float* lower,
//     int n, int period, float numStdDev
// ) {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;

//     int sizeOut = n - period + 1;
//     if (idx >= sizeOut) return;

//     float sum = 0.0f;
//     for (int j = 0; j < period; ++j)
//         sum += prices[idx + j];

//     float mean = sum / period;

//     float variance = 0.0f;
//     for (int j = 0; j < period; ++j) {
//         float diff = prices[idx + j] - mean;
//         variance += diff * diff;
//     }

//     float stddev = sqrtf(variance / period);

//     middle[idx] = mean;
//     upper[idx] = mean + numStdDev * stddev;
//     lower[idx] = mean - numStdDev * stddev;
// }

// BollingerBands computeBollingerBands_GPU(const std::vector<float>& prices, int period = 20, float numStdDev = 2.0f) {
//     BollingerBands bb;
//     int n = prices.size();

//     if (n < period) {
//         std::cerr << "Warning: Not enough data points for Bollinger Bands calculation" << std::endl;
//         return bb;
//     }

//     int sizeOut = n - period + 1;

//     std::cout << "Computing GPU Bollinger Bands with period=" << period << ", stdDev=" << numStdDev 
//               << " on " << n << " prices" << std::endl;

//     // Allocate device memory
//     float *d_prices, *d_middle, *d_upper, *d_lower;
//     cudaMalloc(&d_prices, n * sizeof(float));
//     cudaMalloc(&d_middle, sizeOut * sizeof(float));
//     cudaMalloc(&d_upper, sizeOut * sizeof(float));
//     cudaMalloc(&d_lower, sizeOut * sizeof(float));

//     // Copy input to device
//     cudaMemcpy(d_prices, prices.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     // Launch kernel
//     int blockSize = 256;
//     int numBlocks = (sizeOut + blockSize - 1) / blockSize;
//     bollingerKernel<<<numBlocks, blockSize>>>(d_prices, d_middle, d_upper, d_lower, n, period, numStdDev);
//     cudaDeviceSynchronize();

//     // Resize output and copy from device
//     bb.middle.resize(sizeOut);
//     bb.upper.resize(sizeOut);
//     bb.lower.resize(sizeOut);

//     cudaMemcpy(bb.middle.data(), d_middle, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);
//     cudaMemcpy(bb.upper.data(), d_upper, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);
//     cudaMemcpy(bb.lower.data(), d_lower, sizeOut * sizeof(float), cudaMemcpyDeviceToHost);

//     // Free device memory
//     cudaFree(d_prices);
//     cudaFree(d_middle);
//     cudaFree(d_upper);
//     cudaFree(d_lower);

//     std::cout << "GPU Bollinger Bands calculation complete. Size: " << bb.middle.size() << std::endl;
//     return bb;
// }

// __global__ void macdKernel(
//     const float* prices, float* macdOut, float* signalOut, float* histOut,
//     int n, int fast, int slow, int signalPeriod
// ) {
//     if (threadIdx.x != 0 || blockIdx.x != 0) return;

//     if (n <= slow) return;  // Not enough data

//     float fastMultiplier = 2.0f / (fast + 1);
//     float slowMultiplier = 2.0f / (slow + 1);
//     float signalMultiplier = 2.0f / (signalPeriod + 1);

//     // Compute fast EMA
//     float fastEMA[n - fast + 1];
//     float sumFast = 0.0f;
//     for (int i = 0; i < fast; ++i) sumFast += prices[i];
//     fastEMA[0] = sumFast / fast;
//     for (int i = fast; i < n; ++i)
//         fastEMA[i - fast + 1] = (prices[i] - fastEMA[i - fast]) * fastMultiplier + fastEMA[i - fast];

//     // Compute slow EMA
//     float slowEMA[n - slow + 1];
//     float sumSlow = 0.0f;
//     for (int i = 0; i < slow; ++i) sumSlow += prices[i];
//     slowEMA[0] = sumSlow / slow;
//     for (int i = slow; i < n; ++i)
//         slowEMA[i - slow + 1] = (prices[i] - slowEMA[i - slow]) * slowMultiplier + slowEMA[i - slow];

//     // Align lengths for MACD Line
//     int offset = (slow > fast) ? slow - fast : 0;
//     int macdSize = n - slow + 1;
//     for (int i = 0; i < macdSize; ++i)
//         macdOut[i] = fastEMA[i + offset] - slowEMA[i];

//     // Signal Line (EMA of MACD line)
//     if (macdSize < signalPeriod) return;

//     float sumSignal = 0.0f;
//     for (int i = 0; i < signalPeriod; ++i)
//         sumSignal += macdOut[i];
//     signalOut[0] = sumSignal / signalPeriod;

//     for (int i = signalPeriod; i < macdSize; ++i)
//         signalOut[i - signalPeriod + 1] = (macdOut[i] - signalOut[i - signalPeriod]) * signalMultiplier + signalOut[i - signalPeriod];

//     // Histogram
//     int signalSize = macdSize - signalPeriod + 1;
//     for (int i = 0; i < signalSize; ++i)
//         histOut[i] = macdOut[i + signalPeriod - 1] - signalOut[i];
// }


// MACDResult computeMACD_GPU(const std::vector<float>& prices, int fast = 12, int slow = 26, int signal = 9) {
//     MACDResult result;
//     int n = prices.size();

//     if (n <= slow) {
//         std::cerr << "Warning: Not enough data points for MACD calculation" << std::endl;
//         return result;
//     }

//     std::cout << "Computing GPU MACD with fast=" << fast << ", slow=" << slow 
//               << ", signal=" << signal << " on " << n << " prices" << std::endl;

//     int macdSize = n - slow + 1;
//     int signalSize = macdSize - signal + 1;

//     // Allocate device memory
//     float *d_prices, *d_macd, *d_signal, *d_hist;
//     cudaMalloc(&d_prices, n * sizeof(float));
//     cudaMalloc(&d_macd, macdSize * sizeof(float));
//     cudaMalloc(&d_signal, signalSize * sizeof(float));
//     cudaMalloc(&d_hist, signalSize * sizeof(float));

//     cudaMemcpy(d_prices, prices.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     macdKernel<<<1, 1>>>(d_prices, d_macd, d_signal, d_hist, n, fast, slow, signal);
//     cudaDeviceSynchronize();

//     result.macdLine.resize(macdSize);
//     result.signalLine.resize(signalSize);
//     result.histogram.resize(signalSize);

//     cudaMemcpy(result.macdLine.data(), d_macd, macdSize * sizeof(float), cudaMemcpyDeviceToHost);
//     cudaMemcpy(result.signalLine.data(), d_signal, signalSize * sizeof(float), cudaMemcpyDeviceToHost);
//     cudaMemcpy(result.histogram.data(), d_hist, signalSize * sizeof(float), cudaMemcpyDeviceToHost);

//     cudaFree(d_prices);
//     cudaFree(d_macd);
//     cudaFree(d_signal);
//     cudaFree(d_hist);

//     std::cout << "GPU MACD calculation complete. MACD size: " << result.macdLine.size()
//               << ", Signal size: " << result.signalLine.size()
//               << ", Histogram size: " << result.histogram.size() << std::endl;

//     return result;
// }

// __global__ void stochasticKernel(
//     const float* close, const float* high, const float* low,
//     float* kOut, float* dOut,
//     int n, int periodK, int smoothK, int periodD
// ) {
//     if (threadIdx.x != 0 || blockIdx.x != 0) return;

//     int rawKSize = n - periodK + 1;

//     // Temporary rawK array
//     float rawK[1000];  // Adjust based on expected max size or make dynamic if needed

//     // Step 1: Calculate raw %K
//     for (int i = 0; i < rawKSize; ++i) {
//         float highest = high[i];
//         float lowest = low[i];
//         for (int j = 1; j < periodK; ++j) {
//             highest = fmaxf(high[i + j], highest);
//             lowest = fminf(low[i + j], lowest);
//         }

//         if (highest == lowest) {
//             rawK[i] = 50.0f;  // Avoid division by zero
//         } else {
//             rawK[i] = 100.0f * (close[i + periodK - 1] - lowest) / (highest - lowest);
//         }
//     }

//     // Step 2: Smooth %K
//     int smoothKSize = rawKSize - smoothK + 1;
//     for (int i = 0; i < smoothKSize; ++i) {
//         float sum = 0;
//         for (int j = 0; j < smoothK; ++j)
//             sum += rawK[i + j];
//         kOut[i] = sum / smoothK;
//     }

//     // Step 3: Compute %D (SMA of smoothed %K)
//     int dSize = smoothKSize - periodD + 1;
//     for (int i = 0; i < dSize; ++i) {
//         float sum = 0;
//         for (int j = 0; j < periodD; ++j)
//             sum += kOut[i + j];
//         dOut[i] = sum / periodD;
//     }
// }

// StochasticResult computeStochastic_GPU(
//     const std::vector<float>& close,
//     const std::vector<float>& high,
//     const std::vector<float>& low,
//     int periodK = 14, int periodD = 3, int smoothK = 1
// ) {
//     StochasticResult result;
//     int n = close.size();
//     int rawKSize = n - periodK + 1;
//     int kSize = rawKSize - smoothK + 1;
//     int dSize = kSize - periodD + 1;

//     if (n < periodK || high.size() != n || low.size() != n) {
//         std::cerr << "Warning: Not enough or mismatched data for Stochastic Oscillator" << std::endl;
//         return result;
//     }

//     std::cout << "Computing GPU Stochastic Oscillator with periodK=" << periodK 
//               << ", smoothK=" << smoothK << ", periodD=" << periodD
//               << " on " << n << " prices" << std::endl;

//     // Allocate device memory
//     float *d_close, *d_high, *d_low, *d_k, *d_d;
//     cudaMalloc(&d_close, n * sizeof(float));
//     cudaMalloc(&d_high, n * sizeof(float));
//     cudaMalloc(&d_low,  n * sizeof(float));
//     cudaMalloc(&d_k,     kSize * sizeof(float));
//     cudaMalloc(&d_d,     dSize * sizeof(float));

//     cudaMemcpy(d_close, close.data(), n * sizeof(float), cudaMemcpyHostToDevice);
//     cudaMemcpy(d_high,  high.data(),  n * sizeof(float), cudaMemcpyHostToDevice);
//     cudaMemcpy(d_low,   low.data(),   n * sizeof(float), cudaMemcpyHostToDevice);

//     // Launch single-threaded kernel
//     stochasticKernel<<<1, 1>>>(d_close, d_high, d_low, d_k, d_d, n, periodK, smoothK, periodD);
//     cudaDeviceSynchronize();

//     // Copy results back
//     result.k.resize(kSize);
//     result.d.resize(dSize);
//     cudaMemcpy(result.k.data(), d_k, kSize * sizeof(float), cudaMemcpyDeviceToHost);
//     cudaMemcpy(result.d.data(), d_d, dSize * sizeof(float), cudaMemcpyDeviceToHost);

//     // Cleanup
//     cudaFree(d_close);
//     cudaFree(d_high);
//     cudaFree(d_low);
//     cudaFree(d_k);
//     cudaFree(d_d);

//     std::cout << "Stochastic Oscillator GPU calculation complete. %K size: " << result.k.size()
//               << ", %D size: " << result.d.size() << std::endl;

//     return result;
// }

// __global__ void atrKernel(
//     const float* high, const float* low, const float* close,
//     float* trOut, float* atrOut,
//     int n, int period
// ) {
//     if (threadIdx.x != 0 || blockIdx.x != 0) return;

//     // Step 1: Calculate True Range (TR)
//     for (int i = 1; i < n; ++i) {
//         float highLow = high[i] - low[i];
//         float highClosePrev = fabsf(high[i] - close[i - 1]);
//         float lowClosePrev = fabsf(low[i] - close[i - 1]);
//         trOut[i - 1] = fmaxf(fmaxf(highLow, highClosePrev), lowClosePrev);
//     }

//     // Step 2: Calculate initial ATR (simple average of first `period` TR values)
//     float sum = 0;
//     for (int i = 0; i < period; ++i)
//         sum += trOut[i];
//     atrOut[0] = sum / period;

//     // Step 3: EMA-style ATR calculation
//     for (int i = period; i < n - 1; ++i) {
//         atrOut[i - period + 1] = ((atrOut[i - period] * (period - 1)) + trOut[i]) / period;
//     }
// }

// std::vector<float> computeATR_GPU(
//     const std::vector<float>& high,
//     const std::vector<float>& low,
//     const std::vector<float>& close,
//     int period = 14
// ) {
//     std::vector<float> atr;

//     int n = high.size();
//     if (n < period + 1 || low.size() != n || close.size() != n) {
//         std::cerr << "Warning: Not enough or mismatched data points for ATR calculation" << std::endl;
//         return atr;
//     }

//     std::cout << "Computing ATR (GPU) with period " << period << " on " << n << " data points" << std::endl;

//     int trSize = n - 1;
//     int atrSize = trSize - period + 1;

//     float *d_high, *d_low, *d_close, *d_tr, *d_atr;
//     cudaMalloc(&d_high,  n * sizeof(float));
//     cudaMalloc(&d_low,   n * sizeof(float));
//     cudaMalloc(&d_close, n * sizeof(float));
//     cudaMalloc(&d_tr,    trSize * sizeof(float));
//     cudaMalloc(&d_atr,   atrSize * sizeof(float));

//     cudaMemcpy(d_high,  high.data(),  n * sizeof(float), cudaMemcpyHostToDevice);
//     cudaMemcpy(d_low,   low.data(),   n * sizeof(float), cudaMemcpyHostToDevice);
//     cudaMemcpy(d_close, close.data(), n * sizeof(float), cudaMemcpyHostToDevice);

//     atrKernel<<<1, 1>>>(d_high, d_low, d_close, d_tr, d_atr, n, period);
//     cudaDeviceSynchronize();

//     atr.resize(atrSize);
//     cudaMemcpy(atr.data(), d_atr, atrSize * sizeof(float), cudaMemcpyDeviceToHost);

//     cudaFree(d_high);
//     cudaFree(d_low);
//     cudaFree(d_close);
//     cudaFree(d_tr);
//     cudaFree(d_atr);

//     std::cout << "ATR GPU calculation complete. Output size: " << atr.size() << std::endl;
//     return atr;
// }





void exportIndicatorsToCSV(
    const std::string& filename,
    const std::vector<std::string>& dates,       // full-length date vector (same as prices)
    const std::vector<float>& prices,
    const std::vector<float>& sma,
    const std::vector<float>& ema,
    const std::vector<float>& rsi,
    const BollingerBands& bb,
    const MACDResult& macd,
    const StochasticResult& stoch,
    const std::vector<float>& atr,
    int smaPeriod,
    int emaPeriod,
    int rsiPeriod,
    int bbPeriod,
    int macdSlowPeriod,
    int stochKPeriod,
    int atrPeriod
) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Write header
    file << "Date,Price,SMA,EMA,RSI,BB_Mid,BB_Upper,BB_Lower,MACD,Signal,Histogram,Stoch_K,Stoch_D,ATR\n";

    // Calculate the max offset to align all indicators
    size_t maxOffset = std::max({
        (size_t)smaPeriod - 1,
        (size_t)emaPeriod - 1,
        (size_t)rsiPeriod,
        (size_t)bbPeriod - 1,
        (size_t)macdSlowPeriod,
        (size_t)stochKPeriod - 1,
        (size_t)atrPeriod
    });

    size_t smaOffset = sma.size() - (prices.size() - maxOffset);
    size_t emaOffset = ema.size() - (prices.size() - maxOffset);
    size_t rsiOffset = rsi.size() - (prices.size() - maxOffset);
    size_t bbOffset  = bb.middle.size() - (prices.size() - maxOffset);
    size_t macdOffset = macd.macdLine.size() - (prices.size() - maxOffset);
    size_t signalOffset = macd.signalLine.size() < macd.macdLine.size()
                          ? macd.macdLine.size() - macd.signalLine.size()
                          : 0;
    size_t histOffset = macd.histogram.size() < macd.signalLine.size()
                        ? macd.signalLine.size() - macd.histogram.size()
                        : 0;
    size_t stochOffset = stoch.k.size() - (prices.size() - maxOffset);
    size_t stochDOffset = stoch.d.size() < stoch.k.size()
                          ? stoch.k.size() - stoch.d.size()
                          : 0;
    size_t atrOffset = atr.size() - (prices.size() - maxOffset);

    for (size_t i = maxOffset; i < prices.size(); ++i) {
        size_t index = i - maxOffset;

        file << dates[i] << "," 
             << prices[i] << ",";

        // Safe access with fallback
        auto get = [](const std::vector<float>& v, size_t i) {
            return (i < v.size()) ? v[i] : NAN;
        };

        file << get(sma, index - smaOffset) << ","
             << get(ema, index - emaOffset) << ","
             << get(rsi, index - rsiOffset) << ","
             << get(bb.middle, index - bbOffset) << ","
             << get(bb.upper, index - bbOffset) << ","
             << get(bb.lower, index - bbOffset) << ","
             << get(macd.macdLine, index - macdOffset) << ","
             << get(macd.signalLine, index - macdOffset - signalOffset) << ","
             << get(macd.histogram, index - macdOffset - signalOffset - histOffset) << ","
             << get(stoch.k, index - stochOffset) << ","
             << get(stoch.d, index - stochOffset - stochDOffset) << ","
             << get(atr, index - atrOffset) << "\n";
    }

    file.close();
    std::cout << "Indicators written to CSV: " << filename << std::endl;
}


#endif // INDICATORS_HPP