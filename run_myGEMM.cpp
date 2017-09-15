/*
 * run_myGEMM.c
 *
 *  Created on: Sep 14, 2017
 *      Author: hddls
 */
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "utils.h"
#include <sys/time.h>

// Repeat all kernels multiple times to get an average timing result
#define NUM_RUNS 10

// Size of the matrices - K, M, N (squared)
#define SIZE (4096/4)

// Threadblock sizes (e.g. for kernels myGEMM1 or myGEMM2)
#define TS 32

void run_myGEMM(cl_platform_id platform_id, const char * kernel_filename)
{
    cl_device_id device_id = NULL;
    cl_program program = NULL;

    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    double tbase;

    // Set the sizes
    int K = SIZE;
    int M = SIZE;
    int N = SIZE;

    // Create the matrices and initialize them with random values
    float* A = (float*)malloc(M*K*sizeof(float*));
    float* B = (float*)malloc(K*N*sizeof(float*));
    float* C = (float*)malloc(M*N*sizeof(float*));
    float* D = (float*)malloc(M*N*sizeof(float*) + 1);
    for (int i=0; i<M*K; i++) { A[i] = 3.6*i + i*i + 3.1; }
    for (int i=0; i<K*N; i++) { B[i] = 1.2*i + 0.01*i*i + 13.9; }
    for (int i=0; i<M*N; i++) { C[i] = 0.0; }

    const char * cl_filename = "./myGEMM.cl";

	printf("run_myGEMM() with %s...start \n", cl_filename);
	printf("CPU version start ... \n");
	tbase = gettime_sec();

	if(load_bin("myGEMM.matD", D, (M*N + 1)*sizeof(float*)) <= 0){
		matmult(A,B,D,M,K,N);
		D[M*N] = (float)(gettime_sec() - tbase);
		save_bin("myGEMM.matD", D, (M*N + 1)*sizeof(float*));
	}

	printf("CPU version complete %.3f sec\n", D[M*N]);

    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    clutl_device_caps(device_id);

    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret); /* Create OpenCL context */
    cl_command_queue queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret); /* Create Command Queue */

    if(clutl_build_program(context, device_id,	cl_filename, &program))
    	fprintf(stderr,"clutl_build_program error\n"),exit(1);

    // Prepare OpenCL memory objects
    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  M*K*sizeof(float), NULL, NULL);
    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  K*N*sizeof(float), NULL, NULL);
    cl_mem bufC = clCreateBuffer(context, CL_MEM_READ_WRITE, M*N*sizeof(float), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, M*K*sizeof(float), A, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, K*N*sizeof(float), B, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, M*N*sizeof(float), C, 0, NULL, NULL);

    // Configure the myGEMM kernel and set its arguments
    cl_kernel kernel = clCreateKernel(program, kernel_filename, &ret);
    clutl_CheckError(ret);

    clSetKernelArg(kernel, 0, sizeof(int), (void*)&M);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&N);
    clSetKernelArg(kernel, 2, sizeof(int), (void*)&K);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufA);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufC);

    cl_double g_NDRangePureExecTimeNs = 0;
    // Start the timed loop
    printf(">>> Starting %d myGEMM runs...\n", NUM_RUNS);
    double starttime = gettime_sec();
    for (int r=0; r<NUM_RUNS; r++) {

        // Run the myGEMM kernel
        const size_t local[2] = { 16, 16 };
        const size_t global[2] = { M, N };
        cl_event event = NULL;
        cl_int err;
        err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);

        clutl_CheckError(err);

        // Wait for calculations to be finished
        clWaitForEvents(1, &event);

        cl_ulong start = 0, end = 0;
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

        //END-START gives you hints on kind of “pure HW execution time”
        //the resolution of the events is 1e-09 sec
        g_NDRangePureExecTimeNs += (cl_double)(end - start);
    }

    // End the timed loop
    double endtime = gettime_sec();
    double runtime = (endtime - starttime) / (double)NUM_RUNS;

    double gflop = ((double)K * (double)M * (double)N * 2) / (1000*1000*1000);
    double gflop_add = ((double)M * (double)N) / (1000*1000*1000);
    printf(">>> Done. Host side: took %.3lf seconds per run, %.1lf GFLOPS (%.1lf GFLOPS for add)\n", runtime, gflop/runtime, gflop_add/runtime);
    printf(">>> Event Profiling: took %.3lf seconds per run\n", g_NDRangePureExecTimeNs*1e-9/(cl_double)NUM_RUNS);

    // Copy the output matrix C back to the CPU memory
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, M*N*sizeof(float), C, 0, NULL, NULL);

    int ecnt = matcmp(C, D, M, N);
    if(ecnt)
    	printf(">>> ***** %d(%d%%) errors were found in GPU result ***** \n", ecnt, ecnt*100/(M*N));

    // Free the OpenCL memory objects
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);

    // Clean-up OpenCL
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    clReleaseProgram(program);
    clReleaseKernel(kernel);

    // Free the host memory objects
    free(A);
    free(B);
    free(C);

	printf("run_myGEMM()...over \n");
}
