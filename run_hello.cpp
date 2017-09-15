/*
 * run_hello.c
 *
 *  Created on: Sep 14, 2017
 *      Author: hddls
 */
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "utils.h"

#define MEM_SIZE (128)

void run_hello(cl_platform_id platform_id)
{
	char string[MEM_SIZE]={0};
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;

    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;

	cl_int ret;

	printf("run_hello()...start \n");

    /* Get Platform and Device Info */
    //ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret); /* Create OpenCL context */

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret); /* Create Command Queue */

    if(clutl_build_program(context, device_id,	"./hello.cl", &program))
    	fprintf(stderr,"clutl_build_program error\n"),exit(1);

    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "hello", &ret);
    /* Create Memory Buffer */
    memobj = clCreateBuffer(context, CL_MEM_READ_WRITE,MEM_SIZE * sizeof(char), NULL, &ret);
    /* Set OpenCL Kernel Parameters */
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);
    /* Execute OpenCL Kernel */
    ret = clEnqueueTask(command_queue, kernel, 0, NULL,NULL);
    /* Copy results from the memory buffer */
    ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
    			MEM_SIZE * sizeof(char),string, 0, NULL, NULL);

    /* Display Result */
    puts(string);

	//show_runtime_map();

    /* Finalization */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(memobj);
    ret = clReleaseCommandQueue(command_queue);

    ret = clReleaseContext(context);

    printf("run_hello()...end \n");
}
