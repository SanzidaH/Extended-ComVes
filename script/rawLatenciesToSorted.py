from __future__ import division 
import csv 
import sys 
from collections import defaultdict 
import numpy as np 
import matplotlib.pyplot as plt 
import glob

def main(filepath=None, traceID=None, maxSuccess=None):
    print("initialize")
    count = []
    node_count = 0;
    latency_sum = []
    tp = []
    sent = []
    nodes = {};
    overhead = {};
    sentOver = {}
    overhead1 = 0
    #overhead = 0
    totalSent = 0
    simTime = 300
    for i in range(0,5000):
        tp.append(0)
        count.append(0)
        latency_sum.append(0)
        sent.append(0)
    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    xmax = 250
    cumulatedFullLatencies = []
    CFSLat = {}
    maxLat = {"1":50, "2":50, "3":50, "4":50}
    #filesToProcess = glob.glob("traces/" +filepath+"*" )
    filesToProcess = glob.glob(filepath+"*" )

    print(filesToProcess)
    for filename in filesToProcess:
        pending = {}
        pending1 = {}
        fullLatencies = []
        fSerLat = {}
        latencies = {}
        print("Trying :" + filename)
        with open(filename, 'r') as csvh:
                #dialect = csv.Sniffer().sniff(csvh.read(10*1024))
                dialect = csv.Sniffer().has_header(csvh.read(1024))
                csvh.seek(0)
                reader = csv.DictReader(csvh, dialect=dialect)

                #try:
                for row in reader:          
                    node = int(row['nodeid'])
                    if node not in nodes:
                        nodes[node] = 1;
                        node_count += 1;
                    time = float(row['time'])
                            
                    if row['event'] == 'sent' and time < simTime:
                        try:
                            if (row['service']) not in pending:
                                pending[row['service']]={}
                        except KeyError:
                             print(" Key error")        
                        if (row['name']) not in pending[row['service']]:
                            pending[row['service']][(row['name'])] = time

                        sent[int(time)] += 1
                        timeRound = int(int(time)/2)*2
                        if timeRound not in sentOver:
                            sentOver[timeRound] = 0
                        sentOver[timeRound] += 1
                        totalSent += 1
                        pending1[ row['name']] = time
                    elif (row['event'] == 'received'): #and (row['(hopCount)/seq'] == '6'):
                        #assert((node, row['name']) in pending)
                        service = ""
                        for each in pending:
                            if(row['name'] in pending[each]):
                                service = each
                        if service == "":
                            continue
                        if service not in latencies:
                            latencies[service] = defaultdict(lambda: [])
                            fSerLat[service] = []
                            #maxLat[service] = 0
                        
                        lVal = time - pending[service][( row['name'])]
                        #if lVal>25.0:
                        #    lVal = 25.0
                        latencies[service][node].append(1. * (lVal))
                        latency_sum[int(time)] += 1000. * (lVal)
                        #maxLat[service] = max(maxLat[service], lVal)
                        count[int(time)] += 1
                        tp[int(pending1[( row['name'])])] += 1
                        del pending[service][( row['name'])]
                        #del pending1[(node, row['name'])]
                    elif (row['event'] == 'over') and time < simTime and "server" in row['name']:
                        increment = 1
                        if "PEC" in row['name']:
                            increment = 1
                        #overhead1 += increment
                        timeR = int(int(time)/2)*2

                        if timeR not in overhead:
                            overhead[timeR] = 0;
                
                        overhead[timeR] += increment;
                        overhead1 += 1
                        #del pending[(node, row['name'])]
                        #del pending1[(node, row['name'])]
                #except:
                #    print("SMH")
                #    continue


                #c = 0
                for each in latencies:
                 f = open(traceID + "UnsortedLatencies"+each+".txt","w")
                 for key in latencies[each]:
                    for latencyVal in latencies[each][key]:
                        fullLatencies.append(latencyVal)
                        fSerLat[each].append(latencyVal)
                        f.write(str(latencyVal) + "\n")
                        
                 ct = 0
                 for pend in pending[each]:
                    ct += 1
                    #print(pending[each])
                    fullLatencies.append(35)
                    fSerLat[each].append(maxLat[each])
                    f.write(str(20) + "\n")
                 print(ct)
                 print(str(filename) + " Success rate: " + str(len(fullLatencies)))
	         #maxSuccess = len(fullLatencies)
	         #maxSuccess = 1
	         #if len(fullLatencies) < int(maxSuccess):
                 #for x in range(int(maxSuccess) - len(fullLatencies)):
	         #    fullLatencies.append(xmax)

                 if each not in CFSLat:
                    CFSLat[each] = []
                 CFSLat[each].extend(fSerLat[each])
                cumulatedFullLatencies.extend(fullLatencies)
    print(node_count);
"""
    f = open(traceID + "Overhead.csv","w")
    for i in sentOver:
        #print(i)
        if(i not in overhead):
            f.write(str(0) + "\n")
        else:
            f.write(str(overhead[i]/sentOver[i]) + "\n")
    
    f = open(traceID + "Overhead1.csv","w")
    print("----------------------------------------")
    print(overhead1, totalSent)
    f.write(str(overhead1/totalSent) + "\n")
    
    for i in range(0,180):
        if(count[i]==0):
                latency_sum[i] = 0
        else:
                latency_sum[i] = latency_sum[i]/count[i]
 
    for i in range(0,420):
        if(sent[i]==0):
                tp[i] = 1
        else:
                tp[i] = tp[i]/sent[i]
   
    f = open(traceID+"LossRate.txt","w")
    for i in range(420):
        f.write(str(i) +"\t"+str(tp[i])+ "\n")

#Dump Latencies
    cumulatedFullLatencies.sort()
    f = open(traceID+"SortedLatencies.txt","w")
    for each in cumulatedFullLatencies:
        f.write(str(each) + "\n")

    f = open(traceID+"SortedLatencies.csv","w")
    for each in cumulatedFullLatencies:
        if(each != 25):
            f.write(str(each) + "\n")
"""

   # for ser in CFSLat:
   #     CFSLat[ser].sort()
   #     f = open(traceID+"SortedLatencies"+ser+".txt","w")
   #     for each in CFSLat[ser]:
   #         f.write(str(each) + "\n")
   # return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))

