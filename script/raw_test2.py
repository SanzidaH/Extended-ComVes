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
        with open(filename, "r") as f:
            for line in f:
                if "Found" not in line and "mtable" in line:
                    lines=line.strip().split(" ",3)
                    #print(lines[1])
                    if lines[1] == "1":
                        print(line.strip())
                if "Found" not in line and "Face:" in line:
                    lines=line.strip().split(" ",3)
                    #print(lines[1])
                    if lines[1] == "1":
                        print(line.strip())
    return 0

if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
