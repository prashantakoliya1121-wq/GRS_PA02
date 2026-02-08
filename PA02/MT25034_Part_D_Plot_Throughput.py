import os
import platform
import matplotlib.pyplot as plt

# Hardcoded data from Combined_Results.csv for all thread counts
THREAD_COUNTS = [1, 2, 4, 8]
MESSAGE_SIZES = [128, 512, 1024, 4096]
THROUGHPUT_GBPS_BY_LABEL_THREAD = {
	"TwoCopy": {
		1: [0.000001, 0.000004, 0.000008, 0.000033],
		2: [0.000002, 0.000008, 0.000016, 0.000066],
		4: [0.000004, 0.000016, 0.000033, 0.000131],
		8: [0.000007, 0.000033, 0.000060, 0.000239],
	},
	"OneCopy": {
		1: [0.000001, 0.000004, 0.000008, 0.000033],
		2: [0.000002, 0.000008, 0.000016, 0.000066],
		4: [0.000004, 0.000016, 0.000033, 0.000131],
		8: [0.000008, 0.000033, 0.000060, 0.000239],
	},
	"ZeroCopy": {
		1: [0.000001, 0.000004, 0.000008, 0.000033],
		2: [0.000002, 0.000008, 0.000016, 0.000066],
		4: [0.000004, 0.000016, 0.000033, 0.000131],
		8: [0.000008, 0.000030, 0.000060, 0.000239],
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

for threads in THREAD_COUNTS:
	plt.figure(figsize=(10, 6))
	for label in labels:
		message_sizes = MESSAGE_SIZES
		throughput = THROUGHPUT_GBPS_BY_LABEL_THREAD[label][threads]
		plt.plot(
			message_sizes,
			throughput,
			label=f"{label}",
			color=style[label]["color"],
			marker=style[label]["marker"],
			linestyle=thread_linestyle[threads],
			linewidth=2.0,
			markersize=6,
			alpha=0.95,
			zorder=3,
		)

	plt.title(f"Throughput vs Message Size (Threads = {threads})")
	plt.xlabel("Message Size (bytes)")
	plt.ylabel("Throughput (Gbps)")
	plt.xscale("log")
	plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.6)
	plt.legend(loc="best", frameon=True, fontsize=9)
	plt.figtext(0.5, 0.01, system_info(), ha="center", fontsize=8)
	plt.tight_layout(rect=[0, 0.03, 1, 1])
	plt.savefig(f"Throughput_vs_Message_Size_T{threads}.png")
	plt.close()