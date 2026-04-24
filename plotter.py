import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
from mplfinance.original_flavor import candlestick_ohlc
import datetime
 
indicators_df = pd.read_csv('indicators.csv', parse_dates=['date'])
ohlc_df = pd.read_csv('TSLA_data.csv', parse_dates=['date'])
portfolio_df = pd.read_csv('backtest.csv', parse_dates=['date'])
 
ohlc_df['date'] = pd.to_datetime(ohlc_df['date'])
indicators_df['date'] = pd.to_datetime(indicators_df['date'])
portfolio_df['date'] = pd.to_datetime(portfolio_df['date'])
 
ohlc_data = ohlc_df[['date', 'Open', 'High', 'Low', 'Close']].copy()
ohlc_data['date'] = ohlc_data['date'].map(mdates.date2num)
 
def format_date(x, _):
    try:
        return mdates.num2date(x).strftime('%Y-%m-%d')
    except:
        return ''
 
fig1, ax1 = plt.subplots(figsize=(15, 6))
ax1.set_title('Candlestick Chart with Bollinger Bands')
candlestick_ohlc(ax1, ohlc_data.values, width=0.6, colorup='g', colordown='r', alpha=0.8)
ax1.plot(indicators_df['date'], indicators_df['BB_Mid'], label='BB Mid', color='blue', linestyle='--')
ax1.plot(indicators_df['date'], indicators_df['BB_Upper'], label='BB Upper', color='purple', linestyle='--')
ax1.plot(indicators_df['date'], indicators_df['BB_Lower'], label='BB Lower', color='purple', linestyle='--')
ax1.legend()
ax1.grid(True)
ax1.xaxis.set_major_formatter(ticker.FuncFormatter(format_date))
ax1.tick_params(axis='x', rotation=45)
fig1.tight_layout()
fig1.savefig('candlestick_bollinger.png')
plt.close(fig1)
 
fig2, ax2 = plt.subplots(figsize=(15, 6))
ax2.set_title('Technical Indicators')
ax2.plot(indicators_df['date'], indicators_df['SMA'], label='SMA', color='orange')
ax2.plot(indicators_df['date'], indicators_df['EMA'], label='EMA', color='green')
ax2.plot(indicators_df['date'], indicators_df['RSI'], label='RSI', color='red')
ax2.plot(indicators_df['date'], indicators_df['MACD'], label='MACD', color='blue')
ax2.plot(indicators_df['date'], indicators_df['Signal'], label='Signal Line', color='pink')
ax2.bar(indicators_df['date'], indicators_df['Histogram'], label='MACD Histogram', color='grey', width=0.8, alpha=0.5)
ax2.plot(indicators_df['date'], indicators_df['Stoch_K'], label='Stochastic %K', color='purple')
ax2.plot(indicators_df['date'], indicators_df['Stoch_D'], label='Stochastic %D', color='brown')
ax2.legend()
ax2.grid(True)
ax2.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d'))
ax2.tick_params(axis='x', rotation=45)
fig2.tight_layout()
fig2.savefig('technical_indicators.png')
plt.close(fig2)
 
fig3, ax3 = plt.subplots(figsize=(15, 6))
ax3.set_title('Portfolio Value and Actions')
ax3.plot(portfolio_df['date'], portfolio_df['portfolio_value'], label='Portfolio Value', color='green')
buy_signals = portfolio_df[portfolio_df['action'] == 'BUY']
sell_signals = portfolio_df[portfolio_df['action'] == 'SELL']
ax3.scatter(buy_signals['date'], buy_signals['price'], label='Buy Signal', color='blue', marker='^', alpha=1)
ax3.scatter(sell_signals['date'], sell_signals['price'], label='Sell Signal', color='red', marker='v', alpha=1)
ax3.legend()
ax3.grid(True)
ax3.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d'))
ax3.tick_params(axis='x', rotation=45)
fig3.tight_layout()
fig3.savefig('portfolio_actions.png')
plt.close(fig3)
 
print('candlestick_bollinger.png saved')
print('technical_indicators.png saved')
print('portfolio_actions.png saved')
 
