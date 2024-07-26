#!/bin/bash

dataset_name=(2O)
numClusters=(2)
dataset_name=(wine seeds flame pathbased haberman spiral ecoli liver ionosphere wdbc R13 control aggregation 3C 2Q 2O 3G hdbscan segment madelon D31 sattelite muskv1 thyroid isolet smartphone pendigits shuttle mnist)
numClusters=(3 3 2 3 2 3 8 2 2 2 13 6 7 3 2 2 3 6 7 2 31 7 2 3 26 6 10 7 10)
#dataset_name=(wine seeds flame pathbased haberman spiral ecoli liver ionosphere wdbc R13 control aggregation 3C 2Q 2O 3G hdbscan segment madelon D31 sattelite muskv1 thyroid isolet smartphone pendigits shuttle mnist worms64d worms2d)
#numClusters=(3 3 2 3 2 3 8 2 2 2 13 6 7 3 2 2 3 6 7 2 31 7 2 3 26 6 10 7 10 25 35)
dataset_name=(pendigits)
numClusters=(10)

step_size=0.1
max_percent=1

g++ -o Euclidean_DPC Euclidean_DPC.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization 

g++ -o construct_write_IF construct_write_IF.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization 

g++ -o wolcaMBD_MPC wolcaMBD_MPC.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

g++ -o fastMBD_MPC fastMBD_MPC.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

g++ -o lcaMBD_MPC lcaMBD_MPC.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization




for i in `seq 1 ${#dataset_name[@]}`
do
	
	echo ${dataset_name[i-1]} started $(date) 
	rm -r ./clustering_output/"${dataset_name[i-1]}" 2>/dev/null
	rm -r ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF 2>/dev/null
	rm -r ./evaluation_results/"${dataset_name[i-1]}"/ 2>/dev/null
	
	mkdir -p ./clustering_output/${dataset_name[i-1]}/vanilla_IF 2>/dev/null
	mkdir -p ./stored_IF/${dataset_name[i-1]}/vanilla_IF 2>/dev/null
	mkdir -p ./evaluation_results/${dataset_name[i-1]}/ 2>/dev/null
	
	#--------------------------Euclidean-DPC---------------------------------------------------------------#
	
	./Euclidean_DPC "${dataset_name[i-1]}" "${numClusters[i-1]}" "$step_size" "$max_percent" ./clustering_output/"${dataset_name[i-1]}"/    
	
	echo "percent cutoff f-measure RI entropy" >> ./evaluation_results/"${dataset_name[i-1]}"/_euclidean_DPC_f_r_e.csv
	
	python3 evaluate_clusters.py ${dataset_name[i-1]} $step_size $max_percent ./clustering_output/"${dataset_name[i-1]}"/_euclidean_cId.csv >> ./evaluation_results/${dataset_name[i-1]}/_euclidean_DPC_f_r_e.csv
	
	
	echo "percent cutoff f-measure RI entropy" >> ./evaluation_results/"${dataset_name[i-1]}"/_wolcaMBD_MPC_f_r_e.csv
	echo "percent cutoff f-measure RI entropy" >> ./evaluation_results/"${dataset_name[i-1]}"/_fastMBD_MPC_f_r_e.csv
	echo "percent cutoff f-measure RI entropy" >> ./evaluation_results/"${dataset_name[i-1]}"/_lcaMBD_MPC_f_r_e.csv
	
	echo "dataLoadingTime iforestTime TotalTime" >> ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/runningTime.csv
	for run in `seq 1 10`
	do
	#-------------------------Construct and store iforest------------------------------------------------------#
		#rm -r ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF 2>/dev/null
		mkdir -p ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run" 2>/dev/null
	
		
		./construct_write_IF ${dataset_name[i-1]} ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/
		
	
	
	#------------------------lca-MPC------------------------------------------------------#
		
		./lcaMBD_MPC "${dataset_name[i-1]}" "${numClusters[i-1]}" "$step_size" "$max_percent" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/ ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"
		
		
		echo "--------------------run$run-----------------" >> ./evaluation_results/"${dataset_name[i-1]}"/_lcaMBD_MPC_f_r_e.csv
	
		python3 evaluate_clusters.py "${dataset_name[i-1]}" "$step_size" "$max_percent" ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_lcaMBD_cId.csv >> ./evaluation_results/"${dataset_name[i-1]}"/_lcaMBD_MPC_f_r_e.csv
		
	
	#-------------------------fast-MPC------------------------------------------------------#
		./fastMBD_MPC "${dataset_name[i-1]}" "${numClusters[i-1]}" "$step_size" "$max_percent" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/ ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"
		
		
		echo "--------------------run$run-----------------" >> ./evaluation_results/"${dataset_name[i-1]}"/_fastMBD_MPC_f_r_e.csv
	
		python3 evaluate_clusters.py "${dataset_name[i-1]}" "$step_size" "$max_percent" ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_fastMBD_cId.csv >> ./evaluation_results/"${dataset_name[i-1]}"/_fastMBD_MPC_f_r_e.csv
		
		
		
		
	#-------------------------woLCA-MPC------------------------------------------------------#
		./wolcaMBD_MPC "${dataset_name[i-1]}" "${numClusters[i-1]}" "$step_size" "$max_percent" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/ ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"
		
		echo "--------------------run$run-----------------" >> ./evaluation_results/"${dataset_name[i-1]}"/_wolcaMBD_MPC_f_r_e.csv
	
		python3 evaluate_clusters.py "${dataset_name[i-1]}" "$step_size" "$max_percent" ./clustering_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_wolcaMBD_cId.csv >> ./evaluation_results/"${dataset_name[i-1]}"/_wolcaMBD_MPC_f_r_e.csv
			
		
		
	
	#/usr/bin/time -v /home/scholar/Research/scalableMPC/MPC/mpc_all_cutoff ${dataset_name[i-1]} ${numClusters[i-1]} $step_size $max_percent /
	#./mpc_all_cutoff ${dataset_name[i-1]} ${numClusters[i-1]} $step_size $max_percent
	#python3 evaluate_clusters.py ${dataset_name[i-1]} $step_size $max_percent >> ./evaluation_results/${dataset_name[i-1]}/mpc_cutoff_fmeasure_randIndex_entropy.csv
	
	
	done;
	echo ${dataset_name[i-1]} doone $(date)
	
done
exit;

    

