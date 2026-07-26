#!/usr/bin/env python3
"""
Plots deviation % vs set rate from constant_rate_accuracy.csv

Usage:
    python3 plot_const_accuracy.py constant_rate_accuracy.csv
Outputs:
    constant_rate_accuracy.png
"""

import sys
import csv

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_const_accuracy.py <csv_file>")
        sys.exit(1)

    rates = []
    deviations = []
    with open(sys.argv[1], "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rates.append(int(row["Set Rate (mL/hr)"]))
            deviations.append(float(row["Deviation (%)"]))

    try:
        import matplotlib.pyplot as plt
        plt.figure(figsize=(8, 4))
        bars = plt.bar([str(r) for r in rates], deviations, color="#4C72B0")
        plt.axhline(y=5, color="red", linestyle="--", label="±5% spec limit")
        plt.xlabel("Set Rate (mL/hr)")
        plt.ylabel("Deviation (%)")
        plt.title("Constant Mode - Flow Accuracy vs Set Rate")
        plt.ylim(0, max(6, max(deviations) + 2))
        plt.legend()
        plt.grid(True, axis="y")
        plt.tight_layout()
        plt.savefig("constant_rate_accuracy.png", dpi=150)
        print("Wrote constant_rate_accuracy.png")
    except ImportError:
        print("matplotlib not installed. Run: pip install matplotlib --break-system-packages")

if __name__ == "__main__":
    main()

