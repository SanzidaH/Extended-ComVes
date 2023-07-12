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
       # points1 = defaultdict(lambda: [])
       # points2 = defaultdict(lambda: [])
        dict1 = {}
        dict2 = {}
        print("Trying :" + filename)
        f = open(filename,'r')
        array = filename.split("-")
        for row in f:
            row = row.split('  ')
            if(int(row[0])==0):
                dict1[float(row[1])] = float(row[2])
            else:
                dict2[float(row[1])] = float(row[2])
                #points1[float(row[1])].append(float(row[2])
          #  else:
          #      points2[float(row[1])].append(float(row[2])
       # myKeys1 = list(points1.keys())
       # myKeys1.sort()
       # sorted_dict1 = {i: points1[i] for i in myKeys1}
       # myKeys2 = list(points2.keys())
       # myKeys2.sort()
       # sorted_dict2 = {i: points2[i] for i in myKeys2}
       # points1 = dict(sorted(points1.items())
       # X1, Y1 = zip(*points1)
       # points2 = dict(sorted(points2.items())
       # X2, Y2 = zip(*points2)
       # for k, v in dict1.items():
       #     print(k, v)
       # for k, v in dict2.items():
       #     print(k, v)
        sorted_dict1 = dict(sorted(dict1.items()))
        sorted_dict2 = dict(sorted(dict2.items()))
        for k, v in sorted_dict1.items():
            print(k, v)
        for k, v in sorted_dict2.items():
            print(k, v)
        X1 = list(sorted_dict1.keys())
        Y1 = list(sorted_dict1.values())
        X2 = list(sorted_dict2.keys())
        Y2 = list(sorted_dict2.values())
        #X1, Y1 = zip(*sorted_dict1)
        #X2, Y2 = zip(*sorted_dict2)
        #for x in range(len(X1)):
        #    print(X1[x])
        #for x in range(len(Y1)):
        #    print(Y1[x])
        #for x in range(len(X2)):
        #    print(X2[x])
        #for x in range(len(Y2)):
        #    print(Y2[x])
        plt.plot(X1, Y1, label = "Comves", linestyle="-")
        plt.plot(X2, Y2, label = "Proposed Strategy", linestyle="--")
        plt.xlabel('Frequency(Requests/Second)', fontsize = 12)
        plt.ylabel(str(array[1]), fontsize = 12)
        title = "Frequency vs "+array[1] 
        plt.title(title, fontsize = 20)
        plt.legend()
        plt.savefig(str(array[0])+"_"+array[1]+".png")
       # plt.show()
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
                    
