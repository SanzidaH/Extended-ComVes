from __future__ import division
import csv
import sys
from collections import defaultdict
import numpy as np
import matplotlib.pyplot as plt
import glob


def main(filepath=None, traceID=None, maxSuccess=None):
    count = []
    node_count = 0
    latency_sum = []
    tp = []
    sent = []
    nodes = {}
    for i in range(0, 5000):  # initialization of the lists
        tp.append(0)
        count.append(0)
        latency_sum.append(0)
        sent.append(0)
    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    filesToProcess = glob.glob(
        filepath + "*"
    )  # search for files that's used to search for files that match specific file pattern or name
    print(filesToProcess)
    # f2 = open(traceID + "-Average_Delay-.txt","w")
    # f3 = open(traceID + "-Success_Ratio-.txt","w")
    for filename in filesToProcess:
        array = filename.split("-")
        # f3 = open(traceID + "-" + array[2] + "-" + str(array[3])[:-4] + "-Server.txt","w")
        dicto = dict.fromkeys(range(100), 0)
        dictp = dict.fromkeys(range(100), 0)
        # dicto = dict(zip(range(10), repeat(0)))
        # dictp = dict(zip(range(10), repeat(0)))
        print("Trying :" + filename)
        with open(filename, "r") as csvh:
            # dialect = csv.Sniffer().sniff(csvh.read(10*1024))
            dialect = csv.Sniffer().has_header(csvh.read(1024))
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)
            for row in reader:  # nodeid, name, servercount,status
               # print(row["status"])
               # print(row["nodeid"])
                if str(row["status"]) == "OVERLOADED":
                    dicto[int(row["nodeid"])] = int(dicto[int(row["nodeid"])]) + int(1)
                elif str(row["status"]) == "Data Processing":
                    dictp[int(row["nodeid"])] = int(dictp[int(row["nodeid"])]) + int(1)
            list1 = []
            list2 = []
            for (k, v), (k2, v2) in zip(dicto.items(), dictp.items()):
                if str(v) == "0" and str(v2) == "0":
                    list1.append(k)
                    list2.append(k2)
            for i in list1:
                del dicto[i]
            for i in list1:
                del dictp[i]

            freq = str(array[3])
            sorted_dict1 = dict(sorted(dicto.items()))
            sorted_dict2 = dict(sorted(dictp.items()))
           # for k, v in sorted_dict1.items():
            #    print(k, v)
           # for k, v in sorted_dict2.items():
            #    print(k, v)
            X1 = list(sorted_dict1.keys())
            Y1 = list(sorted_dict1.values())
            X2 = list(sorted_dict2.keys())
            Y2 = list(sorted_dict2.values())

            # plot bars in stack manner
            plt.bar(X1, Y1, color="y")
            plt.bar(X1, Y2, bottom=Y1, color="g")
            plt.xlabel("Server")
            plt.ylabel("Number Of Task")
            plt.legend(["OVERLOADED", "DATA PROCESSED"])
            plt.title("Number of Task per Server")
            # plt.legend()
            arr = filename.split("-")
            plt.savefig(traceID + "-fig-" + arr[2] + "-" + arr[3] + "-Server.png")
            X1.clear()
            X2.clear()
            Y1.clear()
            Y2.clear()
            list1.clear()
            list2.clear()
            dicto.clear()
            dictp.clear()
            sorted_dict1.clear()
            sorted_dict2.clear()
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
