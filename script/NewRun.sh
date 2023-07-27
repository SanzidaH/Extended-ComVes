#!/bin/bash
# python3 raw-a.py comves-0 "0"
# ./waf --run="ndn-simple --Run=0 --freq=70 --dur=5 --util=20 --cor=0"

run="3009"
#freq="40 60 80 100 120 140 180 200"
#freq="200"
freq="50" 
util="5"
dur="30"
#cor="0 1"
#trace=2000

#NS_LOG="ndn.Producer=*|*":"ndn.Consumer=*|*":"ndn-cxx.nfd.uc=*|*":"ndn-cxx.nfd.Forwarder=*|*":"ndn-cxx.nfd.ProcessNackTraits=*|*" ./waf --run="ndn-simple2 --Run=$run --freq=$freq --dur=$dur --util=$util --cor=1">$run-$freq-1.txt 2>$run-$freq-1.txt
NS_LOG="ndn.Producer=*|*":"ndn.Consumer=*|*":"ndn-cxx.nfd.uc=*|*" ./waf --run="ndn-simple4 --Run=$run --freq=$freq --dur=$dur --util=$util --cor=1">"log"-$run-$freq-1.txt 2>"log"-$run-$freq-1.txt
wait  
NS_LOG="ndn.Producer=*|*":"ndn.Consumer=*|*":"ndn-cxx.nfd.Comves=*|*" ./waf --run="ndn-simple4 --Run=$run --freq=$freq --dur=$dur --util=$util --cor=0">"log"-$run-$freq-0.txt 2>"log"-$run-$freq-0.txt
wait
python3 raw_test2.py "log-$run-$freq-1">m_table-$run0-$freq-1.txt 2>m_table-$run0-$freq-1.txt
python3 raw_test2.py "log-$run-$freq-0">m_table-$run0-$freq-0.txt 2>m_table-$run0-$freq-0.txt

# Iterate the string variable using for loop
#for val in $StringVal; do
# echo $val
#done

#userreq=10
#1 - b1, 10 - b4, 3 - b7, 5 - b10, 1/10,5/3
#traceID = "block7"

#for val in $freq
#do	
#	for c in $cor
#	do	
#                NS_GLOBAL_VALUE="RngRun=$run" ./waf --run="ndn-pec --Run=$run --freq=$val --dur=$dur --util=$util --cor=$c" &
#                wait
#                echo "Start instance $i for id $Test"
#       	done	
#done
#wait
#echo "Finished for $Test at $(date)!"
#echo "All Simulations Complete"
#echo "All Simulations Complete for New Run"

#python3 raw-a.py comves-$run $run
#echo "python3 raw-a.py comves-$run $run"
#python3 raw_b.py server-$run $run

#for val in $freq
#do
#        for c in $cor;
#        do
#                python3 raw_b.py "server-$run-$c-$val-" $run
#		echo "python3 raw_b.py "server-$run-$c-$val-" $run"
#		wait
#        done
#done
#wait
#echo "script run for average result"

#python3 plot1.py $run-Average_Delay-.txt
#python3 plot1.py $run-Success_Ratio-.txt
#echo "Plotting done"

#mv -f ndn-proactive-* /newhome/sanzida/scripts/
#mv -f serverUpdate-proactive-* /newhome/sanzida/Plotting/

