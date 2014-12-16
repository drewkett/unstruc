#include "plot3d.h"

#include "block.h"
#include "error.h"

#include <iostream>
#include <fstream>
#include <string>


MultiBlock ReadPlot3D(std::string &filename) {
	std::ifstream f;
	MultiBlock mb;
	std::cerr << "Opening Block File '" << filename << "'" << std::endl;
	f.open(filename.c_str(),std::ios::in|std::ios::binary);
	if (!f.is_open()) Fatal("Could not open file");
	int n_blocks;
	f.read((char *) &n_blocks,4);
	int dim[n_blocks][3];
	std::cerr << n_blocks << " blocks" << std::endl;
	std::cerr << "Dimensions: " << std::endl;
	for (int i = 0; i < n_blocks; i++) {
		f.read((char *) &dim[i],12);
		std::cerr << dim[i][0] << " " << dim[i][1] << " " << dim[i][2] << std::endl;
	}
	for (int ib = 0; ib < n_blocks; ib++) {
		if (dim[ib][0] > 10000) {
			std::cerr << "Something is wrong with idir in block " << ib << std::endl;
			Fatal();
		}
		if (dim[ib][1] > 10000) {
			std::cerr << "Something is wrong with jdir in block " << ib << std::endl;
			Fatal();
		}
		if (dim[ib][2] > 10000) {
			std::cerr << "Something is wrong with kdir in block " << ib << std::endl;
			Fatal();
		}
	}
	for (int ib = 0; ib < n_blocks; ib++) {
		Block blk = Block(dim[ib][0],dim[ib][1],dim[ib][2]); 
		for (int l = 0; l < 3; l++) {
			for (int k = 0; k < dim[ib][2]; k++) {
				for (int j = 0; j < dim[ib][1]; j++) {
					for (int i = 0; i < dim[ib][0]; i++) {
						f.read((char *) blk.at(i,j,k,l),sizeof(double));
					}
				}
			}
		}
		mb.blocks.push_back(blk);
	}
	return mb;
}
