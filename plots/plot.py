import pandas as pd
import matplotlib.pyplot as plt
import os

RESULTS_DIR = os.path.join(os.path.dirname(__file__), '../results')
PLOTS_DIR = os.path.join(os.path.dirname(__file__))

# Read the CSV data
pre_results = pd.read_csv('results/pre_results.csv')
results = pd.read_csv("results/results.csv")

# Get propagation model and distance used in preliminary experiment
propagation_model = pre_results['propagationModel'][0]
distance = pre_results['distance'][0]

# Group results by propagation model
grouped_results = results.groupby("propagationModel")

# Plot 1: Runtime vs Throughput
plt.figure()
duration = pre_results['duration']
throughput = pre_results['throughput']
plt.plot(duration, throughput, color='orange')

# plt.title(f"Runtime vs. UDP Throughput with {propagation_model} over {distance} m")
plt.xlabel("Runtime (seconds)")
plt.ylabel("Throughput (Mbps)")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, 'runtime_vs_throughput.png'))

# Plot 2: Distance vs Signal Strength
plt.figure()
for model, results in grouped_results:
    plt.plot(results["distance"], results["signalStrength"], label=model, marker='o')

# plt.title("Distance vs. Signal Strength for different propagation models")
plt.xlabel("Distance (m)")
plt.ylabel("Signal Strength (dBm)")
plt.legend(title="Propagation Model")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, "distance_vs_signal_strength.png"))

# Plot 3: ThroughpuDistance vs Throughput
plt.figure()
for model, results in grouped_results:
    plt.plot(results["distance"], results["throughput"], label=model, marker='o')

# plt.title("Distance vs. UDP Throughput for different propagation models")
plt.xlabel("Distance (m)")
plt.ylabel("Throughput (Mbps)")
plt.legend(title="Propagation Model")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, "distance_vs_throughput.png"))
