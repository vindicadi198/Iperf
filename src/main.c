#include <stdio.h>
#include "iperf_api.h"

int main(int argc, char **argv){
	struct iperf_test test;
	set_defaults(&test);
	parse_args(&test,argc,argv);
	test.execute(&test);
	destroy(&test);
	return 0;
}
