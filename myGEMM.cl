
//a simpler one
__kernel void myAdd(const int M, const int N, const int K,
                      const __global float* A,
                      const __global float* B,
                      __global float* C)
{
    // Thread identifiers
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)
 
    // Store the result
    C[globalCol*M + globalRow] = A[globalCol*M + globalRow] + B[globalCol*M + globalRow];
}
/********************************************************
 column-wised storing of matrix
********************************************************/
// First naive implementation
__kernel void myGEMM1(const int M, const int N, const int K,
                      const __global float* A,
                      const __global float* B,
                      __global float* C) {
    
    // Thread identifiers
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)
 
    // Compute a single element (loop over K)
    float acc = 0.0f;
    for (int k=0; k<K; k++) {
        acc += A[k*M + globalRow] * B[globalCol*K + k];
    }
 
    // Store the result
    C[globalCol*M + globalRow] = acc;
}


/********************************************************
 myGEMM1b is much slower than myGEMM1
 because the way OpenCL dispatch work-items:
   
     within a work-group, items are executed dimension by dimension, 
     first along dim0, then dim1 and dim2.
   
 so  in ver1 A&C both cached-hitted throughout adjacent work-items
 but in ver1b, only B is cached-hitted throughout adjacent work-items   
********************************************************/
__kernel void myGEMM1b(const int M, const int N, const int K,
                      const __global float* A,
                      const __global float* B,
                      __global float* C) {
    
    // Thread identifiers
    const int globalRow = get_global_id(1); // Row ID of C (0..M)
    const int globalCol = get_global_id(0); // Col ID of C (0..N)
 
    // Compute a single element (loop over K)
    float acc = 0.0f;
    for (int k=0; k<K; k++) {
        acc += A[k*M + globalRow] * B[globalCol*K + k];
    }
 
    // Store the result
    C[globalCol*M + globalRow] = acc;
}





