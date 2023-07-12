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
        picname = filename[:-4] + ".png"
        # f3 = open(traceID + "-" + array[2] + "-" + str(array[3])[:-4] + "-Server.txt","w")
       # dictall0 = dict.fromkeys(range(100), 0)
        dictall0 = defaultdict(lambda: [])
       # dictall1 = dict.fromkeys(range(100), 0)
        dictall1 = defaultdict(lambda: [])
        time = []
        #dicto = dict.fromkeys(range(100), 0)
        #dictp = dict.fromkeys(range(100), 0)
        # dicto = dict(zip(range(10), repeat(0)))
        # dictp = dict(zip(range(10), repeat(0)))
        print("Trying :" + filename)
        with open(filename, "r") as csvh:
            # dialect = csv.Sniffer().sniff(csvh.read(10*1024))
           # dialect = csv.Sniffer().has_header(csvh.read(1024))
            dialect = csv.Sniffer().sniff(csvh.readline())
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)
            for row in reader:  # nodeid, name, servercount,status
               # print(row["status"])
               # print(row["nodeid"])
                time.append(float(row['time']))
                thresholds = np.arange(1, int(max(time)) + 1)
                turn = 2
                if(str(array[3]) == "0"):
                    turn = 0
                    if str(row["status"]) == "OVERLOADED":
                        dictall0[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Processing":
                        dictall0[int(row["nodeid"])].append(float(row["time"]))
                if(str(array[3]) == "1"):
                    turn = 1
                    if str(row["status"]) == "OVERLOADED":
                        dictall1[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Processing":
                        dictall1[int(row["nodeid"])].append(float(row["time"]))
            if(int(turn)==0):
                dictall0Keys = list(dictall0.keys())
                print("Comves")
                print(dictall0Keys)
                index = 0
                while index < len(dictall0Keys):
                    tserver = dictall0Keys[index]
                    server = str(tserver)
                    print("server " + server)
                    #print(str(dictall0[tserver]))
                    res = list(dictall0[tserver])
                    #print(index, dictall0Keys[index])
                   # res = [sub[server] for sub in dictall0]
                   # res = list(map(itemgetter(str(dictall0Keys[index])), dictall0))
                    #print(res)
                   # occurrences = [np.sum((res <= threshold) & (res > threshold - 1)) for threshold in thresholds]
                  #  res = list(dictall0[tserver])  # Sort the list in ascending order
                  #  occurrences = [len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds]
                    occurrences = [len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds]
                   # occurrences = len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds
                    print(occurrences)
# Plot the number of overloaded occurrences per second
                    lbl = "Comves " + server
                    plt.plot(thresholds, occurrences, alpha=0.50, label=lbl,  linestyle=':', linewidth=2)
                    res.clear()
                    #occurrences.clear()
                    index += 1
            if(int(turn)==1):
                dictall1Keys = list(dictall1.keys())
                print("Proposed Strategy")
                #print(dictall1Keys)
                index = 0
                while index < len(dictall1Keys):
                    #print(index, dictall1Keys[index])                    
                   # res = [ sub[index] for sub in dictall0]
                    tserver = dictall1Keys[index]
                    server = str(tserver)
                    print("server " + server)
                    #print(str(dictall1[tserver]))
                    res = list(dictall1[tserver])
                  #  res = list(map(itemgetter(server), dictall1))
                    #print(res)
                    #occurrences = [np.sum((res <= threshold) & (res > threshold - 1)) for threshold in thresholds]
                    occurrences1 = [len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds]
                    #occurrences1 = len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds
                    print(occurrences1)
                   # res = list(dictall0[tserver])  # Sort the list in ascending order
                   # occurrences = [len(np.where((res <= threshold) & (res > threshold - 1))[0]) for threshold in thresholds]

# Plot the number of overloaded occurrences per second
                    lbl = "Proposed Strategy " + server
                    plt.plot(thresholds, occurrences1, alpha=0.60, label=lbl, linestyle='--', linewidth=2)
                    index += 1
                    res.clear()
                    #occurrences.clear()
            plt.xlabel('Time (s)')
            plt.legend()
            plt.ylabel('Number of Requests')
            plt.title('Number of Requests per Second')
            plt.savefig(picname)
               # Filter for 'OVERLOADED' status
                #overloaded_time = [t for t, s in zip(time, status) if s == 'OVERLOADED']
                #print(len(overloaded_time))
                # Calculate the number of overloaded occurrences for each second
                #thresholds = np.arange(1, int(max(time)) + 1)
                #occurrences = [sum(overloaded_time <= threshold) for threshold in thresholds]
                #occurrences = [np.sum((overloaded_time <= threshold) & (overloaded_time > threshold - 1)) for threshold in thresholds]
                # Plot the number of overloaded occurrences per second
                #plt.plot(thresholds, occurrences, marker='o')
                #plt.xlabel('Time (s)')
                #plt.ylabel('Number of Requests')
                #plt.title('Number of Requests per Second')
                #plt.savefig(picname)    
            dictall0.clear()
            dictall1.clear()

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
