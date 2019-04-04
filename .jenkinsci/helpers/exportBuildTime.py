#!/usr/bin/env python
#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

#
# Compares 2 csv files with build time data, makes new file with difference in seconds and percents
#

import re
import argparse
import csv

parser = argparse.ArgumentParser(description='Process time output for compiler')

parser.add_argument('log_file', type=str, help='input file')

args = parser.parse_args()
if __name__ == "__main__":
    lines = []
    with open(args.log_file, "r") as f:
        lines = f.readlines()
    i = 0
    units = []
    while i < len(lines):
        unit = {}
        if "g++" not in lines[i]:
            i+=1
            continue
        try:
            sys_time, user_time = lines[i+1].rstrip().split("\t")
            unit['target'] = re.findall(r"-o (\S+) ", lines[i])[0]
            unit['work_type'] = 'linking' if "-Wl" in lines[i] else "build"
            unit['sys_time'] = float(sys_time)
            unit['user_time'] = float(user_time)
            unit['total'] = round(unit['sys_time'] + unit['user_time'], 2)
            units.append(unit)
            i+=2
        except:
            i+=1
            continue

    csv_filename = args.log_file.split(".")[0] + ".csv"
    with open(csv_filename, 'w') as csvfile:
        fieldnames = ['target', 'sys_time', 'user_time', 'total', 'work_type']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(units)
