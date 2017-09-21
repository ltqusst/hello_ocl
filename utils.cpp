/*
 * utils.c
 *
 *  Created on: Sep 14, 2017
 *      Author: hddls
 */


#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <sys/time.h>

char * load_src(const char *fileName)
{
#define MAX_SOURCE_SIZE (0x100000)
	char * source_str = NULL;
	FILE *fp = fopen(fileName, "r");
	size_t source_size = 0;
    if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		return NULL;
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    source_str[source_size] = 0;
    fclose(fp);
    return source_str;
}
int load_bin(const char *filename, void * data, int size)
{
	int ret = 0;
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL) return 0;
	ret = fread(data, 1, size, fp);
	fclose(fp);
	return ret;
}
int save_bin(const char *filename, void * data, int size)
{
	int ret = 0;
	FILE *fp = fopen(filename, "wb");
	if(fp == NULL) return 0;
	ret = fwrite(data, 1, size, fp);
	fclose(fp);
	return ret;
}
void show_runtime_map(void)
{
	char map_line[1024];
	FILE * fp = fopen("/proc/self/maps","rb");
	while(fgets(map_line, sizeof(map_line), fp))
		printf(map_line);
	fclose(fp);
}

//=======================================================================

/*
 * Given a cl code and return a string represenation
 */
const char* clutl_GetErrorString(int errorCode) {
    switch (errorCode) {
        case 0: return "CL_SUCCESS";
        case -1: return "CL_DEVICE_NOT_FOUND";
        case -2: return "CL_DEVICE_NOT_AVAILABLE";
        case -3: return "CL_COMPILER_NOT_AVAILABLE";
        case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5: return "CL_OUT_OF_RESOURCES";
        case -6: return "CL_OUT_OF_HOST_MEMORY";
        case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8: return "CL_MEM_COPY_OVERLAP";
        case -9: return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15: return "CL_COMPILE_PROGRAM_FAILURE";
        case -16: return "CL_LINKER_NOT_AVAILABLE";
        case -17: return "CL_LINK_PROGRAM_FAILURE";
        case -18: return "CL_DEVICE_PARTITION_FAILED";
        case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66: return "CL_INVALID_COMPILER_OPTIONS";
        case -67: return "CL_INVALID_LINKER_OPTIONS";
        case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
        case -69: return "CL_INVALID_PIPE_SIZE";
        case -70: return "CL_INVALID_DEVICE_QUEUE";
        case -71: return "CL_INVALID_SPEC_ID";
        case -72: return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
        case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
        case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
        case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
        case -1006: return "CL_INVALID_D3D11_DEVICE_KHR";
        case -1007: return "CL_INVALID_D3D11_RESOURCE_KHR";
        case -1008: return "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1009: return "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR";
        case -1010: return "CL_INVALID_DX9_MEDIA_ADAPTER_KHR";
        case -1011: return "CL_INVALID_DX9_MEDIA_SURFACE_KHR";
        case -1012: return "CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR";
        case -1013: return "CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR";
        case -1093: return "CL_INVALID_EGL_OBJECT_KHR";
        case -1092: return "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR";
        case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
        case -1057: return "CL_DEVICE_PARTITION_FAILED_EXT";
        case -1058: return "CL_INVALID_PARTITION_COUNT_EXT";
        case -1059: return "CL_INVALID_PARTITION_NAME_EXT";
        case -1094: return "CL_INVALID_ACCELERATOR_INTEL";
        case -1095: return "CL_INVALID_ACCELERATOR_TYPE_INTEL";
        case -1096: return "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL";
        case -1097: return "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL";
        case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case -1098: return "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL";
        case -1099: return "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL";
        case -1100: return "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL";
        case -1101: return "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL";
        default: return "CL_UNKNOWN_ERROR";
    }
}

void
clutl_device_caps(cl_device_id device)
{
	size_t val_sz;
	char val[1024];
	size_t n;
	cl_uint wi_maxdim;
	size_t *wi_sz;
	cl_uint val_uint;
	cl_device_fp_config fpcfg;
	cl_ulong lc_memsz, gl_memsz;
	int rshift = 0;

#define PRINTINFO(name, fmt, rvalue) \
		clGetDeviceInfo(device, name, sizeof(*(rvalue)), rvalue, &n); \
		printf("%-40s: " fmt "\n",#name, *(rvalue));

#define PRINTINFO2(name, fmt, rvalue, rshift) \
		clGetDeviceInfo(device, name, sizeof(*(rvalue)), rvalue, &n); \
		printf("%-40s: " fmt "\n",#name, (*(rvalue) >> rshift));

	PRINTINFO(CL_DEVICE_NAME, "%s", &val);
	PRINTINFO(CL_DEVICE_MAX_COMPUTE_UNITS, "%u", &val_uint);
	PRINTINFO(CL_DEVICE_MAX_CLOCK_FREQUENCY, "%u", &val_uint);
	PRINTINFO(CL_DEVICE_ADDRESS_BITS, "%u", &val_uint);
	// work group size limit: 512(8EU * 7threads * SIMD-8)
	PRINTINFO(CL_DEVICE_MAX_WORK_GROUP_SIZE, "%u \t(upper limit of product(work_group_size))", &val_sz);
	PRINTINFO(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "%u \t(upper limit of dim(work_group_size))", &wi_maxdim);

	wi_sz = (size_t *)malloc(sizeof(size_t) * wi_maxdim);
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * wi_maxdim, wi_sz, &n);
	printf("%-40s: ", "CL_DEVICE_MAX_WORK_ITEM_SIZES");
	for(int i=0;i<n/sizeof(size_t);i++)
		printf("%u ", wi_sz[i]);
	printf("\t(uper limit of each dim of work_group_size)\n");
	free(wi_sz);

	PRINTINFO2(CL_DEVICE_LOCAL_MEM_SIZE, "%6lu KB  (Gen9 should have 64KB shared local mem)", &lc_memsz, 10)
	PRINTINFO2(CL_DEVICE_GLOBAL_MEM_SIZE, "%6lu MB  (Limited only by GPU's address bits)", &gl_memsz, 20)

	PRINTINFO(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, "%u (SIMD)", &val_uint);
	PRINTINFO(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, "%u (SIMD)", &val_uint);
	PRINTINFO(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, "%u (SIMD)", &val_uint);
	PRINTINFO(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, "%u (SIMD)", &val_uint);
	PRINTINFO(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, "%u (SIMD)", &val_uint);

	PRINTINFO(CL_DEVICE_EXTENSIONS, "%s", &val);
}

int
clutl_build_program(
		cl_context context,
		cl_device_id device,
		const char * filename,
		cl_program *ptr_program)
{
	// Compile the kernel
	const char * kernelstring = load_src(filename);
	if(kernelstring == NULL){
		fprintf(stderr, ">>> clutl_build_kernel: load_src(%s) error\n", filename);
		return 1;
	}

	cl_program program = clCreateProgramWithSource(context, 1, &kernelstring, NULL, NULL);
	clBuildProgram(program, 0, NULL, "", NULL, NULL);

	free((void*)kernelstring);

	// Check for compilation errors
	size_t logSize;
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	char* messages = (char*)malloc((1+logSize)*sizeof(char));
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, messages, NULL);
	messages[logSize] = '\0';
	if (logSize > 10) { fprintf(stderr, ">>> Compiler message: %s\n", messages); }
	free(messages);

	*ptr_program = program;
	return 0;
}

int
clutl_platform_select(
		int select_id,
		cl_platform_id * ptr_platform_id)
{
	cl_int ret;
	cl_uint ret_num_platforms;
	cl_platform_id platform_id[32]={0};
	cl_uint i,j;
	char *info;
	size_t infoSize;
#define ID_NAME(id) {id, #id}
	struct attr_tag{
		cl_platform_info id;
		const char * name;
	}attrs[]={
		ID_NAME(CL_PLATFORM_NAME),
		ID_NAME(CL_PLATFORM_VENDOR),
		ID_NAME(CL_PLATFORM_VERSION),
		ID_NAME(CL_PLATFORM_PROFILE),
		ID_NAME(CL_PLATFORM_EXTENSIONS),
	};

    /* Get Platform and Device Info */
    clGetPlatformIDs(sizeof(platform_id)/sizeof(platform_id[0]),
    				platform_id,
					&ret_num_platforms);

	printf("Total %d Platforms:\n", ret_num_platforms);
	for(i=0; i<ret_num_platforms; i++) {
		if(select_id == (i+1) || select_id <= 0){
			printf("PLATFORM %u: %s\n", i+1, select_id == (i+1)?"is selected":"");
			for (j = 0; j < sizeof(attrs)/sizeof(attrs[0]); j++) {
				clGetPlatformInfo(platform_id[i], attrs[j].id, 0, NULL, &infoSize);
				info = (char*) malloc(infoSize);
				clGetPlatformInfo(platform_id[i], attrs[j].id, infoSize, info, &infoSize);
				printf("\t%-23s : %s\n",attrs[j].name, info);
				free(info);
			}
		}
	}

    printf("\n\n");

	if(select_id > 0){
		*ptr_platform_id = platform_id[select_id - 1];
		return 0;
	}else{
		*ptr_platform_id = 0;
		return ret_num_platforms;
	}
}

/* first dimension is continuously allocated in memory
 * OR we say its stored column-by-column
 * A: M*K
 * B: K*N
 * C: M*N
 * */
void matmult(float *A, float *B, float * C, int M, int K, int N)
{
	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			float acc = 0.0f;
			for (int k=0; k<K; k++) {
				acc += A[k*M + m] * B[n*K + k];
			}
			C[n*M + m] = acc;
		}
	}
}

int matcmp(float *A, float *B, int M, int N)
{
	int ecnt = 0;
	for (int m=0; m<M; m++) {
		for (int n=0; n<N; n++) {
			if(A[m + n*M] != B[m + n*M])
				ecnt ++;
		}
	}
	return ecnt;
}

double gettime_sec(void)
{
    // Timers
    struct timeval Tvalue;
    struct timezone dummy;

	gettimeofday(&Tvalue, &dummy);

	return (double)Tvalue.tv_sec + 1.0e-6*((double)Tvalue.tv_usec);
}

