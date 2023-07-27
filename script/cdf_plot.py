from __future__ import division 
import csv 
import sys 
from collections import defaultdict 
import numpy as np 
import matplotlib.pyplot as plt 
import glob

def main(filepath=None, traceID=None, maxSuccess=None):
    if filepath is None and maxSuccess is None:
        sys.stderr.write("usage: python3 %s <file-path> <expected-success-number>\n")
        return 1
    filesToProcess = glob.glob(filepath + "*" ) #search for files that's used to search for files that match specific file pattern or name
    print(filesToProcess)
    s1filesToProcess = sorted(filesToProcess, key=lambda x: int(x.split("-")[2]))
   # s2filesToProcess = sorted(s1filesToProcess, key=lambda x: int((x.split("-")[3]).replace(".txt","")))
    mfilesToProcess = s1filesToProcess[:2] + s1filesToProcess[-2:]
    s2filesToProcess = sorted(mfilesToProcess, key=lambda x: int((x.split("-")[3]).replace(".txt","")))
    print(s1filesToProcess)
    print(s2filesToProcess)
    print(mfilesToProcess)
    filecount=0
    for filename in s2filesToProcess:
        filecount=filecount+1
        delays = []
        print("Trying :" + filename)
        f = open(filename,'r')
        array = filename.split("-")
        for row in f:
            row = row.split('  ')
            delays.append(float(row[0]))
        #delays.append(2)
        delays.sort()
        # converting list to array
        X1 = np.zeros(2, dtype = float)
        Y1 = np.zeros(2, dtype = float)
        X1 = np.asarray(delays)
        print(X1)
        #X2 = np.arange(X1.all())
        X2 = np.arange(min(X1), max(X1), 0.2)
        X2 = np.around(X2,1)
        #X2 = ["{:.2f}".format(value) for value in X2]
        print(X2)
        X3 = X2.astype(str)
        #X2 = map(str, X2)
        X3[-1]="inf"
        print(X3)
        #print(X1)
        #print(X2)
        Y1 = np.arange(len(X1)) / float(len(X1))
        #X2 = np.array(map(str, Y1))
        #np.append(Y1,2)
        #print(filename)
        #print(X1)
        #print(Y1)i
        p = str(array[3]).replace('.txt','')
        #print(p)
        if(filecount==1 or filecount==3):
            clr='tab:blue'
        else:
            clr='tab:orange'
        if(p=="0"):
            #plt.xticks(X1, X2)
            #plt.xticks(ticks = X1,labels=X2)
            plt.xticks(ticks=X2,labels=X3, rotation=25,fontsize=8)
            plt.plot(X1, Y1, label = "Comves "+str(array[2]), linestyle=":",color=clr)
            plt.legend()
            #X1.clear()
            #Y1.clear()
        elif(p=="1"):
            #plt.xticks(X1, X2)
            #plt.xticks(ticks = X1,labels=X2)
            plt.xticks(ticks=X2,labels=X3, rotation=25,fontsize=8)
            plt.plot(X1, Y1, label = "Proposed Strategy "+str(array[2]), linestyle="--",color=clr)
            plt.legend()
            #X1.clear()
            #Y1.clear()
    plt.xlabel('Delay')
    plt.ylabel('Probability')
    #plt.xticks(ticks = X1,labels=X2)
    title = "CDF of Delays " 
    plt.title(title)
   # plt.legend(["This is my legend"])
    plt.savefig(str(array[1])+"_"+array[2]+str(array[0])+".png")
       # plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
                    
