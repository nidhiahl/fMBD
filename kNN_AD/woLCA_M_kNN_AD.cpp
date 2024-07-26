#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include "../data_loading/dataset.cpp"
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

void write_aScoreFORallK_toFile(std::vector<std::vector<double>> & aScoreArray,std::string file_path){

	std::ofstream write_aScoreFORallK_toFile(file_path.c_str(),std::ios::app|std::ios::binary);
	
	
	if(!write_aScoreFORallK_toFile){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	
	
	for(auto row:aScoreArray){
		for(auto aScore:row){
			write_aScoreFORallK_toFile<<aScore<<" ";
		}
		write_aScoreFORallK_toFile<<std::endl;
		
	}
	//write_massMatrix_toFile<<std::endl;
	write_aScoreFORallK_toFile.close();
	
}



int main( int argc, char* argv[]){
	const std::string dataset_name = argv[1];
	const std::string path_to_results = argv[2];
	const int min_K_value = atoi(argv[3]);
	const int max_K_value = atoi(argv[4]);
	std::string path_to_IF = argv[5];
	
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
	//std::cout<<"dataLoadingTime="<<dataLoadingTime<<std::endl;
	
	int dataLoadedRAM = getValue(1);
	int dataLoadedVRAM = getValue(2);
	//std::cout<<"RAM used="<<getValue(1)<<" VRAM used="<<getValue(2)<<std::endl<<std::endl;
	
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
	//std::string path_to_IF = "../stored_IF/"+dataset_name+"/vanillaIF";
	//std::string path_to_IF = "None";
	
	
	struct timespec start_iforest,end_iforest;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iforest);
	
	vanillaIF *iForestObject = new vanillaIF(data_matrix, 100, 256);
	vanillaIF &refiForestObject = *iForestObject;
	
	if (path_to_IF=="None"){iForestObject->constructVanillaIF();}
	else{iForestObject->read_IF_fromFile(path_to_IF);}

	//iForestObject->constructVanillaIF();
	//std::cout<<"iforest constructin done"<<std::endl;
	
	//std::cout<<getValue(1)<<" "<<getValue(2)<<std::endl;
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iforest);
	double iforestTime =  (((end_iforest.tv_sec - start_iforest.tv_sec) * 1e9)+(end_iforest.tv_nsec - start_iforest.tv_nsec))*1e-9;
	
	
	
	struct timespec start_massMatrix,end_massMatrix;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_massMatrix);
	
	/***********************Compute nXn mass matrix******************************/
	
	std::vector<std::vector<double>> massMatrix;
	
	
	iForestObject->computeNodeMass();
	//std::cout<<"nodemass computation done"<<std::endl;
	
	iForestObject->computeMassMatrix_NxN_woLCA(massMatrix);
	
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_massMatrix);
	double massMatrixTime =  (((end_massMatrix.tv_sec - start_massMatrix.tv_sec) * 1e9)+(end_massMatrix.tv_nsec - start_massMatrix.tv_nsec))*1e-9;
	
	
	int totalPoints = datasetObj->getnumInstances();
	
	
	
	/*Sort neighborlist of all the points*/
	struct timespec start_sortNeigh,end_sortNeigh;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_sortNeigh);
	
	for (int i = 0; i < totalPoints; i++){
		
		std::sort(massMatrix[i].begin(),massMatrix[i].end());
	}
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_sortNeigh);
	double sortNeighTime =  (((end_sortNeigh.tv_sec - start_sortNeigh.tv_sec) * 1e9)+(end_sortNeigh.tv_nsec - start_sortNeigh.tv_nsec))*1e-9;
		
	std::vector<std::vector<double>> aScore; 
	aScore.resize(totalPoints);
	
	/*Writing running time to file*/	
		
	std::string runningTimeFile_path = path_to_results+"_woLCA_massBased_runningTime.csv";
	std::ofstream write_runningTime(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
	
	if(!write_runningTime){
		std::cout<<"Can not open output file: "<<runningTimeFile_path<<std::endl;
		exit(0);
	}
	
	write_runningTime<<" dataLoadingTime iforestTime massMatrixTime sortNeighTime aScoreComputationTime TotalTime"<<std::endl;
	
	write_runningTime.close();
	
	
	for(int k = min_K_value; k <= max_K_value; k++){
		
		/*Compute Anomaly score*/
		struct timespec start_aScoreComputation,end_aScoreComputation;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_aScoreComputation);
	
		for (int i = 0; i < totalPoints; i++){
			aScore[i].push_back(massMatrix[i][k-1]);
		}
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_aScoreComputation);
		double aScoreComputationTime =  (((end_aScoreComputation.tv_sec - start_aScoreComputation.tv_sec) * 1e9)+(end_aScoreComputation.tv_nsec - start_aScoreComputation.tv_nsec))*1e-9;

		/*Writing running time to file*/	
		
		//std::string runningTimeFile_path = "./clustering_output/"+dataset_name+"/runningTime.csv";
		//std::ofstream write_runningTime(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
		
		write_runningTime.open(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
		
		if(!write_runningTime){
			std::cout<<"Can not open output file: "<<runningTimeFile_path<<std::endl;
			exit(0);
		}
			
		write_runningTime<<"k="<<k<<" ";
		
		write_runningTime<<dataLoadingTime<<" "<<iforestTime<<" "<<massMatrixTime<<" "<<sortNeighTime<<" "<<aScoreComputationTime<<" "<<dataLoadingTime+iforestTime+massMatrixTime+sortNeighTime+aScoreComputationTime<<" ";
		//write_runningTime<<dataLoadedRAM-initialRAM<<" + "<<dataLoadedVRAM-initialVRAM<<" "<<iForestRAM-dataLoadedRAM<<" + "<<iForestVRAM-dataLoadedVRAM<<" "<<distMatrixRAM-iForestRAM<<" + "<<distMatrixVRAM-iForestVRAM<<" "<<endMPCRAM-startMPCRAM<<" + "<<endMPCVRAM-startMPCVRAM<<" ";
		
		write_runningTime<<std::endl;
		write_runningTime.close();
	}	



	std::string file_path =path_to_results+"_woLCA_massBased_AScores.csv";
	std::ofstream write_allK_toFile(file_path.c_str(),std::ios::out|std::ios::binary);
	if(!write_allK_toFile){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
		
	for(int k = min_K_value; k <= max_K_value; k++){
		write_allK_toFile<<k<<" ";
	}
	write_allK_toFile<<std::endl;
	write_allK_toFile.close();
	
	write_aScoreFORallK_toFile(aScore, path_to_results+"_woLCA_massBased_AScores.csv");
	
	//write_massMatrix_toFile(massMatrix,path_to_results+"_woLCA_massMatrix.csv");
	
	return 0;
}

















