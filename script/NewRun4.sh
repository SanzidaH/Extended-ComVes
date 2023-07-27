#!/bin/bash
# python3 raw-a.py comves-0 "0"
# ./waf --run="ndn-simple --Run=0 --freq=70 --dur=5 --util=20 --cor=0"

runs="708 709 710"
#runs=($runs1)
crun="711"
temp="1000"
#freq="40 60 80 100 120 140 180 200"
#freq="200"
#freq="75 80 85 90 95 100" 
#freq="30 35 40 45 50"
#freq="30 32 34 36"
#freq="40 45 50 55 60"
#freq="30 35 40 45"
#freq="40 45 50 55" #one consumer, 100capacity
#freq="10 20 30 40" 
#freq="30 34 38 40"
#freq="15 16 17 18" # 2 consumer, 50/100 capacity
freq="30 35 40 45"
#freq="25 30 35 40"
#freq="25 30 35 40"# two freq confirm
#freq="10 11 12 13"
#freq="27 30 33 36"
#freq="12 14 16 18"
#freq="30 40 50 60 70"
util="5"
dur="30"
#cor="0 1"
cor="0 1"
echo ${runs:0:3} 
# Iterate the string variable using for loop
#for val in $StringVal; do
# echo $val
#done

#userreq=10
#1 - b1, 10 - b4, 3 - b7, 5 - b10, 1/10,5/3
#traceID = "block7"
for c in $cor 
do
	for val in $freq
	do	
		for run in $runs
		do	
                	NS_GLOBAL_VALUE="RngRun=$run" ./waf --run="ndn-simple4 --Run=$run --freq=$val --dur=$dur --util=$util --cor=$c" &
			#NS_GLOBAL_VALUE="RngRun=$((run*temp))" ./waf --run="ndn-simple2 --Run=$run --freq=$val --dur=$dur --util=$util --cor=$c" &
			wait
                	echo "Start instance freq $val and strategy $c for id $run"
			echo "NS_GLOBAL_VALUE="RngRun=$run" ./waf --run="ndn-simple4 --Run=$run --freq=$val --dur=$dur --util=$util --cor=$c" &"
       		done	
	done
done

for c in $cor
do
        for val in $freq
        do
		first_run=1
                for run in $runs
                do
			if (($first_run==1))
			then
                        	cp server-$run-$val-$c-.csv server-$crun-$val-$c-.csv
				cp comves-$run-$c-$val.csv comves-$crun-$c-$val.csv
				echo "cp server-$run-$val-$c-.csv >> server-$crun-$val-$c-.csv"
                                echo "cp comves-$run-$c-$val.csv >> comves-$crun-$c-$val.csv"
				echo "$first_run"
				first_run=0
				echo "$first_run"
			else
				echo "Not first"
				tail -n +2 "server-$run-$val-$c-.csv" > server_out.csv 
				cat server_out.csv >> "server-$crun-$val-$c-.csv" 
              			tail -n +2 "comves-$run-$c-$val.csv" > comves_out.csv
				cat comves_out.csv >> "comves-$crun-$c-$val.csv"
				echo "tail -n +2 "server-$run-$val-$c-.csv" >> "server-$crun-$val-$c-.csv""
                                echo "tail -n +2 "comves-$run-$c-$val.csv" >> "comves-$crun-$c-$val.csv""
			fi
                        wait
                        echo "Appending files for freq $val and strategy $c for id $run"
                done
        done
done

echo "Finished for $Test at $(date)!"
#echo "All Simulations Complete"
echo "All Simulations Complete for New Run"
wait
python3 raw-a.py comves-$crun $crun
echo "python3 raw-a.py comves-$crun $crun"
#python3 raw_b.py server-$run $run

for val in $freq
do
      # for c in $cor;
      # do
               # python3 raw_b.py "server-$run-$c-$val-" $run
               #        echo "python3 raw_b.py "server-$run-$c-$val-" $run"
               python3 raw_b3.py "server-$crun-$val-" $crun
	       python3 raw_b6.py "server-$crun-$val-" $crun
       #        python3 raw_b2.py "server-$run-$val-$c" $run
               echo "python3 raw_b3.py "server-$crun-$val-" $crun"
               echo "python3 raw_b6.py "server-$crun-$val-" $crun"
	       wait
      # done
done
wait
echo "script run for average result"

python3 plot1.py $crun-Average_Delay-.txt
echo "python3 plot1.py $crun-Average_Delay-.txt"
python3 plot1.py $crun-Success_Ratio-.txt
echo "python3 plot1.py $crun-Success_Ratio-.txt"
python3 plot1.py $crun-Success_Ratio_Consumer0.txt
echo "plot1.py $crun-Success_Ratio_Consumer0.txt"
python3 cdf_plot.py UnsortedLatencies-$crun
echo "python3 cdf_plot.py UnsortedLatencies-$crun"
echo "Plotting done"

#mv -f ndn-proactive-* /newhome/sanzida/scripts/
#mv -f serverUpdate-proactive-* /newhome/sanzida/Plotting/

