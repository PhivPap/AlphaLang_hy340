#ifndef OPT_H    
#define OPT_H

#include "definitions.h"

typedef struct usedTempVars* usedTempVars;

void run_optimization_code(Quad* quads, uint currQuad, uint deadcodeIndex);

void run_final_optimization_code(Quad* quads, uint currQuad);

#endif /* OPT_H */