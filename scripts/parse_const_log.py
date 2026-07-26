#!/usr/bin/env python3
"""
Parses infusion pump serial log for ConstantRateMode [STATUS] lines,
groups by set rate, and outputs an accuracy table.

Usage:
    python3 parse_const_log.py const_log.txt
Outputs:
    constant_rate_accuracy.csv
"""

import sys
import csv
import re

def parse_log(filepath):
    pattern = re.compile(
        r"\[STATUS\] Rate:(\d+) mL/hr \| Delivered:([\d.]+) mL \| Expected:([\d.]+) mL \| Dev:(\d+)%"
    )
    rows = []
    with open(filepath, "r", errors="ignore") as f:
        for line in f:
            m = pattern.search(line)
            if m:
                rate = int(m.group(1))
                delivered = float(m.group(2))
                expected = float(m.group(3))
                dev = int(m.group(4))
                rows.append((rate, delivered, expected, dev))
    return rows

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 parse_const_log.py <logfile>")
        sys.exit(1)

    rows = parse_log(sys.argv[1])
    if not rows:
        print("No STATUS lines found. Check log format.")
        sys.exit(1)

    last_per_rate = {}
    for rate, delivered, expected, dev in rows:
        last_per_rate[rate] = (delivered, expected, dev)

    csv_path = "constant_rate_accuracy.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Set Rate (mL/hr)", "Delivered (mL)", "Expected (mL)", "Deviation (%)", "Within +/-5%"])
        for rate in sorted(last_per_rate.keys()):
            delivered, expected, dev = last_per_rate[rate]
            within = "YES" if dev <= 5 else "NO"
            writer.writerow([rate, delivered, expected, dev, within])

    print(f"Wrote {csv_path} with {len(last_per_rate)} rate entries")

if __name__ == "__main__":
    main()
