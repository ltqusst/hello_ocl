#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "utils.h"

void run_hello(cl_platform_id platform_id);
void run_myGEMM(cl_platform_id platform_id, const char * kernel_filename);

int main(int argc, char * argv[])
{
	int cnt, i;
	cl_platform_id platform_id = NULL;
	/* select platform or just show all of them*/
	if(argc > 1) {
		i = atoi(argv[1]);
		if(clutl_platform_select(i, &platform_id))
			fprintf(stderr, "clutl_platform_select error\n"), exit(1);
	}else{
		cnt = clutl_platform_select(-1, &platform_id);
		printf("=======================\n");
		printf("Please select platform:");
		i = fgetc(stdin) - '0';

		if(i < 1 || i> cnt ) exit(0);

		if(clutl_platform_select(i, &platform_id))
			fprintf(stderr, "clutl_platform_select error\n"), exit(1);
	}

	run_hello(platform_id);

	run_myGEMM(platform_id, argc > 2?argv[2]:"myGEMM1");

    return 0;
}

