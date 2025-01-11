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

# Plot 1: Throughput vs Simulation time
plt.figure()
duration = pre_results['duration']
throughput = pre_results['throughput']
plt.plot(duration, throughput, marker='o', color='orange')

plt.title(f"UDP Throughput vs Distance for {propagation_model} over {distance}m")
plt.xlabel("Simulation Runtime (seconds)")
plt.ylabel("Throughput (Mbps)")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, 'throughput_vs_simulation_time.png'))

# Plot 2: Signal Strength vs Distance
plt.figure()
for model, results in grouped_results:
    plt.plot(results["distance"], results["signalStrength"], label=model, marker='o')

plt.title("Signal Strength vs Distance for Different Propagation Models")
plt.xlabel("Distance (m)")
plt.ylabel("Signal Strength (dBm)")
plt.legend(title="Propagation Model")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, "signal_strength_vs_distance.png"))

# Plot 3: Throughput vs Distance
plt.figure()
for model, results in grouped_results:
    plt.plot(results["distance"], results["throughput"], label=model, marker='o')

plt.title("UDP Throughput vs Distance for Different Propagation Models")
plt.xlabel("Distance (m)")
plt.ylabel("Throughput (Mbps)")
plt.legend(title="Propagation Model")
plt.grid(True)
plt.savefig(os.path.join(PLOTS_DIR, "throughput_vs_distance.png"))
