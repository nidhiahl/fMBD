#include "MPC.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>



MPC::MPC(const std::vector<std::vector<double>> & X, const double & cutoffDistance):_X(X), _cutoffDistance(cutoffDistance){
	_totalPoints = X.size();
	_maxDistance = 0;
}

MPC::~MPC(){}

void MPC::computeDensity(){
	_density.assign(_totalPoints,0);
	for(int point_i=0; point_i < _totalPoints-1; point_i++){
		for(int point_j = 0;point_j <= point_i; point_j++){
			double dist = distance(point_i,point_j);
			_maxDistance = std::max(_maxDistance, dist);
			if(dist < _cutoffDistance){
				_density[point_i]++;
				_density[point_j]++;
			}
		}
	}

}

double MPC::distance(int point_i, int point_j){
	return _X[point_i][point_j];
}


void MPC::computeNNHd(){
	for(int i = 0; i < _totalPoints; i++){
        _sortedDensity.push_back({(_density[i]*1.0/_totalPoints),i});
    }
    
    sort(_sortedDensity.rbegin(),_sortedDensity.rend());
    
	_nnhdDistance.resize(_totalPoints);
    _nnhd.resize(_totalPoints);
    
    
    //Initialization for highest density point
	_nnhdDistance[_sortedDensity[0].second] = _maxDistance;
	_nnhd[_sortedDensity[0].second] = -1;
    //_densityXnnhdDistance.push_back({_sortedDensity[0].first * _maxDistance,_sortedDensity[0].second});
    
    //double secondMax_densityXnnhdDistance = 0;
    
    //std::cout<<_sortedDensity[0].first<<" "<<_maxDistance<<" "<<_sortedDensity[0].first * _maxDistance<<" "<<_sortedDensity[0].second<<" "<<_nnhd[_sortedDensity[0].second]<<std::endl;
    
    for(int i = 1; i < _totalPoints; i++){
    
		double min = _maxDistance;
		int minNN;
		
		
		for(int j = 0;j < i; j++){
		double dis = _sortedDensity[i].second > _sortedDensity[j].second ? distance(_sortedDensity[i].second,_sortedDensity[j].second) : distance(_sortedDensity[j].second,_sortedDensity[i].second);
		if(dis < min){
				min = dis;
				minNN = _sortedDensity[j].second;
            }
        }
        
		_nnhdDistance[_sortedDensity[i].second] = min;
		_nnhd[_sortedDensity[i].second] = minNN;
		
		//std::cout<<_sortedDensity[i].first<<" "<<min<<" "<<_sortedDensity[i].first * min<<" "<<_sortedDensity[i].second<<" "<<minNN<<" "<<_nnhd[_sortedDensity[i].second]<<std::endl;
		
	
		//_densityXnnhdDistance.push_back({_sortedDensity[i].first * min, minNN});


		
		/*//replace NNDhDistance of highest density point with the NNHd of second cluster center
		if(_densityXnnhdDistance[i].first>secondMax_densityXnnhdDistance){
			secondMax_densityXnnhdDistance = _densityXnnhdDistance[i].first;
			_nnhdDistance[_sortedDensity[0].second] = _nnhdDistance[_sortedDensity[i].second];
		}*/
        
    }
    compute_DenXnnhdDis();
    
}

void MPC :: compute_DenXnnhdDis(){
	for(int i = 0; i < _totalPoints; i++){
		int point_i = _sortedDensity[i].second;
		_densityXnnhdDistance.push_back({_sortedDensity[i].first * _nnhdDistance[point_i], point_i});
	}
	sort(_densityXnnhdDistance.rbegin(),_densityXnnhdDistance.rend());

}





void MPC :: find_k_clustercenters(int k){
	_numClusters=k;
	_clusterCenters.resize(_numClusters); 
	for(int i = 0; i < _numClusters; i++){
		_clusterCenters[i]=_densityXnnhdDistance[i].second;
	}
}


void MPC :: clusterAssignment(){
	_clusters.resize(_numClusters);
	_cId.resize(_totalPoints,0);
	for(int cId = 0; cId < _numClusters; cId++){
		int cc = _clusterCenters[cId];
		_cId[cc] = cId+1;
		_clusters[cId].push_back(cc);
	}
	for(int i = 0; i < _totalPoints; i++){
		int currPoint = _sortedDensity[i].second;
		if(_cId[currPoint] == 0){ 
			_cId[currPoint] = _cId[_nnhd[currPoint]];
			_clusters[_cId[currPoint]-1].push_back(currPoint);
		}	
	}
	
}


void MPC :: findHalo(){
	for(int i =0;i<_totalPoints;i++){
        _haloId.push_back(_cId[i]);
    }

    if(_numClusters>1){
        std::vector<double>bord_rho(_numClusters+1,0);
        
        for(int i = 0; i < _totalPoints; i++){
            for(int j = 0; j < i; j++){
                if(_cId[i]!=_cId[j] && distance(i,j)<=_cutoffDistance){
                    double rho_aver = (_density[i]+_density[j])/2;
                    bord_rho[_cId[i]] = std::max(bord_rho[_cId[i]],rho_aver);
                    bord_rho[_cId[j]] = std::max(bord_rho[_cId[j]],rho_aver);
                }
            }
        }
        
        for(int i =0;i<_totalPoints;i++){
            if (_density[i]<bord_rho[_cId[i]]){
                _haloId[i]=0;
            }
        }
    }
}








void MPC::write_id_denNNdis_den_nnDis_NN(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_id_denNNdis_den_nnDis_NN;
	
	if(open_mode =="out"){
		write_id_denNNdis_den_nnDis_NN.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_id_denNNdis_den_nnDis_NN.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_id_denNNdis_den_nnDis_NN){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_id_denNNdis_den_nnDis_NN<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	
	
	
	for(int point = 0; point<_totalPoints;point++){
		int point_id = _densityXnnhdDistance[point].second;	
		write_id_denNNdis_den_nnDis_NN<<point_id<<" "<<_densityXnnhdDistance[point].first<<" "<<_density[point_id]<<" "<<_nnhdDistance[point_id]<<" "<<_nnhd[point_id]<<std::endl;
	}
	
	//write_id_denNNdis_den_nnDis_NN<<std::endl;
	write_id_denNNdis_den_nnDis_NN.close();

}


void MPC :: write_ClusterCenters(std::string file_path, std::string open_mode = "app"){
	std::ofstream writeClusterCenters;
	
	if(open_mode =="out"){
		writeClusterCenters.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		writeClusterCenters.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!writeClusterCenters){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	writeClusterCenters<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	for(auto cc:_clusterCenters){
		writeClusterCenters<<cc<<std::endl;
	}
	writeClusterCenters<<std::endl;
	writeClusterCenters.close();
}




void MPC :: write_cId(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_cId;
	
	if(open_mode =="out"){
		write_cId.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_cId.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_cId){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	//write_cId<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	write_cId<<_cutoffDistance<<std::endl;
	
	for(auto cid:_cId){
		write_cId<<cid<<std::endl;
	}
	
	//write_cId<<std::endl;
	write_cId.close();
}


void MPC :: write_haloId(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_haloId;
	
	if(open_mode =="out"){
		write_haloId.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_haloId.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_haloId){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_haloId<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	for(auto haloid:_haloId){
		write_haloId<<haloid<<std::endl;
	}
	
	//write_haloId<<std::endl;
	write_haloId.close();
}

void MPC::write_sortedDensityRank(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_sortedDensityRank;
	
	if(open_mode =="out"){
		write_sortedDensityRank.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_sortedDensityRank.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_sortedDensityRank){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_sortedDensityRank<<"cutoff_distance="<<_cutoffDistance<<" ";
	
	for(auto den:_sortedDensity){
		write_sortedDensityRank<<den.second<<" ";
	}
	
	write_sortedDensityRank<<std::endl;
	write_sortedDensityRank.close();
	
}



void MPC::write_sortedDensity(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_sortedDensity;
	
	if(open_mode =="out"){
		write_sortedDensity.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_sortedDensity.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_sortedDensity){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_sortedDensity<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	for(auto den:_sortedDensity){
		write_sortedDensity<<den.second<<" "<<den.first<<std::endl;
	}
	
	//write_sortedDensity<<std::endl;
	write_sortedDensity.close();
	
}


void MPC::write_nnhd_nnhdDis(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_nnhd_nnhdDis;
	
	if(open_mode =="out"){
		write_nnhd_nnhdDis.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_nnhd_nnhdDis.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_nnhd_nnhdDis){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_nnhd_nnhdDis<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	for(int i =0;i<_totalPoints;i++){
		write_nnhd_nnhdDis<<_nnhd[i]<<" "<<_nnhdDistance[i]<<std::endl;
	}
	
	//write_nnhd_nnhdDis<<std::endl;
	write_nnhd_nnhdDis.close();
	

}




void MPC::write_density(std::string file_path, std::string open_mode = "app"){
	std::ofstream write_density;
	
	if(open_mode =="out"){
		write_density.open(file_path.c_str(),std::ios::out|std::ios::binary) ;
	}else if(open_mode =="app"){
		write_density.open(file_path.c_str(),std::ios::app|std::ios::binary) ;
	}else{
		std::cout<<"You have entered inappropriate file opening mode to write in file"<<file_path<<std::endl;
		exit(0);
	}
	
	if(!write_density){
		std::cout<<"Can not open output file: "<<file_path<<std::endl;
		exit(0);
	}
	
	write_density<<"cutoff_distance="<<_cutoffDistance<<std::endl;
	
	for(auto den:_density){
		write_density<<den<<std::endl;
	}
	//write_density<<std::endl;
	write_density.close();
	
}















void MPC::print_id_denNNdis_den_nnDis_NN(){
	for(int point = 0; point<_totalPoints;point++){
	int point_id = _sortedDensity[point].second;	
	std::cout<<point_id<<"_"<<_densityXnnhdDistance[point].first<<" "<<_densityXnnhdDistance[point_id].second<<"_"<<_density[point_id]<<"_"<<_nnhdDistance[point_id]<<"_"<<_nnhd[point_id]<<std::endl;
		
	}

}


void MPC :: print_ClusterCenters(){
	for(auto cc:_clusterCenters){
		std::cout<<cc<<std::endl;
	}
}


void MPC :: print_cId(){
	for(auto cid:_cId){
		std::cout<<cid<<std::endl;
	}
}


void MPC :: print_haloId(){
	for(auto haloid:_haloId){
		std::cout<<haloid<<std::endl;
	}
}





void MPC::print_sortedDensity(){
	std::cout<<"--------------print sorted density-------------"<<std::endl;
	for(auto den:_sortedDensity){
		std::cout<<den.second<<" "<<den.first<<std::endl;
	}
}


void MPC::print_nnhd_nnhdDis(){
	std::cout<<"--------------print nnhd_nnhdDis-------------"<<std::endl;
	for(int i =0;i<_totalPoints;i++){
		std::cout<<_nnhd[i]<<" "<<_nnhdDistance[i]<<std::endl;
	}
}




void MPC::print_density(){
	std::cout<<"--------------print density-------------"<<std::endl;
	
	for(auto den:_density){
		std::cout<<den<<std::endl;
	}
}
