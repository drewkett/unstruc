
#include "tetgen.h"


extern "C"
{
  typedef struct tetgenio tetgenio;
  tetgenio* new_tetgenio() {
    return new tetgenio();
  }

  void set_mesh_dim(tetgenio* tg, int mesh_dim) {tg->mesh_dim = mesh_dim;}
  void set_firstnumber(tetgenio* tg, int firstnumber) {tg->firstnumber = firstnumber;}
  void set_numberofpoints(tetgenio* tg, int numberofpoints) {tg->numberofpoints = numberofpoints;}
  void set_pointlist(tetgenio* tg, REAL *pointlist) {tg->pointlist = pointlist;}
}
