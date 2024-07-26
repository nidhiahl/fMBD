#include <iostream>
#include <string>
#include <algorithm>
#include "../data_loading/dataset.cpp"
#include "../isolation_forest/vanillaIF/vanillaIF.cpp"
#include "MPC.cpp"
#include <vector>
#include <cstring>


int parseLine(char *line){
    int i = strlen(line);
    const char *p = line;
    while(*p <'0' || *p >'9')
	p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(int par){
    FILE *file = fopen("/proc/self/status","r");
    int result = -1;
    char line[128];

    while(fgets(line,128,file) != NULL){
	if(par == 1){               //Ram used
   	    if(strncmp(line, "VmRSS:", 6) == 0){
		result = parseLine(line);
		break;
	    }
	}
	if(par == 2){               //virtual memory used
   	    if(strncmp(line, "VmSize:", 7) == 0){
		result = parseLine(line);
		break;
	    }
	}
    }
    fclose(file);
    return result; 
}

double distance(std::vector<std::vector<double>> & data, int point_i, int point_j){
	double dist=0.0;
	int num_attr = data[0].size();
	for(size_t attr = 0; attr < num_attr; attr++){
		dist += pow(data[point_i][attr] - data[point_j][attr], 2);
	}
	return sqrt(dist);	
}


void sort_all_distance(std::vector<std::vector<double>> & data_matrix, std::vector<double> & sorted_distance){
	int numpoints=data_matrix.size();
    for(int i = 0; i < numpoints; i++){
        for(int j = 0; j < i; j++){
            sorted_distance.push_back(distance(data_matrix,i,j));
        }
    }
    sort(sorted_distance.begin(),sorted_distance.end());
	
}

void sort_and_store_all_distances(std::vector<double> & sorted_distance,std::vector<std::vector<double>> & distance_matrix){
	for(auto i:distance_matrix){
        for(auto j:i){
            sorted_distance.push_back(j);
        }
    }
    sort(sorted_distance.begin(),sorted_distance.end());
	
}

int main( int argc, char* argv[]){
	const std::string dataset_name = argv[1];
	const int num_Clusters = atoi(argv[2]);
	float step_size = atof(argv[3]);
	float max_percent = atof(argv[4]);
	std::string path_to_IF = argv[5];
	const std::string path_to_results = argv[6];
	
	
	
	int initialRAM = getValue(1);
	int initialVRAM = getValue(2);
	
	
	
	/*data loading */
	struct timespec start_dataLoading,end_dataLoading;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dataLoading);
	
	
	const std::string datafile_path = "./datasets/"+dataset_name+"/"+dataset_name+".csv";
	dataset *datasetObj = new dataset(datafile_path);
    
    std::vector<std::vector<double>> data_matrix;
	datasetObj->getDataMatrix(data_matrix);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dataLoading);
	double dataLoadingTime =  (((end_dataLoading.tv_sec - start_dataLoading.tv_sec) * 1e9)+(end_dataLoading.tv_nsec - start_dataLoading.tv_nsec))*1e-9;
	
	int dataLoadedRAM = getValue(1);
	int dataLoadedVRAM = getValue(2);
	
	//std::cout<<"RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
	
	
	
	/*isolation forest construction*/
	struct timespec start_iForest,end_iForest;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iForest);
	
	vanillaIF *iForestObject = new vanillaIF(data_matrix, 100, 0.01);
	vanillaIF &refiForestObject = *iForestObject;
	
	//iForestObject->constructVanillaIF();
	//iForestObject->computeNodeMass();
	
	
	//std::cout<<"ifobject created"<<std::endl;
	if (path_to_IF=="None"){iForestObject->constructVanillaIF();}
	else{iForestObject->read_IF_fromFile(path_to_IF);}
	
	//std::cout<<"iforest constructed "<<std::endl;
	
	//iForestObject->computeNodeMass();
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iForest);
	double iForestTime =  (((end_iForest.tv_sec - start_iForest.tv_sec) * 1e9)+(end_iForest.tv_nsec - start_iForest.tv_nsec))*1e-9;

	int iForestRAM = getValue(1);
	int iForestVRAM = getValue(2);
	//std::cout<<"iForestTiem="<<iForestTime<<std::endl;
	
	
	/*distance computation*/
	struct timespec start_distanceComputation,end_distanceComputation;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_distanceComputation);
	
	std::vector<std::vector<double>> distance_matrix;
	iForestObject->efficient_massMatrix(distance_matrix);
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_distanceComputation);
	double distanceComputationTime =  (((end_distanceComputation.tv_sec - start_distanceComputation.tv_sec) * 1e9)+(end_distanceComputation.tv_nsec - start_distanceComputation.tv_nsec))*1e-9;



	/*sort all distances to find appropriate cuttoff distance*/
	std::vector<double> sorted_distance;
	sort_and_store_all_distances(sorted_distance,distance_matrix);
	
	int distMatrixRAM = getValue(1);
	int distMatrixVRAM = getValue(2);
	//std::cout<<"RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
	
	
	/*Execute MPC for all cutoff distances*/
	double cutoff_percent = 0;
	for(cutoff_percent = step_size; cutoff_percent < max_percent+step_size; cutoff_percent+=step_size){
		
		/*Compute Density*/
		struct timespec start_densityComputation,end_densityComputation;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_densityComputation);
	
		double cutoffDist = sorted_distance[int(cutoff_percent*(sorted_distance.size()/100))-1];
		
		int startMPCRAM = getValue(1);
		int startMPCVRAM = getValue(2);
		//std::cout<<"1 RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
		MPC *MPCObj = new MPC(distance_matrix,cutoffDist);  // third argument is 0, meaning first argument is a distance matrix of n (i.e. no. of points) X n (i.e. no. of points) 
		MPCObj->computeDensity();
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_densityComputation);
		double densityComputationTime =  (((end_densityComputation.tv_sec - start_densityComputation.tv_sec) * 1e9)+(end_densityComputation.tv_nsec - start_densityComputation.tv_nsec))*1e-9;

		/*Compute NNHD*/
		struct timespec start_NNHDComputation,end_NNHDComputation;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_NNHDComputation);
	
		
		MPCObj->computeNNHd();
	
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_NNHDComputation);
		double NNHDComputationTime =  (((end_NNHDComputation.tv_sec - start_NNHDComputation.tv_sec) * 1e9)+(end_NNHDComputation.tv_nsec - start_NNHDComputation.tv_nsec))*1e-9;

		//std::string sortedDensityRank_path = path_to_results+"_fastMBD_sortedDensityRank.csv";
		//MPCObj->write_sortedDensityRank(sortedDensityRank_path);


		/*Find k clusters centers*/
		struct timespec start_findCC,end_findCC;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_findCC);
	
		
		MPCObj->find_k_clustercenters(num_Clusters);
	
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_findCC);
		double findCCTime =  (((end_findCC.tv_sec - start_findCC.tv_sec) * 1e9)+(end_findCC.tv_nsec - start_findCC.tv_nsec))*1e-9;

		
		/*Cluster Assignment*/
		struct timespec start_assignClusterId,end_assignClusterId;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_assignClusterId);
	
		
		MPCObj->clusterAssignment();
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_assignClusterId);
		double assignClusterIdTime =  (((end_assignClusterId.tv_sec - start_assignClusterId.tv_sec) * 1e9)+(end_assignClusterId.tv_nsec - start_assignClusterId.tv_nsec))*1e-9;

		
		/*Find Halo*/
		struct timespec start_findHalo,end_findHalo;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_findHalo);
	
		
		MPCObj->findHalo();
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_findHalo);
		double findHaloTime =  (((end_findHalo.tv_sec - start_findHalo.tv_sec) * 1e9)+(end_findHalo.tv_nsec - start_findHalo.tv_nsec))*1e-9;

		int endMPCRAM = getValue(1);
		int endMPCVRAM = getValue(2);
		//std::cout<<"2 RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
		
		/*Writing clustreing outputs to file*/	
		
		std::string runningTimeFile_path = path_to_results+"_fastMBD_runningTime.csv";
		std::ofstream write_runningTime(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
		
		if(!write_runningTime){
			std::cout<<"Can not open output file: "<<runningTimeFile_path<<std::endl;
			exit(0);
		}
			
		write_runningTime<<"cutoff_distance="<<cutoffDist<<" ";
		
		write_runningTime<<dataLoadingTime<<" "<<distanceComputationTime<<" "<<densityComputationTime+NNHDComputationTime+findCCTime+assignClusterIdTime+findHaloTime<<std::endl;
		
		
		//write_runningTime<<dataLoadingTime<<" "<<distanceComputationTime<<" "<<densityComputationTime<<" "<<NNHDComputationTime<<" "<<findCCTime<<" "<<assignClusterIdTime<<" "<<findHaloTime<<" = "<<dataLoadingTime+distanceComputationTime+densityComputationTime+NNHDComputationTime+findCCTime+assignClusterIdTime+findHaloTime<<" - "<<dataLoadedRAM-initialRAM<<" + "<<dataLoadedVRAM-initialVRAM<<" "<<iForestRAM-dataLoadedRAM<<" + "<<iForestVRAM-dataLoadedVRAM<<" "<<distMatrixRAM-iForestRAM<<" + "<<distMatrixVRAM-iForestVRAM<<" "<<endMPCRAM-startMPCRAM<<" + "<<endMPCVRAM-startMPCVRAM<<std::endl;
		write_runningTime.close();
			
		//std::string clusterCenterFile_path = path_to_results+"_fastMBD_clusterCenters.csv";
		//MPCObj->write_ClusterCenters(clusterCenterFile_path);		
		
		std::string cIdFile_path = path_to_results+"_fastMBD_cId.csv";
		MPCObj->write_cId(cIdFile_path);
		
		//std::string haloIdFile_path = path_to_results+"_fastMBD_haloId.csv";
		//MPCObj->write_haloId(haloIdFile_path);
		
		//std::string path_to_forest = "../stored_IF/"+dataset_name+"/vanillaIF";
		//iForestObject->write_IF_toFile(path_to_forest);
		
		
		delete(MPCObj);
		
	
	}
	
	return 0;
}
