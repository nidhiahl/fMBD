#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include "../data_loading/dataset.cpp"
#include "MPC.cpp"
#include "../isolation_forest/vanillaIF/vanillaIF.cpp"
#include <vector>


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

void write_massMatrix_toFile(std::vector<std::vector<double>> & mass_matrix,std::string file_path, std::string open_mode="out"){

	std::ofstream write_massMatrix_toFile;
	
	if(open_mode =="out"){
		write_massMatrix_toFile.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_massMatrix_toFile.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_massMatrix_toFile){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	
	
	for(auto row:mass_matrix){
		for(auto pairmass:row){
			write_massMatrix_toFile<<pairmass<<" ";
		}
		write_massMatrix_toFile<<std::endl;
		
	}
	//write_massMatrix_toFile<<std::endl;
	write_massMatrix_toFile.close();
	
}


int main( int argc, char* argv[]){
	//std::cout<<argc<<std::endl;
	const std::string dataset_name = argv[1];
	
	int initialRAM = getValue(1);
	int initialVRAM = getValue(2);
	//std::cout<<"RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
	/*data loading */
	struct timespec start_dataLoading,end_dataLoading;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dataLoading);
	
	
	const std::string datafile_path = "../datasets/"+dataset_name+"/"+dataset_name+".csv";
	dataset *datasetObj = new dataset(datafile_path);
    const dataset &refDataObject = *datasetObj;
	std::vector<std::vector<double>> data_matrix;
	datasetObj->getDataMatrix(data_matrix);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dataLoading);
	double dataLoadingTime =  (((end_dataLoading.tv_sec - start_dataLoading.tv_sec) * 1e9)+(end_dataLoading.tv_nsec - start_dataLoading.tv_nsec))*1e-9;
	
	int dataLoadedRAM = getValue(1);
	int dataLoadedVRAM = getValue(2);
	//std::cout<<"RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl;
	
	/*isolation forest construction*/
	
	//struct timespec start_iForest,end_iForest;
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iForest);
	
	vanillaIF *iForestObject = new vanillaIF(data_matrix, 100, 300);
	vanillaIF &refiForestObject = *iForestObject;
	
	iForestObject->constructVanillaIF();
	std::cout<<"vanillaIF constructin done"<<std::endl;
	
	std::cout<<getValue(1)<<" "<<getValue(2)<<std::endl;
	
	
	iForestObject->computeNodeMass();
	std::cout<<"nodemass computation done"<<std::endl;
	
	
	/*distance computation*/
	//struct timespec start_distanceComputation,end_distanceComputation;
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_distanceComputation);
	
	
	std::vector<std::vector<double>> distance_matrix1;
	
	iForestObject->computeMassMatrix(distance_matrix1);
	std::cout<<"mass matrix coputation donae"<<std::endl;
	
	
	
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_distanceComputation);
	//double distanceComputationTime =  (((end_distanceComputation.tv_sec - start_distanceComputation.tv_sec) * 1e9)+(end_distanceComputation.tv_nsec - start_distanceComputation.tv_nsec))*1e-9;

	write_massMatrix_toFile(distance_matrix1, "./write_read_IF_demo_results/mass_matrix1.csv");
	
	std::string forest_path = "./write_read_IF_demo_results/vanillaIF";
	iForestObject->write_IF_toFile(forest_path);
	
	
	/*
	iForestObject->compute_mass();
	std::cout<<"vanillaIF compute_mass done"<<std::endl;
	
	iForestObject->write_massOFpointsTOfile("./write_read_IF_demo_results/massOfpoits_using_newIF.csv");
	
	iForestObject->compute_massRanks();
	std::cout<<"vanillaIF compute_massRanks done"<<std::endl;
	
	iForestObject->compute_neighborList();
	std::cout<<"vanillaIF compute_mass_neighborList done"<<std::endl;
	
	//iForestObject->write_massOFpointsTOfile("massOfpoits.csv");
	
	iForestObject->write_neighborListTOfile("./write_read_IF_demo_results/neighborList_using_newIF.csv");
	
	
	
	std::cout<<"vanillaIF write_IF_toFile done"<<std::endl;
	*/
	delete(iForestObject);
	
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
	iForestObject = new vanillaIF(data_matrix);
	
	iForestObject->read_IF_fromFile(forest_path);
	
	std::cout<<"vanillaIF reading done"<<std::endl;
	
	iForestObject->computeNodeMass();
	std::cout<<"nodemass computation done"<<std::endl;
	
	
	/*distance computation*/
	/*struct timespec start_distanceComputation,end_distanceComputation;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_distanceComputation);
	*/
	
	std::vector<std::vector<double>> distance_matrix2;
	
	iForestObject->computeMassMatrix(distance_matrix2);
	std::cout<<"mass matrix coputation donae"<<std::endl;
	
	
	
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_distanceComputation);
	//double distanceComputationTime =  (((end_distanceComputation.tv_sec - start_distanceComputation.tv_sec) * 1e9)+(end_distanceComputation.tv_nsec - start_distanceComputation.tv_nsec))*1e-9;

	write_massMatrix_toFile(distance_matrix2, "./write_read_IF_demo_results/mass_matrix2.csv");
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
	/*
	sMPC *sMPCObject = new sMPC(data_matrix);
	
	sMPCObject->localMass();
	
	sMPCObject->write_density("localmass.csv");
	
	sMPCObject->compute_NNHD_using_neighborList();
	
	sMPCObject->write_nnhd_nnhdDis("nnhd_nnhdDist.csv");
	
	sMPCObject->find_k_clustercenters(num_Clusters);
	
	sMPCObject->write_ClusterCenters("clusterCEnters.csv");
	
	sMPCObject->clusterAssignment();
	
	sMPCObject->write_cId("cIds.csv");
	*/
	
	
	
	
	
	
	/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*
	iForestObject->computeNodeMass();
	std::cout<<"nodemass computation done"<<std::endl;
	
	
	//distance computation
	
	struct timespec start_distanceComputation,end_distanceComputation;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_distanceComputation);
	
	
	std::vector<std::vector<double>> distance_matrix;
	
	iForestObject->computeMassMatrix(distance_matrix);
	std::cout<<"mass matrix coputation donae"<<std::endl;
	
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_distanceComputation);
	double distanceComputationTime =  (((end_distanceComputation.tv_sec - start_distanceComputation.tv_sec) * 1e9)+(end_distanceComputation.tv_nsec - start_distanceComputation.tv_nsec))*1e-9;

	write_massMatrix_toFile(distance_matrix, "mass_matrix.csv");
	
	*/
	
	
	return 0;
}


















