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
    for filename in filesToProcess:
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
        Y1 = np.arange(len(X1)) / float(len(X1))
        #np.append(Y1,2)
        print(filename)
        #print(X1)
        #print(Y1)i
        p = str(array[3]).replace('.txt','')
        print(p)
        if(p=="0"):
            plt.plot(X1, Y1, label = "Comves "+str(array[2]), linestyle="-")
            plt.legend()
            #X1.clear()
            #Y1.clear()
        elif(p=="1"):
            plt.plot(X1, Y1, label = "Proposed Strategy"+str(array[2]), linestyle="--")
            plt.legend()
            #X1.clear()
            #Y1.clear()
    plt.xlabel('Delay')
    plt.ylabel('Probability')
    title = "CDF of Delays " 
    plt.title(title)
   # plt.legend(["This is my legend"])
    plt.savefig(str(array[1])+"_"+array[2]+"1.png")
       # plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
                    
