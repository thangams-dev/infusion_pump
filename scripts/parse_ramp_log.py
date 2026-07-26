#!/usr/bin/env python3
"""
Parses infusion pump serial log for LinearRampMode data and plots rate vs time.

Expects log lines in the format:
    [DATA] time_ms:<ms> rate:<rate>

Usage:
    python3 parse_ramp_log.py ramp_log.txt
Outputs:
    ramp_profile.csv  - time_s, rate_mlhr columns
    ramp_profile.png  - rate vs time plot
"""

import sys
import csv
import re

def parse_log(filepath):
    pattern = re.compile(r"\[DATA\] time_ms:(\d+) rate:(\d+)")
    rows = []
    with open(filepath, "r", errors="ignore") as f:
        for line in f:
            m = pattern.search(line)
            if m:
                t_ms = int(m.group(1))
                rate = int(m.group(2))
                rows.append((t_ms, rate))
    return rows

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 parse_ramp_log.py <logfile>")
        sys.exit(1)

    rows = parse_log(sys.argv[1])
    if not rows:
        print("No DATA lines found. Check log format matches '[DATA] time_ms:<ms> rate:<rate>'.")
        sys.exit(1)

    t0 = rows[0][0]
    csv_path = "ramp_profile.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time_s", "rate_mlhr"])
        for t_ms, rate in rows:
            writer.writerow([(t_ms - t0) / 1000.0, rate])
    print(f"Wrote {csv_path} ({len(rows)} points)")

    try:
        import matplotlib.pyplot as plt
        times = [(t - t0) / 1000.0 for t, r in rows]
        rates = [r for t, r in rows]
        plt.figure(figsize=(8, 4))
        plt.plot(times, rates, marker="o", markersize=2)
        plt.xlabel("Time (s)")
        plt.ylabel("Rate (mL/hr)")
        plt.title("Linear Ramp Mode - Rate vs Time")
        plt.grid(True)
        plt.tight_layout()
        plt.savefig("ramp_profile.png", dpi=150)
        print("Wrote ramp_profile.png")
    except ImportError:
        print("matplotlib not installed - CSV written, skip plot. Run: pip install matplotlib --break-system-packages")

if __name__ == "__main__":
    main()