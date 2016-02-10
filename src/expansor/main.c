#include <stdio.h>
#include <stdlib.h>
#include "expander.h"

int main(int argc, char *argv[]){
	Expander expander;
	createExpander(argc, argv, &expander);
	getMacroInfo(&expander, argv[1]);
	expandMacros(&expander, argv[1], argv[2]);
    return 0;
}