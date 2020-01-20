#!/usr/bin/env python3

import glob
import matplotlib.pyplot as plt
import pandas as pd
import re
import sys

benchmarks = []

if(len(sys.argv) != 1):
    exit("Call without arguments in a folder containing *-PQP.svg files")

all_operator_breakdowns = {}
for file in sorted(glob.glob('*-PQP.svg')):
    operator_breakdown = {}
    with open(file, 'r') as svg:
        svg_string = svg.read().replace("\n", '|')
        table_string = re.findall(r'Total by operator(.*?)</g>', svg_string)[0]
        table_string = re.sub(r'<.*?>', '', table_string)
        table_string = re.sub(r'^\|*', '', table_string)
        table_string = re.sub(r'\|*$', '', table_string)
        row_strings = table_string.split('||')

        operator_names = []
        operator_durations = []

        for operator_name in row_strings[0].split('|'):
            operator_names.append(operator_name.strip())

        for operator_duration_str in row_strings[1].split('|'):
            operator_duration_str = operator_duration_str.replace(' min ', ' * 60 * 1e9 + ')
            operator_duration_str = operator_duration_str.replace(' s ', ' * 1e9 + ')
            operator_duration_str = operator_duration_str.replace(' ms ', ' * 1e6 + ')
            operator_duration_str = operator_duration_str.replace(' µs ', ' * 1e3 + ')
            operator_duration_str = operator_duration_str.replace(' ns ', ' + ')
            operator_duration_str += '0'
            operator_duration = float(eval(operator_duration_str))
            operator_durations.append(operator_duration)

        operator_breakdown = dict(zip(operator_names, operator_durations))
        del operator_breakdown['total']

    all_operator_breakdowns[file.replace('-PQP.svg', '')] = operator_breakdown

df = pd.DataFrame(all_operator_breakdowns).transpose() # TODO sort by operator name

# Normalize data from nanoseconds to percentage of total cost
df.iloc[:,0:] = df.iloc[:,0:].apply(lambda x: x / x.sum(), axis=1)
print(df)

ax = df.plot.bar(stacked=True)
ax.set_yticklabels(['{:,.0%}'.format(x) for x in ax.get_yticks()])
ax.set_ylabel('Share of query run time')

# Reverse legend so that it matches the stacked bars
handles, labels = ax.get_legend_handles_labels()
ax.legend(reversed(handles), reversed(labels), bbox_to_anchor=(1.0, 1.0))

plt.tight_layout()
plt.savefig('operator_breakdown.png')

# TODO filter operators that take less <1%
# TODO Sum-up absolute (1 execution per item) and geomean