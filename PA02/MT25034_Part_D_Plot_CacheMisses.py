import os
import platform
import matplotlib.pyplot as plt

# Hardcoded data from Combined_Results.csv for all thread counts
THREAD_COUNTS = [1, 2, 4, 8]
MESSAGE_SIZES = [128, 512, 1024, 4096]
CACHE_MISSES_BY_LABEL_THREAD = {
	"TwoCopy": {
		1: [107219.0, 106700.0, 97806.0, 102821.0],
		2: [346419.0, 217883.0, 229812.0, 141078.0],
		4: [238685.0, 269616.0, 289266.0, 385535.0],
		8: [286539.0, 208377.0, 250563.0, 238552.0],
	},
	"OneCopy": {
		1: [115070.0, 115127.0, 77705.0, 106093.0],
		2: [148972.0, 232640.0, 167781.0, 147285.0],
		4: [205132.0, 255603.0, 252697.0, 322010.0],
		8: [212370.0, 223984.0, 200958.0, 201646.0],
	},
	"ZeroCopy": {
		1: [141916.0, 109552.0, 98646.0, 100333.0],
		2: [150452.0, 209518.0, 178890.0, 164661.0],
		4: [255476.0, 227507.0, 240896.0, 314653.0],
		8: [243738.0, 245050.0, 213396.0, 217562.0],
	},
}


def system_info():
	cpu_model = "Unknown CPU"
	try:
		with open("/proc/cpuinfo", "r", encoding="utf-8") as f:
			for line in f:
				if "model name" in line:
					cpu_model = line.strip().split(":", 1)[1].strip()
					break
	except OSError:
		pass
	return f"System: {platform.platform()} | CPU: {cpu_model} | Cores: {os.cpu_count()}"


labels = ["TwoCopy", "OneCopy", "ZeroCopy"]
style = {
	"TwoCopy": {"color": "#1f77b4", "marker": "o"},
	"OneCopy": {"color": "#2ca02c", "marker": "s"},
	"ZeroCopy": {"color": "#d62728", "marker": "^"},
}
thread_linestyle = {
	1: "-",
	2: "--",
	4: "-.",
	8: ":",
}

plt.figure(figsize=(10, 6))
for threads in THREAD_COUNTS:
	plt.figure(figsize=(10, 6))
	for label in labels:
		message_sizes = MESSAGE_SIZES
		cache_misses = CACHE_MISSES_BY_LABEL_THREAD[label][threads]
		plt.plot(
			message_sizes,
			cache_misses,
			label=f"{label}",
			color=style[label]["color"],
			marker=style[label]["marker"],
			linestyle=thread_linestyle[threads],
			linewidth=2.0,
			markersize=6,
			alpha=0.95,
			zorder=3,
		)

	plt.title(f"Cache Misses vs Message Size (Threads = {threads})")
	plt.xlabel("Message Size (bytes)")
	plt.ylabel("Cache Misses")
	plt.xscale("log")
	plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.6)
	plt.legend(loc="best", frameon=True, fontsize=9)
	plt.figtext(0.5, 0.01, system_info(), ha="center", fontsize=8)
	plt.tight_layout(rect=[0, 0.03, 1, 1])
	plt.savefig(f"Cache_Misses_vs_Message_Size_T{threads}.png")
	plt.close()