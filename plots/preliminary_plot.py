import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('../preliminary_results.csv')

propagation_model = df['propagationModel'][0]
distance = df['distance'][0]
duration = df['duration']
throughput = df['throughput']

plt.figure()
plt.plot(duration, throughput, marker='o')

plt.suptitle("Preliminary simulation experiment")
plt.title(f"Propagation model: {propagation_model}, Distance: {distance}m", fontsize=10)
plt.xlabel("Simulation runtime (seconds)")
plt.ylabel("Throughput (Mbps)")
plt.grid(True)

plt.savefig('preliminary_results.png')
