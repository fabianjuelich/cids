import matplotlib.pyplot as plt

# Data extracted from the given CSV
duration = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14]
throughput = [0, 38.1738, 50.8314, 57.2725, 61.0639, 63.6762, 65.5033,
              66.8426, 67.8763, 68.7719, 69.4757, 70.0503, 70.5129, 70.9752]

# Create the plot
plt.figure(figsize=(8, 5))
plt.plot(duration, throughput, marker='o', linestyle='-', color='blue')

# Adding titles and labels
plt.title("Throughput vs Simulation Runtime")
plt.xlabel("Simulation Runtime (seconds)")
plt.ylabel("Throughput (Mbps)")
plt.grid(True)

# Display the plot
plt.show()
