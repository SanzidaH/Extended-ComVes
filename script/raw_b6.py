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
    m=80
    global f
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
    f, plt1 = plt.subplots(2)
    f.set_figheight(8)
    f.set_figwidth(12)
    filesToProcess.sort(key=str.lower)
    for filename in filesToProcess:
        array = filename.split("-")
        picname = "Int-" +filename[:-4] + ".png"
        f1 = open("EUnsortedLatencies-"+str(array[1]) + "-"+str(array[2])+"-"+str(array[3])+".txt","w")
        # f3 = open(traceID + "-" + array[2] + "-" + str(array[3])[:-4] + "-Server.txt","w")
       # dictall0 = dict.fromkeys(range(100), 0)
        dictall0 = defaultdict(lambda: [])
        dicto0 = defaultdict(lambda: [])
        dictp0 = defaultdict(lambda: [])
        dicte0 = defaultdict(lambda: [])
        dictd0 = defaultdict(lambda: [])
       # dictall1 = dict.fromkeys(range(100), 0)
        dictall1 = defaultdict(lambda: [])
        dicto1 = defaultdict(lambda: [])
        dictp1 = defaultdict(lambda: [])
        dicte1 = defaultdict(lambda: [])
        dictd1 = defaultdict(lambda: [])
        #plt3 = plt.figure(figsize=(15,6))
        #plt1 = plt3.add_subplot(121)
        #plt2 = plt3.add_subplot(122)
        #f, plt1 = plt.subplots(2)
        timeall = []
        #dicto = dict.fromkeys(range(100), 0)
        #dictp = dict.fromkeys(range(100), 0)
        # dicto = dict(zip(range(10), repeat(0)))
        # dictp = dict(zip(range(10), repeat(0)))
        print("Trying :" + filename)
#        with open(filename, "r") as csvh:
        with open(filename, encoding="utf8", errors='ignore') as csvh:
            # dialect = csv.Sniffer().sniff(csvh.read(10*1024))
           # dialect = csv.Sniffer().has_header(csvh.read(1024))
            dialect = csv.Sniffer().sniff(csvh.readline())
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)
            for row in reader:  # nodeid, name, servercount,status
               # print(row["status"])
               # print(row["nodeid"])
                #print(str(row['time']))
                timeall.append(float(row['time']))
                time1 = float(row['time'])
                thresholds = np.arange(1, int(max(timeall)) + 1)
                turn = 2
                if(str(array[3]) == "0"):#(m-10)
                    turn = 0
                    global x0
                    x0=1
                    f=0
                    #print("hello2")
                    if str(row["status"]) == "OVERLOADED" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                       # print(int((str(row["service"]).split('/')[-1]).split('=')[-1]))
                       # print(float(row["time"]))
                        dicto0[int(row["nodeid"])].append(float(row["time"]))
                        dictall0[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Processing" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dictp0[int(row["nodeid"])].append(float(row["time"]))
                        dictall0[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Correction" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dictd0[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Expired" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dicte0[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Computation time":
                        f1.write(str(row["service"])+"\n")

                if(str(array[3]) == "1"):
                    turn = 1
                    global x1
                    x1=1
                    f=1
                    #print(str(float(row["time"])) + "strategy " + str(row["nodeid"]))
                    if str(row["status"]) == "OVERLOADED" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dicto1[int(row["nodeid"])].append(float(row["time"]))
                        dictall1[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Processing" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        #print(int((str(row["service"]).split('/')[-1]).split('=')[-1]))
                        #print(float(row["time"]))
                        #print(int(row["nodeid"]))
                        dictp1[int(row["nodeid"])].append(float(row["time"]))
                        dictall1[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Data Correction" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dictd1[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Expired" and int((str(row["service"]).split('/')[-1]).split('=')[-1])<5000:
                        dicte1[int(row["nodeid"])].append(float(row["time"]))
                    elif str(row["status"]) == "Computation time":
                        f1.write(str(row["service"])+"\n")

            if(int(turn)==0):
                dictall0Keys = list(dictall0.keys())
                print("Comves")
                #print(dictall0Keys)
                index = 0
                while index < len(dictall0Keys):
                    tserver = dictall0Keys[index]
                    server = str(tserver)
                    print("server " + server)
                    reso0 = list(dicto0[tserver])
                    resp0 = list(dictp0[tserver])
                    rese0 = list(dicte0[tserver])
                    resd0 = list(dictd0[tserver])
                    #print("reso0")
                    #print(reso0)
                    o0 = [len(np.where((reso0 <= threshold) & (reso0 > threshold - 1))[0]) for threshold in thresholds]
                    #print(o0)
                    occurrenceso0=np.cumsum(o0).tolist()
                    #print("hello")
                    #print(occurrenceso0)
                    #occurrenceso0 = np.cumsum([len(np.where((reso0 <= threshold) & (reso0 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrenceso0")
                    occurrencesp0 =np.cumsum([len(np.where((resp0 <= threshold) & (resp0 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrencesp0")
                    occurrencese0 =np.cumsum([len(np.where((rese0 <= threshold) & (rese0 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrencese0")
                    occurrencesd0 =np.cumsum([len(np.where((resd0 <= threshold) & (resd0 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    index += 1
                    lblo = "Overload- " + server + "- " + str(occurrenceso0[-1]) 
                    lblp = "Processed- " + server + "- " + str(occurrencesp0[-1])
                    lble = "Expired- " + server+ "- " + str(occurrencese0[-1])
                    lbld = "Sent- " + server + "- " + str(occurrencesd0[-1])    
                    #plt.figure(figsize=(15,6))
                    #plt1.subplot(1, 2, 1)                   
                    plt1[0].set_title("Comves")
                    #plt1.set_xlabel('Time (s)')
                    #plt1.set_ylabel('Number of Requests')
                    #plt1.legend(loc="upper left")
                    plt1[0].plot(thresholds, occurrenceso0, alpha=1, label=lblo, marker='*', linestyle='--', linewidth=2)
                    #plt1[0].plot(thresholds, occurrencesp0, alpha=0.3, label=lblp, marker='<', linestyle='-', linewidth=1)
                    plt1[0].plot(thresholds, occurrencese0, alpha=0.8, label=lble, marker='.', linestyle='-.', linewidth=1)
                    plt1[0].plot(thresholds, occurrencesd0, alpha=0.8, label=lbld, marker='x', linestyle=':', linewidth=1)
                    plt1[0].legend(loc="upper left")
                    occurrenceso0.clear()
                    occurrencesp0.clear()
                    occurrencese0.clear()
                    occurrencesd0.clear()

            if(int(turn)==1):
                dictall1Keys = list(dictall1.keys())
                print("Proposed Strategy")
                #print(dictall1Keys)
                index = 0
                while index < len(dictall1Keys):
                    print("index "+ str(index))
                #tserver = dictall1Keys[index]
                #server = str(tserver)
                    tserver = dictall1Keys[index]
                    server = str(tserver)
                    reso1 = list(dicto1[tserver])
                    resp1 = list(dictp1[tserver])
                    rese1 = list(dicte1[tserver])
                    resd1 = list(dictd1[tserver])
                    occurrenceso1 = np.cumsum([len(np.where((reso1 <= threshold) & (reso1 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrenceso0")
                    occurrencesp1 = np.cumsum([len(np.where((resp1 <= threshold) & (resp1 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrencesp0")
                    occurrencese1 = np.cumsum([len(np.where((rese1 <= threshold) & (rese1 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    #print("occurrencese0")
                    occurrencesd1 = np.cumsum([len(np.where((resd1 <= threshold) & (resd1 > threshold - 1))[0]) for threshold in thresholds]).tolist()
                    index += 1
                    lblo = "Overload- " + server + "- " + str(occurrenceso1[-1])
                    lblp = "Processed- " + server+ "- " + str(occurrencesp1[-1])
                    lble = "Expired- " + server+ "- " + str(occurrencese1[-1])
                    lbld = "Sent- " + server+ "- " + str(occurrencesd1[-1])
                    #if('x0' in globals() or f ==0):
                    #plt2.subplot(1, 2, 2)
                    plt1[1].set_title("Proposed Strategy")
                    #plt2.set_xlabel('Time (s)')
                    #plt2.set_ylabel('Number of Requests')
                    #plt2.legend(loc="upper left")
                    plt1[1].plot(thresholds, occurrenceso1, alpha=1, label=lblo, marker='*', linestyle='--', linewidth=2)
                    #plt1[1].plot(thresholds, occurrencesp1, alpha=0.3, label=lblp, marker='o', linestyle='-', linewidth=1)
                    #plt.plot(thresholds, occurrencese1, alpha=0.8, label=lble, marker='x', linestyle='-.', linewidth=1)
                    plt1[1].plot(thresholds, occurrencesd1, alpha=0.8, label=lbld, marker='x', linestyle=':', linewidth=1)
                    plt1[1].legend(loc="upper left")
                    occurrenceso1.clear()
                    occurrencesp1.clear()
                    occurrencese1.clear()
                    occurrencesd1.clear()
                    #resd1.clear()
           # plt3.xlabel('Time (s)')
           # plt3.legend()
           # plt3.ylabel('Number of Requests')
           # plt3.set_xlabel('Time(s)')
           # plt3.set_ylabel('Number of Requests')
            if('x0' in globals() and 'x1' in globals()):
                plt.suptitle('Number of Requests/Sec(Consumer 0) vs Time (s)',fontsize = 16)
                plt.savefig(picname)
                dictall0.clear()
                dictall1.clear()

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
