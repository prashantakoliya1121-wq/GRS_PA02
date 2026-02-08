#!/bin/bash
# ==========================================================
# MT25034 - PA02 Automated Experiment Script
# ==========================================================

set -euo pipefail

# -------------------------------
# Configuration
# -------------------------------
MESSAGE_SIZES=(128 512 1024 4096)
THREAD_COUNTS=(1 2 4 8)
DURATION=10

PORT_TWO_COPY=9000
PORT_ONE_COPY=9001
PORT_ZERO_COPY=9002

# Executables
A1_SERVER="./MT25034_Part_A1_Server"
A1_CLIENT="./MT25034_Part_A1_Client"

A2_SERVER="./MT25034_Part_A2_Server"
A2_CLIENT="./MT25034_Part_A2_Client"

A3_SERVER="./MT25034_Part_A3_Server"
A3_CLIENT="./MT25034_Part_A3_Client"

# -------------------------------
# Cleanup function
# -------------------------------
cleanup() {
    echo "[CLEANUP] Killing leftover servers..."
    pkill -f MT25034_Part_A1_Server 2>/dev/null || true
    pkill -f MT25034_Part_A2_Server 2>/dev/null || true
    pkill -f MT25034_Part_A3_Server 2>/dev/null || true
    sleep 1
}

trap cleanup EXIT

# -------------------------------
# Initial cleanup
# -------------------------------
cleanup

# -------------------------------
# Compile
# -------------------------------
echo "[BUILD] Compiling all implementations..."

gcc -pthread -O2 -o MT25034_Part_A1_Server MT25034_Part_A1_Server.c
gcc -pthread -O2 -o MT25034_Part_A1_Client MT25034_Part_A1_Client.c

gcc -pthread -O2 -o MT25034_Part_A2_Server MT25034_Part_A2_Server.c
gcc -pthread -O2 -o MT25034_Part_A2_Client MT25034_Part_A2_Client.c

gcc -pthread -O2 -o MT25034_Part_A3_Server MT25034_Part_A3_Server.c
gcc -pthread -O2 -o MT25034_Part_A3_Client MT25034_Part_A3_Client.c

echo "[BUILD] Compilation complete."

# -------------------------------
# Initialize combined CSV file
# -------------------------------
COMBINED_CSV="Combined_Results.csv"
echo "Label,Message_Size,Threads,Duration_s,Time_Elapsed_s,Bytes_Sent,Throughput_Gbps,Latency_us,Cycles,Instructions,Cache_Misses,Context_Switches" > "$COMBINED_CSV"

# -------------------------------
# Parse perf output and append to CSV
# -------------------------------
append_to_csv() {
    LABEL=$1
    MSG_SIZE=$2
    THREADS=$3
    PERF_FILE=$4
    DURATION_S=$5

    # Extract metrics from perf output
    CYCLES=$(awk '/cycles/{print $1; exit}' "$PERF_FILE" | tr -d ',' || true)
    INSTRUCTIONS=$(awk '/instructions/{print $1; exit}' "$PERF_FILE" | tr -d ',' || true)
    CACHE_MISSES=$(awk '/cache-misses/{print $1; exit}' "$PERF_FILE" | tr -d ',' || true)
    CONTEXT_SWITCHES=$(awk '/context-switches/{print $1; exit}' "$PERF_FILE" | tr -d ',' || true)
    TIME_ELAPSED=$(awk '/seconds time elapsed/{print $1; exit}' "$PERF_FILE" | tr -d ',' || true)

    CYCLES=${CYCLES:-0}
    INSTRUCTIONS=${INSTRUCTIONS:-0}
    CACHE_MISSES=${CACHE_MISSES:-0}
    CONTEXT_SWITCHES=${CONTEXT_SWITCHES:-0}
    TIME_ELAPSED=${TIME_ELAPSED:-0}

    BYTES_SENT=$((MSG_SIZE * THREADS * DURATION_S))
    THROUGHPUT_GBPS=$(awk -v bytes="$BYTES_SENT" -v t="$TIME_ELAPSED" 'BEGIN{if (t>0) printf "%.6f", (bytes*8)/(t*1e9); else printf "0"}')
    LATENCY_US=$(awk -v t="$TIME_ELAPSED" -v thr="$THREADS" -v dur="$DURATION_S" 'BEGIN{if (t>0 && thr>0 && dur>0) printf "%.3f", (t*1e6)/(thr*dur); else printf "0"}')

    # Append to combined CSV
    echo "$LABEL,$MSG_SIZE,$THREADS,$DURATION_S,$TIME_ELAPSED,$BYTES_SENT,$THROUGHPUT_GBPS,$LATENCY_US,$CYCLES,$INSTRUCTIONS,$CACHE_MISSES,$CONTEXT_SWITCHES" >> "$COMBINED_CSV"
}

# -------------------------------
# Run one experiment
# -------------------------------
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid 2>/dev/null || true
echo 0 | sudo tee /proc/sys/kernel/kptr_restrict 2>/dev/null || true
run_experiment() {
    SERVER=$1
    CLIENT=$2
    PORT=$3
    LABEL=$4
    MSG_SIZE=$5
    THREADS=$6

    echo
    echo "[RUN] $LABEL | MSG_SIZE=$MSG_SIZE | THREADS=$THREADS"

    # Start server
    $SERVER $PORT $MSG_SIZE &
    SERVER_PID=$!
    sleep 1

    # Run client with perf (use a temp file and delete it after parsing)
    PERF_FILE=$(mktemp)
    perf stat \
        -e cycles,instructions,cache-misses,context-switches \
        -o "$PERF_FILE" \
        $CLIENT 127.0.0.1 $PORT $THREADS $MSG_SIZE $DURATION

    # Stop server safely
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID"
        wait "$SERVER_PID" 2>/dev/null || true
    fi

    # Append results to combined CSV
    append_to_csv "$LABEL" "$MSG_SIZE" "$THREADS" "$PERF_FILE" "$DURATION"
    rm -f "$PERF_FILE"

    echo "[DONE] $LABEL | MSG_SIZE=$MSG_SIZE | THREADS=$THREADS"
}

# -------------------------------
# Main experiment loop
# -------------------------------
for MSG_SIZE in "${MESSAGE_SIZES[@]}"; do
    for THREADS in "${THREAD_COUNTS[@]}"; do

        run_experiment "$A1_SERVER" "$A1_CLIENT" \
            "$PORT_TWO_COPY" "TwoCopy" "$MSG_SIZE" "$THREADS"

        run_experiment "$A2_SERVER" "$A2_CLIENT" \
            "$PORT_ONE_COPY" "OneCopy" "$MSG_SIZE" "$THREADS"

        run_experiment "$A3_SERVER" "$A3_CLIENT" \
            "$PORT_ZERO_COPY" "ZeroCopy" "$MSG_SIZE" "$THREADS"

    done
done

echo
echo "[SUCCESS] All PA02 experiments completed."
