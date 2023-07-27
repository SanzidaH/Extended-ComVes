from __future__ import division
import csv
import sys
from collections import defaultdict
import numpy as np
import matplotlib.pyplot as plt
import glob
from operator import itemgetter
#global first
#first = 3
def main(filepath=None, traceID=None, maxSuccess=None):
    count = []
    node_count = 0
    latency_sum = []
    tp = []
    sent = []
    nodes = {}
    global first
    first = 3
    # first = [];
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
   # global first
    filesToProcess.sort(key=str.lower)
    for filename in filesToProcess:
        array = filename.split("-")
        print("Trying :" + filename)
        with open(filename, "r") as csvh:
            dialect = csv.Sniffer().sniff(csvh.readline())
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)
            count = 0
            count1 = 0
            for row in reader:  # nodeid, name, servercount,status
                #if str(row["status"]) == "OVERLOADED" or str(row["status"]) == "Data Processing":
                if str(row["status"]) == "OVERLOADED":
                        if(int(int((str(row["service"]).split('/')[-1]).split('=')[-1]))<5000 and str(row["nodeid"]) =="8"):
                            print(str(row))
                            count=count+1
                        elif(int(int((str(row["service"]).split('/')[-1]).split('=')[-1]))<5000 and str(row["nodeid"]) =="7"):
                            count1=count1+1      
            print(count)
            print("server 7:")
            print(count1)
                            #print(str(row))
    return 0

if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
