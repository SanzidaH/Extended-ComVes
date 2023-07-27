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
    for i in range(0,5000): #initialization of the lists
        tp.append(0)
        count.append(0)
        latency_sum.append(0)
        sent.append(0)
    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    xmax = 250 # node no
    cumulatedFullLatencies = []  #lantencies for all files
    filesToProcess = glob.glob(filepath + "*" ) #search for files that's used to search for files that match specific file pattern or name
    print(filesToProcess)
    for filename in filesToProcess:
        pending = {}
        pending1 = {}
        fullLatencies = []
        totalSent = 0
        latencies = defaultdict(lambda: [])
        print("Trying :" + filename)
        with open(filename, 'r') as csvh:
            #dialect = csv.Sniffer().sniff(csvh.read(10*1024))
            dialect = csv.Sniffer().has_header(csvh.read(1024))
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)

            for row in reader:     # nodeid, name, event, time
                node = int(row['nodeid'])  #current node
                if node not in nodes:
                        nodes[node] = 1 #storing all the nodes in nodes[]
                        node_count += 1 # counting all nodes
                time = float(row['time'])  #current time for the event
                if row['event'] == 'sent': # current event = sent
                   totalSent = totalSent + 1 # counting total sent
                   if (row['name']) not in pending:
                      pending[(row['name'])] = time # storing unique sent packet as pending[name][time] which is not in pending
                   sent[int(time)] += 1 # storing number sent packet in particular time sent[time][sent_count]
                   pending1[ row['name']] = time # storing all the sent packet as pending[name][time] like PIT
                elif (row['event'] == 'received'): # current event = received
                    # and (row['(hopCount)/seq'] == '6'):
                    #assert((node, row['name']) in pending)
                    if (row['name']) in pending:
                      latencies[node].append(1. * (time - pending[(row['name'])])) # node wise latency
                      latency_sum[int(time)] += 1000. * (time - pending[( row['name'])]) # total latency for all node
                      count[int(time)] += 1 # counting total received
                      tp[int(pending1[( row['name'])])] += 1 # satisfied interest track and count
                      del pending[( row['name'])] # remove the satisfied interest from pending[][]
                    #del pending1[(node, row['name'])]
                #else:
                    #del pending[(node, row['name'])]
                    #del pending1[(node, row['name'])]
            ###### All row reading is done
            c = 0
            f = open(traceID + "UnsortedLatencies.txt","w")
            cnt = 0
            total = 0
            for key in latencies: # for each node == key
                for latencyVal in latencies[key]: # for each latency in each node
                    fullLatencies.append(latencyVal) # unsorted list for all success latency
                    f.write(str(latencyVal) + "\n") # wrtitng unsorted latency
                    total = total + float(latencyVal) # sum of all latency
                    cnt = cnt + 1 # count of success
            totalsucceed = total
            print("total delay of successful sent: " + str(totalsucceed))
            print("Accurate avg delay " + str(float(totalsucceed/cnt)))
            total = total + ((totalSent-cnt)*100) # setting high latency for unsuccesful sent
           # f = open(traceID + "avgRes.txt","w")
           # f.write("3" + " "  + str(float(total/totalSent)) + "\n")
           # f.write("3" + " "  + str(float(total/totalSent) + "\n")
            print("count for succesful task" + str(cnt))
            print("totalSent " + str(totalSent))
            print("total response time after setting hgh delay or unsuccess: " + str(total) )
            print("response time avg " + str(float(total/totalSent)))
            print(str(filename) + "Total Successful interest : " + str(len(fullLatencies)))
            print(str(filename) + "Success Rate:(success sent/ total sent) " + str( cnt /totalSent))
            maxSuccess = len(fullLatencies)
            #maxSuccess = 1
            #if len(fullLatencies) < int(maxSuccess):
            for x in range(int(maxSuccess) - len(fullLatencies)):
                fullLatencies.append(xmax)
            cumulatedFullLatencies.extend(fullLatencies)
    print(node_count) 


    for i in range(0,180):
        if(count[i]==0):
                latency_sum[i] = 0
        else:
                latency_sum[i] = latency_sum[i]/count[i] ## Average latency

    for i in range(0,180):
        if(count[i]==0):
                latency_sum[i] = 0
        else:
                latency_sum[i] = latency_sum[i]/count[i]
        #print("Avg response time: " + str(latency_sum[i]) )

 
    for i in range(0,180):
        if(sent[i]==0):
                tp[i] = 1
        else:
                tp[i] = tp[i]/sent[i]
   
 
    #Dump Latencies
    cumulatedFullLatencies.sort()
    f = open(traceID+"SortedLatencies.txt","w")
    for each in cumulatedFullLatencies:
        f.write(str(each) + "\n")

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
