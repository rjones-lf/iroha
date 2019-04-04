#!/usr/bin/env python
#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

#
# Analyzes build time per each iroha build target (g++ target) and stores all results to csv-file:
# target, work_type, sys_time, user_time, total, sys_time_diff, user_time_diff, total_diff, sys_time_perc, user_time_perc, total_perc
#

import csv
from sys import argv
from os import path

F1, F2 = argv[1], argv[2] # base, compare

# time difference in seconds (multiply by -1 show difference - faster or lower)
def diff_secs(a, b):
    return round( (float(a) - float(b)) * -1 , 2)

# time difference in percents (multiply by -1 show difference - faster or lower)
def diff_perc(a, b):
    if float(a) == 0.0:
        return 0.0
    return round( (float(a) - float(b)) / float(a) * 100 * -1, 2)

if __name__ == '__main__':
    if not path.isfile(F1) or not path.isfile(F2):
        print("Can't find files!")
        exit(1)

    with open(F1) as base, open(F2) as compare:
        base_reader, comp_reader = csv.DictReader(base), csv.DictReader(compare)
        b, c = { row['target']: row for row in base_reader }, { row['target']: row for row in comp_reader }
    
    if len(c) == 0:
        print("No records found")
        exit(1)

    for target in c:
        c[target].update( 
            { 'user_time_diff': 0, 'sys_time_diff': 0, 'total_diff': 0, 
              'user_time_perc': 0, 'sys_time_perc': 0, 'total_perc': 0 } 
        )
        if b[target] is None:
            continue
        c[target]['sys_time_diff']  = diff_secs(b[target]['sys_time'], c[target]['sys_time'])
        c[target]['user_time_diff'] = diff_secs(b[target]['user_time'], c[target]['user_time'])
        c[target]['total_diff']     = diff_secs(b[target]['total'],  c[target]['total'])
        c[target]['sys_time_perc']  = diff_perc(b[target]['sys_time'], c[target]['sys_time'])
        c[target]['user_time_perc'] = diff_perc(b[target]['user_time'], c[target]['user_time'])
        c[target]['total_perc']     = diff_perc(b[target]['total'],  c[target]['total'])

    with open ('diff.csv', 'w+') as csvfile:
        fieldnames = ['target', 'work_type', 
            'sys_time', 'user_time', 'total', 
            'sys_time_diff', 'user_time_diff', 'total_diff', 
            'sys_time_perc', 'user_time_perc', 'total_perc'
        ]
        # fieldnames = sorted(next(iter(c.iteritems()))[1].keys())
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for row in c:
            writer.writerow(c[row])
