import os
import platform
import matplotlib.pyplot as plt

# Hardcoded data from Combined_Results.csv for all thread counts
THREAD_COUNTS = [1, 2, 4, 8]
MESSAGE_SIZES = [128, 512, 1024, 4096]
CYCLES_PER_BYTE_BY_LABEL_THREAD = {
	"TwoCopy": {
		1: [15334655.465625, 3080080.6521484377, 1529750.9689453125, 393239.0876708984],
		2: [9076736.035546875, 2429070.119042969, 1252781.7425292968, 314529.3873657227],
		4: [7653557.527734375, 1926191.2646972656, 965561.8845458984, 245084.02803344728],
		8: [5917618.3640625, 1447180.3382324218, 729572.9404052735, 184931.03284912108],
	},
	"OneCopy": {
		1: [16211882.43125, 3195281.6955078123, 1601059.655078125, 408160.13186035154],
		2: [9875100.371484375, 2563633.9357421873, 1291311.951220703, 317636.4027832031],
		4: [7933292.2783203125, 1983442.1498535157, 993479.5411376953, 250943.77531738282],
		8: [6064362.389257813, 1484344.9203613282, 757194.0787963867, 189021.22482910156],
	},
	"ZeroCopy": {
		1: [15279225.1734375, 3207343.0443359376, 1603676.8663085937, 402516.4403320312],
		2: [10048702.289453125, 2520274.549121094, 1265971.702001953, 324690.3137207031],
		4: [7890813.145898437, 1982327.1212890625, 995552.3055908203, 250693.31728515626],
		8: [5962638.28828125, 1520611.0869384767, 760846.1693481446, 189473.7475402832],
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
		cycles_per_byte = CYCLES_PER_BYTE_BY_LABEL_THREAD[label][threads]
		plt.plot(
			message_sizes,
			cycles_per_byte,
			label=f"{label}",
			color=style[label]["color"],
			marker=style[label]["marker"],
			linestyle=thread_linestyle[threads],
			linewidth=2.0,
			markersize=6,
			alpha=0.95,
			zorder=3,
		)

	plt.title(f"CPU Cycles per Byte vs Message Size (Threads = {threads})")
	plt.xlabel("Message Size (bytes)")
	plt.ylabel("CPU Cycles per Byte")
	plt.xscale("log")
	plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.6)
	plt.legend(loc="best", frameon=True, fontsize=9)
	plt.figtext(0.5, 0.01, system_info(), ha="center", fontsize=8)
	plt.tight_layout(rect=[0, 0.03, 1, 1])
	plt.savefig(f"CPU_Cycles_per_Byte_vs_Message_Size_T{threads}.png")
	plt.close()