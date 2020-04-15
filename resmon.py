"""
    Simple resources monitoring script

    Author: L1ttl3S1st3r
    Date: 14 April 2020
"""
from psutil import cpu_percent
from psutil import cpu_count
from psutil import virtual_memory


OUTPUT_DELAY = 0.4


def format_columns(columns):
    formatted_columns = ["{0:^23}".format(column) for column in columns]
    return '|'.join(formatted_columns)


def resources_header():
    columns = [f'cpu {number+1} usage (%)' for number in range(cpu_count())]
    columns += ['memory occupied (%)']

    return format_columns(columns)


def resourses_row():
    columns = cpu_percent(interval=OUTPUT_DELAY, percpu=True)
    columns += [virtual_memory().percent]

    return format_columns(columns)


if __name__ == '__main__':
    print(resources_header())
    while True:
        print(resourses_row())
