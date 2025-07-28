import pandas as pd
import matplotlib.pyplot as plt

# Path to your metrics file
filepath = 'Output/metrics_output.csv'

# Load the data
df = pd.read_csv(filepath)

# Convert 'MidPrice' and 'Spread' to rupees if in paise
df['MidPrice'] = df['MidPrice'] / 100.0
df['Spread'] = df['Spread'] / 100.0

# Ensure 'TimestampRaw' is interpreted as a number
df['TimestampRaw'] = pd.to_numeric(df['TimestampRaw'], errors='coerce')

plt.figure(figsize=(12, 10))

plt.subplot(4, 1, 1)
plt.plot(df['TimestampRaw'], df['MidPrice'], color='blue')
plt.ylabel('Mid Price (INR)')
plt.title('Mid Price vs Raw Timestamp')
plt.grid(True, linestyle='--', alpha=0.6)
plt.ylim(420, 500)

plt.subplot(4, 1, 2)
plt.plot(df['TimestampRaw'], df['Spread'], color='orange')
plt.ylabel('Spread (INR)')
plt.title('Spread vs Raw Timestamp')
plt.grid(True, linestyle='--', alpha=0.6)

plt.subplot(4, 1, 3)
plt.plot(df['TimestampRaw'], df['OFI_Top'], color='green')
plt.ylabel('OFI_Top')
plt.title('Order Flow Imbalance (Top) vs Raw Timestamp')
plt.grid(True, linestyle='--', alpha=0.6)

plt.subplot(4, 1, 4)
plt.plot(df['TimestampRaw'], df['OFI_Depth'], color='purple')
plt.ylabel('OFI_Depth')
plt.title('Order Flow Imbalance (Depth) vs Raw Timestamp')
plt.xlabel('Raw Timestamp (nanoseconds)')
plt.grid(True, linestyle='--', alpha=0.6)

plt.tight_layout()
plt.savefig('Output/lob_metrics_vs_rawtimestamp.png', dpi=150)
plt.show()
