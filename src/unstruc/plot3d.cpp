#include "plot3d.h"

#include "block.h"
#include "error.h"
#include "math.h"
#include "grid.h"

#include <iostream>
#include <fstream>
#include <string>
#include <array>

namespace unstruc {

  MultiBlock plot3d_read_to_multiblock(const std::string& filename) {
    std::ifstream f;
    std::cerr << "Opening Block File '" << filename << "'" << std::endl;
    f.open(filename.c_str(),std::ios::in|std::ios::binary);
    if (!f.is_open()) fatal("Could not open file");
    size_t n_blocks;
    float temp_f, temp_f2;
    f.read((char *) &n_blocks,4);
    std::vector<std::array<size_t,3>> dim(n_blocks);
    std::cerr << n_blocks << " blocks" << std::endl;
    std::cerr << "Dimensions: " << std::endl;
    for (size_t i = 0; i < n_blocks; i++) {
      f.read((char *) &dim[i],12);
      std::cerr << dim[i][0] << " " << dim[i][1] << " " << dim[i][2] << std::endl;
    }
    for (size_t ib = 0; ib < n_blocks; ib++) {
      if (dim[ib][0] > 10000) {
        std::cerr << "Something is wrong with idir in block " << ib << std::endl;
        fatal();
      }
      if (dim[ib][1] > 10000) {
        std::cerr << "Something is wrong with jdir in block " << ib << std::endl;
        fatal();
      }
      if (dim[ib][2] > 10000) {
        std::cerr << "Something is wrong with kdir in block " << ib << std::endl;
        fatal();
      }
    }

    f.read((char *) &temp_f,sizeof(temp_f));
    f.read((char *) &temp_f2,sizeof(temp_f2));
    bool is_float = !(fabs(temp_f) > 1e8 || fabs(temp_f2) > 1e8);
    f.seekg(-2*int(sizeof(temp_f)),std::ios_base::cur);

    MultiBlock mb;
    for (size_t ib = 0; ib < n_blocks; ib++) {
      Block blk = Block(dim[ib][0],dim[ib][1],dim[ib][2]);
      for (size_t l = 0; l < 3; l++) {
        for (size_t k = 0; k < dim[ib][2]; k++) {
          for (size_t j = 0; j < dim[ib][1]; j++) {
            for (size_t i = 0; i < dim[ib][0]; i++) {
              if (is_float) {
                f.read((char *) &temp_f,sizeof(temp_f));
                *blk.at_ref(i,j,k,l) = temp_f;
              } else {
                f.read((char *) blk.at_ref(i,j,k,l),sizeof(double));
              }
            }
          }
        }
      }
      mb.blocks.push_back(blk);
    }
    return mb;
  }

  Grid plot3d_read(const std::string& filename) {
    MultiBlock mb = plot3d_read_to_multiblock(filename);
    return mb.to_grid();
  }

} // namespace unstruc
