#!/bin/bash

#dataset_name=(velocity mfeat bloodDonation thyroid tuandromd liver)
#points=(115 205 374 3600 2232 173)
#numClusters=(3)
#dataset_name=(wine seeds flame pathbased haberman spiral ecoli liver ionosphere wdbc R13 control aggregation 3C 2Q 2O 3G hdbscan segment madelon D31 sattelite muskv1 thyroid isolet smartphone pendigits shuttle mnist worms64d worms2d)

#dataset_name=(wine seeds flame pathbased haberman spiral ecoli liver ionosphere wdbc R13 control aggregation 3C 2Q 2O 3G hdbscan segment madelon D31 sattelite muskv1 thyroid isolet smartphone pendigits shuttle mnist worms64d worms2d)
#numClusters=(3 3 2 3 2 3 8 2 2 2 13 6 7 3 2 2 3 6 7 2 31 7 2 3 26 6 10 7 10 25 35)
#dataset_name=(mutantp53 tuandromd thyroid bloodDonation mfeat liver velocity)
#points=(5500 2232 3600 374 205 173 115)
dataset_name=(parkinson)
points=(380)

minK=1
maxK=229

g++ -o Euclidean_kNN_AD Euclidean_kNN_AD.cpp 

g++ -o construct_write_IF construct_write_IF.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization 

g++ -o woLCA_M_kNN_AD woLCA_M_kNN_AD.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

g++ -o fast_M_kNN_AD fast_M_kNN_AD.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization

g++ -o lca_M_kNN_AD lca_M_kNN_AD.cpp -L /usr/share/doc/libboost-all-dev -lboost_serialization
	
for i in `seq 1 ${#dataset_name[@]}`
do
	maxK=${points[i-1]}
	rm -r ./AD_output/"${dataset_name[i-1]}"/vanilla_IF 2>/dev/null
	rm -r ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF 2>/dev/null
	
	mkdir -p ./AD_output/${dataset_name[i-1]}/vanilla_IF 2>/dev/null
	mkdir -p ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF 2>/dev/null
	
	echo ${dataset_name[i-1]} started $(date) 
	
	
	#-------------------------Euclidean-KNN-AD------------------------------------------------------#
		
	./Euclidean_kNN_AD ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/ "$minK" "$maxK"
	
	echo "k auc f1" >> ./AD_output/"${dataset_name[i-1]}"/_euclidean_auc_f1.csv
	python3 AUC_F1.py ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/_euclidean_AScores.csv >> ./AD_output/"${dataset_name[i-1]}"/_euclidean_auc_f1.csv
	
	
	echo "k auc f1" >> ./AD_output/"${dataset_name[i-1]}"/_woLCA_MBD_auc_f1.csv
	echo "k auc f1" >> ./AD_output/"${dataset_name[i-1]}"/_fastMBD_auc_f1.csv
	echo "k auc f1" >> ./AD_output/"${dataset_name[i-1]}"/_lcaMBD_auc_f1.csv
	
	echo "dataLoadingTime iforestTime TotalTime" >> ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/runningTime.csv
	for run in `seq 1 5`
	do
	
		#-------------------------Construct and store iforest------------------------------------------------------#
		mkdir -p ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run" 2>/dev/null
	
		
		./construct_write_IF ${dataset_name[i-1]} ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/
		
		#------------------------lca-M-KNN-AD------------------------------------------------------#
		
		./lca_M_kNN_AD ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run" "$minK" "$maxK" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/
		
		
		echo "--------------------run$run-----------------" >> ./AD_output/"${dataset_name[i-1]}"/_lcaMBD_auc_f1.csv
	
		python3 AUC_F1.py ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_lca_massBased_AScores.csv>> ./AD_output/"${dataset_name[i-1]}"/_lcaMBD_auc_f1.csv
		
		
		#-------------------------fast-M-KNN-AD------------------------------------------------------#
		
		./fast_M_kNN_AD ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run" "$minK" "$maxK" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/
		
		echo "--------------------run$run-----------------" >> ./AD_output/"${dataset_name[i-1]}"/_fastMBD_auc_f1.csv
	
		python3 AUC_F1.py ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_fast_massBased_AScores.csv >> ./AD_output/"${dataset_name[i-1]}"/_fastMBD_auc_f1.csv
		
		
		#-------------------------woLCA-M-KNN-AD------------------------------------------------------#
		
		./woLCA_M_kNN_AD ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run" "$minK" "$maxK" ./stored_IF/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"/
		
		echo "--------------------run$run-----------------" >> ./AD_output/"${dataset_name[i-1]}"/_woLCA_MBD_auc_f1.csv
	
		python3 AUC_F1.py ${dataset_name[i-1]} ./AD_output/"${dataset_name[i-1]}"/vanilla_IF/IF_"$run"_woLCA_massBased_AScores.csv >> ./AD_output/"${dataset_name[i-1]}"/_woLCA_MBD_auc_f1.csv
		
		
		
	done;
	
	echo ${dataset_name[i-1]} doone $(date)
done;
exit;

  
