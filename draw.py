from matplotlib import pyplot as plt
import pandas as pd

green = pd.read_csv("green.csv", header=None)
pthread = pd.read_csv("pthread.csv", header=None)

plt.plot(green[0], green[1], marker="*", color="g", label="green")
plt.plot(green[0], pthread[1], marker ="d", color="b", label="pthread")
plt.legend()
plt.xscale("log", base=2)
plt.savefig("result.eps")
