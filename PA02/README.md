# PA02 Assignment
# Roll Number: MT25034

## Overview
This project implements and compares three socket communication mechanisms:
- Two-Copy
- One-Copy
- Zero-Copy

## Files
- **Source Code**:
  - `MT25034_Part_A1_Server.c` and `MT25034_Part_A1_Client.c`: Two-Copy implementation.
  - `MT25034_Part_A2_Server.c` and `MT25034_Part_A2_Client.c`: One-Copy implementation.
  - `MT25034_Part_A3_Server.c` and `MT25034_Part_A3_Client.c`: Zero-Copy implementation.
- **Experiment Script**:
  - `MT25034_Part_C_RunExperiments.sh`: Automates experiments and collects results.
- **Plots**:
  - Python scripts for plotting throughput, latency, cache misses, and CPU cycles.
  - Plotting scripts use hardcoded arrays (no CSV input).
- **Report**:
  - `MT25034_Part_E_Report.md`: Technical report.

## Build Instructions
1. Run `make` to compile all programs.
2. Use `make clean` to remove compiled binaries.

## Run Instructions
1. Start the server:
  ```bash
  ./MT25034_Part_A1_Server
  ```
2. Start the client:
  ```bash
  ./MT25034_Part_A1_Client
  ```
3. Repeat for other implementations.

## Automated Experiments
Run the experiment script:
```bash
bash MT25034_Part_C_RunExperiments.sh
```

## Plots
Generate plots using the Python scripts (hardcoded data, no CSV required):
```bash
python3 MT25034_Part_D_Plot_Throughput.py
python3 MT25034_Part_D_Plot_Latency.py
python3 MT25034_Part_D_Plot_CacheMisses.py
python3 MT25034_Part_D_Plot_CPUCyclesPerByte.py
```

### Plot Outputs
- **Throughput vs Message Size** (separate per thread):
  - `Throughput_vs_Message_Size_T1.png`
  - `Throughput_vs_Message_Size_T2.png`
  - `Throughput_vs_Message_Size_T4.png`
  - `Throughput_vs_Message_Size_T8.png`
- **Latency vs Thread Count** (separate per message size):
  - `Latency_vs_Thread_Count_MSG128.png`
  - `Latency_vs_Thread_Count_MSG512.png`
  - `Latency_vs_Thread_Count_MSG1024.png`
  - `Latency_vs_Thread_Count_MSG4096.png`
- **Cache Misses vs Message Size** (separate per thread):
  - `Cache_Misses_vs_Message_Size_T1.png`
  - `Cache_Misses_vs_Message_Size_T2.png`
  - `Cache_Misses_vs_Message_Size_T4.png`
  - `Cache_Misses_vs_Message_Size_T8.png`
- **CPU Cycles per Byte vs Message Size** (separate per thread):
  - `CPU_Cycles_per_Byte_vs_Message_Size_T1.png`
  - `CPU_Cycles_per_Byte_vs_Message_Size_T2.png`
  - `CPU_Cycles_per_Byte_vs_Message_Size_T4.png`
  - `CPU_Cycles_per_Byte_vs_Message_Size_T8.png`

## Clean Up
To clean up the directory:
```bash
make clean
```