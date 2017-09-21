/*
 * run_nv12tobgr.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: hddls
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>
#include "utils.h"
#include <sys/time.h>
#include <string>

static void nv12_to_bgr24(const uint8_t *pNV12, int width, int height, uint8_t *pBGR24);

// Threadblock sizes (e.g. for kernels myGEMM1 or myGEMM2)
#define TS 32



template<typename T>
class img{
public:
	enum img_type{ NV12=0, BGR24 };

	img(int w, int h, img_type t){
		width = w;
		height = h;
		type = t;
		switch(type)
		{
		case NV12:
			size = w*h + w*(h>>1);
			break;
		case BGR24:
			size = w*h*3;
			break;
		}
		ptr = new T[size];
	}

	void clear(void){
		memset(ptr, 0, size);
	}

	~img(){
		if(ptr) delete []ptr;
	}

	//accessable by []
	T & operator[](int id){
		if(id < 0) id += size;
		return ptr[id];
	}

	//convert to T*
	operator T*(){
		return ptr;
	}

	int width;
	int height;
	int size;
	img_type type;
	T * ptr;
};




#define NUM_RUNS 1
void run_myNV12toBGR24(cl_platform_id platform_id, const char * kernel_filename)
{
    cl_device_id device_id = NULL;
    cl_program program = NULL;

    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    double tbase;

    // Set the sizes
    int M = 1920;
    int N = 1080;
    int r;

    const char * cl_filename = "./myNV12toBGR24.cl";

    // Create the matrices and initialize them with random values
    img<uint8_t> A(M, N, img<uint8_t>::NV12);
    img<uint8_t> B(M, N, img<uint8_t>::BGR24);
    img<uint8_t> D(M, N+1, img<uint8_t>::BGR24);
    for (int i=0; i<A.size; i++)
    {
    	A[i] = i & 0xFF;
    }

	printf("%s() with %s...start \n", __FUNCTION__, cl_filename);
	printf("CPU version start ... \n");
	tbase = gettime_sec();

#define MATD_CACHE_FILENAME "matD.NV12toBGR24"


	static uint32_t buff[1920*1088*3];
	float * pf = (float *)&(D[-4]);

	if((r=load_bin(MATD_CACHE_FILENAME, D.ptr, D.size)) != D.size){
		printf("\trunning...(r=%d)\n", r);
		for(int i=0;i<NUM_RUNS;i++)
			nv12_to_bgr24(A, M, N, D);
		*(pf) = (float)(gettime_sec() - tbase)/NUM_RUNS;
		save_bin(MATD_CACHE_FILENAME, D.ptr, D.size);
	}else{
		printf("\tcache file myNV12toBGR24.matD loaded\n");
	}
	printf("CPU version complete %.3f sec\n", *pf);

    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    clutl_device_caps(device_id);

    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret); /* Create OpenCL context */
    cl_command_queue queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret); /* Create Command Queue */

    if(clutl_build_program(context, device_id,	cl_filename, &program))
    	fprintf(stderr,"clutl_build_program error\n"),exit(1);
    //setenv("OCL_OUTPUT_ASM","1",0);
    //setenv("OCL_OUTPUT_BUILD_LOG","1",0);

    // Prepare OpenCL memory objects
    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  A.size, NULL, NULL);
    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  B.size, NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, A.size, A, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, B.size, B, 0, NULL, NULL);

    // Configure the myGEMM kernel and set its arguments
    cl_kernel kernel = clCreateKernel(program, kernel_filename, &ret);
    clutl_CheckError(ret);

    Dtimer<std::string> dt;
    dt.go();

    clSetKernelArg(kernel, 0, sizeof(int), (void*)&M);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&N);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufA);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufB);

    cl_double g_NDRangePureExecTimeNs = 0;
    // Start the timed loop
    printf(">>> Starting %d %s runs...\n", NUM_RUNS, __FUNCTION__);
    dt.stop("bufin");
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

    dt.stop("comp");

    // End the timed loop
    // Copy the output matrix C back to the CPU memory
    clEnqueueReadBuffer(queue, bufB, CL_TRUE, 0, B.size, B, 0, NULL, NULL);

    dt.stop("bufout");

    double gflop = ((double)M * (double)N * 3) / (1000*1000*1000);
    printf(">>> Done. Host side: took %.3lf seconds per run, %.1lf GFLOPS\n",
    		(dt["comp"] - dt["bufin"])/NUM_RUNS, gflop/(dt["comp"] - dt["bufin"]));
    printf(">>> Event Profiling: took %.3lf seconds per run\n", g_NDRangePureExecTimeNs*1e-9/(cl_double)NUM_RUNS);
    printf(">>>     detail: %.6lf seconds bufin\n", dt["bufin"]);
    printf(">>>           : %.6lf seconds comp\n", dt["comp"] - dt["bufin"]);
    printf(">>>           : %.6lf seconds bufout\n", dt["bufout"] - dt["comp"]);

    if(memcmp(B, D, B.size))
    	printf(">>> ***** errors were found in GPU result ***** \n");

    // Free the OpenCL memory objects
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);

    // Clean-up OpenCL
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    clReleaseProgram(program);
    clReleaseKernel(kernel);

	printf("%s()...over \n", __FUNCTION__);

}

/**
* brief@ nv12_to_bgr24, color space convert, fix-point calculate.
* param@ pNV12: src NV12 image
* param@ width: NV12 width
* param@ height: NV12 height
* param@ pBGR24: Out BGR24 image, size = width * height * 3
*/
#define RANGE_UCHAR(x) ((x)>255?255:((x)<0?0:(x)))

static void nv12_to_bgr24(const uint8_t *pNV12, int width, int height, uint8_t *pBGR24)
{
       int w, h, tmp1, tmp2, tmp3;
       const uint8_t* pY, *pUV;
       uint8_t* pTmpBGRBuf;
       int halfWidth = width >> 1;

       pTmpBGRBuf = pBGR24;
       pY = pNV12;
       pUV = pNV12 + width * height;
       for(h = 0; h < height; h++) {
             for(w = 0; w < width; w++) {
                    uint8_t curY = pY[0];
                    uint8_t curU = pUV[(h >> 1) * width + (w&0XFFFE)];
                    uint8_t curV = pUV[(h >> 1) * width + (w&0XFFFE) + 1];
                    tmp1 = curY - 16;
                    tmp2 = curU - 128;
                    tmp3 = curV - 128;
                    pTmpBGRBuf[0] = (uint8_t)(RANGE_UCHAR(( 298 * tmp1 + 516 * tmp2 + 128) >> 8));
                    pTmpBGRBuf[1] = (uint8_t)(RANGE_UCHAR(( 298 * tmp1 - 100 * tmp2 - 208 * tmp3 + 128) >> 8));
                    pTmpBGRBuf[2] = (uint8_t)(RANGE_UCHAR(( 298 * tmp1  + 409 * tmp3 + 128) >> 8));
                    pY++;
                    pTmpBGRBuf += 3;
             }
       }

       /* If height is not even number, need to process last line data */
       if (height % 2 == 1) {
             h = height - 1;
             pY = (pNV12 + h * width);
             pTmpBGRBuf = pBGR24 + h * width * 3;
             for (w = 0; w < width; w++) {
                    pTmpBGRBuf[0] = pTmpBGRBuf[1] = pTmpBGRBuf[2] = pY[0];
                    pTmpBGRBuf += 3;
                    pY++;
             }
             height--;
       }
}


