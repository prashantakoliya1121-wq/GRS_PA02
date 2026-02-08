import os
import platform
import matplotlib.pyplot as plt

# Hardcoded data from Combined_Results.csv for all message sizes
MESSAGE_SIZES = [128, 512, 1024, 4096]
THREADS = [1, 2, 4, 8]
LATENCY_US_BY_LABEL_MSG = {
	"TwoCopy": {
		128: [966781.678, 498905.83, 249028.484, 137185.773],
		512: [995608.632, 498985.451, 249426.998, 124491.427],
		1024: [995986.857, 499012.047, 249426.582, 137002.518],
		4096: [997431.681, 498606.587, 249406.535, 137164.701],
	},
	"OneCopy": {
		128: [998199.001, 498752.266, 249029.239, 124732.743],
		512: [998064.951, 498771.721, 248952.782, 124635.482],
		1024: [996800.75, 498869.486, 249280.127, 137196.52],
		4096: [997912.541, 498053.331, 249337.548, 137222.225],
	},
	"ZeroCopy": {
		128: [998119.985, 498506.949, 248977.717, 124543.21],
		512: [997973.264, 498901.948, 249103.656, 137118.722],
		1024: [997866.436, 498820.674, 249003.115, 137001.523],
		4096: [997938.78, 498837.172, 249337.812, 137205.775],
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
msg_linestyle = {
	128: "-",
	512: "--",
	1024: "-.",
	4096: ":",
}

plt.figure(figsize=(10, 6))
for msg_size in MESSAGE_SIZES:
	plt.figure(figsize=(10, 6))
	for label in labels:
		threads = THREADS
		latency = LATENCY_US_BY_LABEL_MSG[label][msg_size]
		plt.plot(
			threads,
			latency,
			label=f"{label}",
			color=style[label]["color"],
			marker=style[label]["marker"],
			linestyle=msg_linestyle[msg_size],
			linewidth=2.0,
			markersize=6,
			alpha=0.95,
			zorder=3,
		)

	plt.title(f"Latency vs Thread Count (Message Size = {msg_size} bytes)")
	plt.xlabel("Thread Count")
	plt.ylabel("Latency (µs)")
	plt.grid(True, which="both", linestyle="--", linewidth=0.5, alpha=0.6)
	plt.legend(loc="best", frameon=True, fontsize=9)
	plt.figtext(0.5, 0.01, system_info(), ha="center", fontsize=8)
	plt.tight_layout(rect=[0, 0.03, 1, 1])
	plt.savefig(f"Latency_vs_Thread_Count_MSG{msg_size}.png")
	plt.close()