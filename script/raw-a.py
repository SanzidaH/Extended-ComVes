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
    m = 50
    cumulatedFullLatencies = []  #lantencies for all files
    filesToProcess = glob.glob(filepath + "*" ) #search for files that's used to search for files that match specific file pattern or name
    print(filesToProcess)
    f2 = open(traceID + "-Average_Delay-.txt","w")
    f3 = open(traceID + "-Success_Ratio-.txt","w")
    f4 = open(traceID + "-Success_Ratio_Consumer0-.txt","w")
    for filename in filesToProcess:
        pending = {}
        pending1 = {}
        senti = {}
        fullLatencies = []
        totalSent = 0
        totalSent0 = 0
        cnt = 0
        cnt0 = 0
        latencies = defaultdict(lambda: [])
        latencies0 = defaultdict(lambda: [])
        print("Trying :" + filename)
        with open(filename, 'r') as csvh:
            #dialect = csv.Sniffer().sniff(csvh.read(10*1024))
            dialect = csv.Sniffer().has_header(csvh.read())
            csvh.seek(0)
            reader = csv.DictReader(csvh, dialect=dialect)

            for row in reader:     # nodeid, name, event, time
                node = int(row['nodeid'])  #current node
                if node not in nodes:
                        nodes[node] = 1 #storing all the nodes in nodes[]
                        node_count += 1 # counting all nodes
                time = float(row['time'])  #current time for the event
                if row['event'] == 'sent': # current event = sent
                  # totalSent = totalSent + 1 # counting total sent
                   senti[(row['name'])]=0 
                   #if (row['name']) not in pending:
                   if (row['name']) not in pending and int(time) <=(m-3):#int(time) <=(m-10)  int(time) > 5
                      totalSent = totalSent + 1 # counting total sent
                      pending[(row['name'])] = time # storing unique sent packet as pending[name][time] which is not in pending
                      if (node==0): 
                          totalSent0 = totalSent0 + 1
                  # else:
                   #   print(str(row['name'])+ " same interest ...") 
                   sent[int(time)] += 1 # storing number sent packet in particular time sent[time][sent_count]
                   pending1[ row['name']] = time # storing all the sent packet as pending[name][time] like PIT
                elif (row['event'] == 'received'): # current event = received
                    # and (row['(hopCount)/seq'] == '6'):
                    #assert((node, row['name']) in pending)
                    c1=0
                    if (row['name']) in pending:
                      fullLatencies.append(1. * (time - pending[(row['name'])]))
                      cnt = cnt + 1
                      latencies[node].append(1. * (time - pending[(row['name'])])) # node wise latency
                      latency_sum[int(time)] += 1000. * (time - pending[( row['name'])]) # total latency for all node
                      count[int(time)] += 1 # counting total received
                      tp[int(pending1[( row['name'])])] += 1 # satisfied interest track and count
                      del pending[( row['name'])] # remove the satisfied interest from pending[][]
                      senti[(row['name'])]=1
                      if (node==0):
                          cnt0 = cnt0 + 1
                        #  latencies0[node].append(1. * (time - pending[(row['name'])]))
                    else:
                      c1=c1+1
                     # print( str( row['name'])  + " is recived, but not sent")
                    #print("Total received, not sent:" + str(c))
                   # del pending[( row['name'])] # remove the satisfied interest from pending[][]
                    #del pending1[(node, row['name'])]
                #else:
                    #del pending[(node, row['name'])]
                    #del pending1[(node, row['name'])]
            ###### All row reading is done
            print("********* not received *** ")
            cnt_p = 0
            for k,v in senti.items():
               # print("*********")
                if(v==0):
                    cnt_p = cnt_p + 1
                    #print(k,v)
             
       #     print("total received: " + str(cnt_p))
       #     print("total sent: " + str(totalSent))
       #     print("total success: " + str(cnt))
            c = 0
            array = filename.split("-")
            freq =  str(array[3])
            freq = freq[:-4]
            print("freq " + freq)
            f = open("UnsortedLatencies-"+traceID + "-"+freq+"-"+str(array[2])+".txt","w")
            #cnt = 0
            total = 0
            print("before adding 2:")
            print(len(fullLatencies))
            #print(fullLatencies)
            #print(fullLatencies)
            for key in latencies: # for each node == key
                for latencyVal in latencies[key]: # for each latency in each node
                    #fullLatencies.append(latencyVal) # unsorted list for all success latency
                    #f.write(str(latencyVal) + "\n") # wrtitng unsorted latency
                    total = total + float(latencyVal) # sum of all latency
                    #cnt = cnt + 1 # count of success
            totalsucceed = total
            #for _ in range(cnt_p):
             #   f.write("2" + "\n")
            cnt_q = totalSent - cnt
            print("cnt_q " + str(cnt_q) )
            print("cnt_p " + str(cnt_p) )
            fullLatencies.extend([float(2.00) for i in range(cnt_q)])
            print("After adding 2:")
            print(len(fullLatencies))
            #print(fullLatencies)
            p=0
            for item in fullLatencies:
                f.write(str(item) + "\n")
                p=p+1
            print(p)
            print("fullLatencies length: " + str(len(fullLatencies)))
            print("total  not received: " + str(cnt_p))
            print("total sent: " + str(totalSent))
            print("total success: " + str(cnt))

            print("total delay of successful sent: " + str(totalsucceed))
            print("Accurate avg delay " + str(float(totalsucceed/cnt)))
           # array = filename.split("-")
           # for word in array:
           #     print("word " +str(word))
           # freq =  str(array[3])
           # freq = freq[:-4]
           # print("freq " + freq)
            f2.write(str(array[2]) + "  "  + freq + "  " + str(totalsucceed/cnt) + "\n")
            total = total + ((totalSent-cnt)*100) # setting high latency for unsuccesful sent
           # f = open(traceID + "avgRes.txt","w")
           # f.write("3" + " "  + str(float(total/totalSent)) + "\n")
           # f.write("3" + " "  + str(float(total/totalSent) + "\n")
            print("count for succesful task:" + str(cnt))
            print("totalSent: " + str(totalSent))
            print("total response time after setting hgh delay or unsuccess: " + str(total) )
            print("response time avg: " + str(float(total/totalSent)))
            print(str(filename) + "Total Successful interest : " + str(len(fullLatencies)))
            print(str(filename) + "Success Rate:(success sent/ total sent): " + str( cnt /totalSent))
            f3.write(str(array[2]) + "  "  + freq + "  " + str(cnt/totalSent) + "\n")
            f4.write(str(array[2]) + "  "  + freq + "  " + str(cnt0/totalSent0) + "\n")
            maxSuccess = len(fullLatencies)
            #maxSuccess = 1
            #if len(fullLatencies) < int(maxSuccess):
        #for x in range(int(maxSuccess) - len(fullLatencies)):
         #   fullLatencies.append(xmax)
        #cumulatedFullLatencies.extend(fullLatencies)
        #cumulatedFullLatencies.sort()
        #f4 = open(traceID+"-" +freq +"-SortedLatencies.txt","w")
        #for each in cumulatedFullLatencies:
            #f4.write(str(each) + "\n")
            #print(node_count) 
    #f4.close()
    f2.close()
    f3.close()
    f4.close()
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
    #cumulatedFullLatencies.sort()
    #f = open(traceID+"SortedLatencies.txt","w")
    #for each in cumulatedFullLatencies:
     #   f.write(str(each) + "\n")

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
