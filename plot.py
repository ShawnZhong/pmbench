#!/usr/bin/env python3
import argparse
import os
import re
import json

import pandas as pd
import matplotlib.pyplot as plt


def parse(name, i):
    return re.split("[/:]", name)[i]


def read_data(file):
    with open(file, "r") as f:
        json_data = json.load(f)
        data = pd.DataFrame.from_dict(json_data["benchmarks"])
        data["group"] = data["name"].apply(parse, args=(1,)).apply(int)
        data["benchmark"] = data["name"].apply(parse, args=(0,))
        data["x"] = data["name"].apply(parse, args=(-1,))
        data["y"] = data["items_per_second"]
        return data


def plot_groups(
        label_groups,
        logx=False,
        logy=False,
        xlabel=None,
        ylabel=None,
        title=None,
        output_path=None,
):
    plt.clf()
    for label, group in label_groups:
        plt.plot(group["x"], group["y"], label=label, marker=".")

    if logx:
        plt.xscale("log")
    if logy:
        plt.yscale("log")

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)

    if len(label_groups) > 1:
        plt.legend()

    if output_path:
        plt.savefig(output_path, bbox_inches="tight")

    plt.show()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(dest="file", help="path to benchmark csv")
    args = parser.parse_args()

    data = read_data(args.file)
    for benchmark_name, benchmark in data.groupby("benchmark"):
        output_path = os.path.splitext(args.file)[0] + "-" + benchmark_name + ".pdf"
        plot_groups(
            benchmark.groupby("group"),
            title=benchmark_name,
            xlabel="Number of threads",
            ylabel="Throughput (items per sec)",
            output_path=output_path,
        )


if __name__ == "__main__":
    main()
