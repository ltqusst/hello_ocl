/*
 * utils.h
 *
 *  Created on: Sep 14, 2017
 *      Author: hddls
 */

#ifndef UTILS_H_
#define UTILS_H_


int
clutl_platform_select(
		int select_id,
		cl_platform_id *);

int
clutl_build_program(
		cl_context context,
		cl_device_id device,
		const char * filename,
		cl_program *ptr_program);
void
clutl_device_caps(
		cl_device_id device);

const char*
clutl_GetErrorString(
		int errorCode);

#define clutl_CheckError(errorCode) \
    if (errorCode != 0) {\
        fprintf(stderr, ">>> **** %s:%d  %s\n",__FILE__,__LINE__, clutl_GetErrorString(errorCode));\
    }\

int load_bin(const char *filename, void * data, int size);
int save_bin(const char *filename, void * data, int size);
char * load_src(const char *fileName);
void show_runtime_map(void);
void matmult(float *A, float *B, float * C, int M, int K, int N);
int matcmp(float *A, float *B, int M, int N);
double gettime_sec(void);
int clutl_check_err(cl_int ret);

#ifdef __cplusplus

#include <map>

template<typename K>
class Dtimer{
public:
	void go(void){
		tims.clear();
		start = gettime_sec();
	}

	double stop(K key){
		double tim =  gettime_sec() - start;
		tims.insert(std::pair<K, double>(key, tim));
		return tim;
	}

	double get(K key){
		return tims[key];
	}
	double operator[](K key){
		return tims[key];
	}

	double start;
	std::map<K, double> tims;
};

#endif


#endif /* UTILS_H_ */
