
#include "tetgen.h"


extern "C"
{
  typedef struct tetgenio tetgenio;
  tetgenio* new_tetgenio() {
    return new tetgenio();
  }
}
