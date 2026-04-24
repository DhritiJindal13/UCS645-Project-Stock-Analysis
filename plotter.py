import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from mplfinance.original_flavor import candlestick_ohlc
import matplotlib.ticker as ticker
import datetime

# ========== Load CSV Files ==========
indicators_df = pd.read_csv('indicators.csv', parse_dates=['date'])
ohlc_df = pd.read_csv('TSLA_data.csv', parse_dates=['date'])
portfolio_df = pd.read_csv('backtest.csv', parse_dates=['date'])

# ========== Preprocessing ==========
# Convert to datetime format
ohlc_df['date'] = pd.to_datetime(ohlc_df['date'])
indicators_df['date'] = pd.to_datetime(indicators_df['date'])
portfolio_df['date'] = pd.to_datetime(portfolio_df['date'])

# Combine date and OHLC for candlestick plotting
ohlc_data = ohlc_df[['date', 'Open', 'High', 'Low', 'Close']].copy()
ohlc_data['date'] = ohlc_data['date'].map(mdates.date2num)

# ========== Plotting ==========
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(15, 18), sharex=True)

# ----- Candlestick Plot -----
ax1.set_title('Candlestick Chart with Bollinger Bands')
candlestick_ohlc(ax1, ohlc_data.values, width=0.6, colorup='g', colordown='r', alpha=0.8)

# Plot Bollinger Bands
ax1.plot(indicators_df['date'], indicators_df['BB_Mid'], label='BB Mid', color='blue', linestyle='--')
ax1.plot(indicators_df['date'], indicators_df['BB_Upper'], label='BB Upper', color='purple', linestyle='--')
ax1.plot(indicators_df['date'], indicators_df['BB_Lower'], label='BB Lower', color='purple', linestyle='--')
ax1.legend()
ax1.grid(True)
ax1.xaxis.set_major_locator(mdates.MonthLocator())
ax1.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
ax1.xaxis.set_minor_locator(mdates.WeekdayLocator())
ax1.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: datetime.datetime.fromtimestamp(x).strftime('%Y-%m-%d')))
ax1.tick_params(axis='x', rotation=45)
plt.savefig('candlestick_bollinger.png') # Save the first subplot

# Clear the figure for the next subplot
plt.clf()
ax2 = fig.add_subplot(3, 1, 2, sharex=ax1) # Add the subplot back

# ----- Technical Indicators -----
ax2.set_title('Technical Indicators')
ax2.plot(indicators_df['date'], indicators_df['SMA'], label='SMA', color='orange')
ax2.plot(indicators_df['date'], indicators_df['EMA'], label='EMA', color='green')
ax2.plot(indicators_df['date'], indicators_df['RSI'], label='RSI', color='red')
ax2.legend()

# MACD & Signal Line
ax2.plot(indicators_df['date'], indicators_df['MACD'], label='MACD', color='blue')
ax2.plot(indicators_df['date'], indicators_df['Signal'], label='Signal Line', color='pink')
ax2.bar(indicators_df['date'], indicators_df['Histogram'], label='MACD Histogram', color='grey', width=0.8, alpha=0.5)

# Stochastic Oscillator
ax2.plot(indicators_df['date'], indicators_df['Stoch_K'], label='Stochastic %K', color='purple')
ax2.plot(indicators_df['date'], indicators_df['Stoch_D'], label='Stochastic %D', color='brown')
ax2.legend()
ax2.grid(True)
ax2.xaxis.set_major_locator(mdates.MonthLocator())
ax2.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
ax2.xaxis.set_minor_locator(mdates.WeekdayLocator())
ax2.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: datetime.datetime.fromtimestamp(x).strftime('%Y-%m-%d')))
ax2.tick_params(axis='x', rotation=45)
plt.savefig('technical_indicators.png') # Save the second subplot

# Clear the figure for the next subplot
plt.clf()
ax3 = fig.add_subplot(3, 1, 3, sharex=ax1) # Add the subplot back

# # ----- Portfolio Actions -----
ax3.set_title('Portfolio Value and Actions')
ax3.plot(portfolio_df['date'], portfolio_df['portfolio_value'], label='Portfolio Value', color='green')
buy_signals = portfolio_df[portfolio_df['action'] == 'BUY']
sell_signals = portfolio_df[portfolio_df['action'] == 'SELL']

# Plot buy/sell signals
ax3.scatter(buy_signals['date'], buy_signals['price'], label='Buy Signal', color='blue', marker='^', alpha=1)
ax3.scatter(sell_signals['date'], sell_signals['price'], label='Sell Signal', color='red', marker='v', alpha=1)
ax3.legend()
ax3.grid(True)
ax3.xaxis.set_major_locator(mdates.MonthLocator())
ax3.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
ax3.xaxis.set_minor_locator(mdates.WeekdayLocator())
ax3.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: datetime.datetime.fromtimestamp(x).strftime('%Y-%m-%d')))
ax3.tick_params(axis='x', rotation=45)
plt.savefig('portfolio_actions.png') # Save the third subplot

plt.tight_layout()
# plt.show() # Still show the combined plot if you want