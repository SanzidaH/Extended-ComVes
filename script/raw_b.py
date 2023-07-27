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
   # barWidth = 0.25
   # fig = plt.subplots(figsize =(12, 8))
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

            freq = str(array[2])
            sorted_dict1 = dict(sorted(dicto.items()))
            sorted_dict2 = dict(sorted(dictp.items()))
           # for k, v in sorted_dict1.items():
            #    print(k, v)
           # for k, v in sorted_dict2.items():
            #    print(k, v)
           # print(array[3])
            if(str(array[3]) == "0"):
                print(array[3])
                X1 = []
                X2 = []
                Y1 = []
                Y2 = []
                X1 = list(sorted_dict1.keys())
                Y1 = list(sorted_dict1.values())
                X2 = list(sorted_dict2.keys())
                Y2 = list(sorted_dict2.values())
               # plt.bar(X1, Y1, color="y")
               # plt.bar(X1, Y2, bottom=Y1, color="g")
#                plt.bar(X2, Y2, color="y")
#                plt.bar(X2, Y3, bottom=Y1, color="g")
              #  plt.xlabel("Server")
              #  plt.ylabel("Number Of Task")
              #  plt.legend(["OVERLOADED", "DATA PROCESSED"])
              #  plt.title("Number of Task per Server")
                # plt.legend()
              #  arr = filename.split("-")
              #  plt.savefig(traceID + "-" + arr[2] + "-" + arr[3] + "-Server-d0.png")
            if(str(array[3]) =="1"):
                print(array[3])
                X3 = []
                X4 = []
                Y3 = []
                Y4 = []
                X3 = list(sorted_dict1.keys())
                Y3 = list(sorted_dict1.values())
                X4 = list(sorted_dict2.keys())
                Y4 = list(sorted_dict2.values())
            # plot bars in stack manner
               # X_axis = np.arange(len(X1))
               # plt.bar(X_axis - 0.2, Y1, 0.4, label = 'ComVes',color="y")                                
               # plt.bar(X_axis - 0.2, Y2, 0.4, label = 'ComVes',color="g", bottom=Y1)
               # plt.bar(X_axis + 0.2, Y3, 0.4, label = 'Proposed Strategy', color="b")
               # plt.bar(X_axis - 0.2, Y4, 0.4, label = 'ComVes',color="r", bottom=Y3)
               # plt.xticks(X_axis, X1)
                X_axis = np.arange(len(X1))
                width = 0.4
                # First set of bars (ComVes)
                plt.bar(X_axis - width/2, Y1, width, label='ComVes(Overloaded)', color="g", edgecolor='black')
                plt.bar(X_axis - width/2, Y2, width, label='ComVes(Processed)', color="y", bottom=Y1, edgecolor='black', hatch='/' )
                # Second set of bars (Proposed Strategy)
                plt.bar(X_axis + width/2, Y3, width, label='Proposed(Overloaded)', color="m", edgecolor='black')
                plt.bar(X_axis + width/2, Y4, width, label='Proposed(Processed)', color="c", bottom=Y3 , edgecolor='black', hatch='.')
                # Set X and Y axis labels
               # plt.xlabel('X1')
               # plt.ylabel('Y values')

                # Set X axis tick labels
                plt.xticks(X_axis, X1)                
             #   plt.bar(X1, Y1, color="y")
             #   plt.bar(X1, Y2, bottom=Y1, color="g")
             #   plt.bar(X2, Y2, color="y")
             #   plt.bar(X2, Y3, bottom=Y1, color="g")
                plt.xlabel("Server")
                plt.ylabel("Number Of Task")
               # plt.legend(["ComVes(OVERLOADED)", "ComVes(DATA PROCESSED)", "Proposed(OVERLOADED)", "Proposed(DATA PROCESSED)"])
                plt.title("Number of Task per Server")
                plt.legend()
                arr = filename.split("-")
                plt.savefig(traceID + "-fig1-" + arr[2] + "-" + arr[3] + "-Server-d1.png")
         #       X1.clear()
         #       X2.clear()
         #       Y1.clear()
         #       Y2.clear()
         #       X3.clear()
         #       X4.clear()
         #       Y3.clear()
         #       Y4.clear()
                list1.clear()
                list2.clear()
                dicto.clear()
                dictp.clear()
                sorted_dict1.clear()
                sorted_dict2.clear()
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
