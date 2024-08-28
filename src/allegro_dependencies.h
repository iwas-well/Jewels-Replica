#ifndef AL_DEPENDENCIES
#define AL_DEPENDENCIES

#include "structs.h"

//terminate the program if "test" parameter is 0 (NULL)
//printing message given by the "description" parameter
void must_init(bool test, const char *description);

void deallocate_allegro_structures(game_struct* game_st);

int initialize_allegro_dependencies(game_struct* game_st);

#endif
