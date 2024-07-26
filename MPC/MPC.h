#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>



class MPC{
	private:
	const std::vector<std::vector<double>> _X;
	int _totalPoints;
	double _cutoffDistance;
	double _maxDistance;
	int _numClusters;
	std::vector<double> _density;
	std::vector<std::pair<double,int>> _sortedDensity;
	std::vector<double> _nnhdDistance;
	std::vector<std::pair<double,int>> _densityXnnhdDistance;
	std::vector<int> _nnhd;
	std::vector<int> _clusterCenters;
	std::vector<std::vector<int>> _clusters;
	std::vector<int> _cId;
	std::vector<int> _haloId;
	
	
	
	public:
	
	MPC(const std::vector<std::vector<double>> &, const double &);
	virtual ~MPC();
	void computeDensity();
	void computeNNHd();
	void findClusterCenters(int );
	void assignClusterId();
	void compute_DenXnnhdDis();
	void find_k_clustercenters(int );
	void clusterAssignment();
	void findHalo();
	double distance(int,int);
	
	void write_density(std::string ,std::string);
	void write_id_denNNdis_den_nnDis_NN(std::string ,std::string);
	void write_sortedDensity(std::string ,std::string);
	void write_sortedDensityRank(std::string, std::string);
	void write_nnhd_nnhdDis(std::string ,std::string);
	void write_ClusterCenters(std::string ,std::string);
	void write_cId(std::string ,std::string);
	void write_haloId(std::string ,std::string);

	
	
	void print_density();
	void print_id_denNNdis_den_nnDis_NN();
	void print_sortedDensity();
	void print_nnhd_nnhdDis();
	void print_ClusterCenters();
	void print_cId();
	void print_haloId();

};
