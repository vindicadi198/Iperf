#include <stdio.h>
#include "iperf_defaults.h"

int main(int argc, char **argv){
	struct iperf_test test;
	set_defaults(&test);
	parse_args(&test,argc,argv);
	return 0;
}
