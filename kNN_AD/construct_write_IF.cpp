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

int main( int argc, char* argv[]){
	
	const std::string dataset_name = argv[1];
	std::string forest_path = argv[2];
	
	/*Writing running time to file*/	
		
	std::string runningTimeFile_path = "./stored_IF/"+dataset_name+"/vanilla_IF/runningTime.csv";
	std::ofstream write_runningTime(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
	
	if(!write_runningTime){
		std::cout<<"Can not open output file: "<<runningTimeFile_path<<std::endl;
		exit(0);
	}
	
	//write_runningTime<<"IF_run dataLoadingTime iforestTime TotalTime"<<std::endl;
	
	write_runningTime.close();
	
	
	/*data loading */
	struct timespec start_dataLoading,end_dataLoading;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_dataLoading);
		
	const std::string datafile_path = "./datasets/"+dataset_name+"/"+dataset_name+".csv";
	dataset *datasetObj = new dataset(datafile_path);
    
    std::vector<std::vector<double>> data_matrix;
	datasetObj->getDataMatrix(data_matrix);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_dataLoading);
	double dataLoadingTime =  (((end_dataLoading.tv_sec - start_dataLoading.tv_sec) * 1e9)+(end_dataLoading.tv_nsec - start_dataLoading.tv_nsec))*1e-9;
	
	/*isolation forest construction*/
	
	struct timespec start_iForest,end_iForest;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_iForest);
	vanillaIF *iForestObject;
	//std::string forest_path = "./stored_IF/"+dataset_name+"/vanillaIF/IF_"+run;
	
	iForestObject = new vanillaIF(data_matrix, 100, 300);
	
	iForestObject->constructVanillaIF();
	//std::cout<<"vanillaIF constructin done"<<std::endl;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_iForest);
	double iForestTime =  (((end_iForest.tv_sec - start_iForest.tv_sec) * 1e9)+(end_iForest.tv_nsec - start_iForest.tv_nsec))*1e-9;
	
	write_runningTime.open(runningTimeFile_path.c_str(),std::ios::app|std::ios::binary) ;
		
	if(!write_runningTime){
		std::cout<<"Can not open output file: "<<runningTimeFile_path<<std::endl;
		exit(0);
	}
		
	
	write_runningTime<<dataLoadingTime<<" "<<iForestTime<<" "<<dataLoadingTime+iForestTime<<" ";
		
	write_runningTime<<std::endl;
	write_runningTime.close();
	
	iForestObject->write_IF_toFile(forest_path);
	
	
	delete(iForestObject);
	
	
	
	return 0;
	
}
