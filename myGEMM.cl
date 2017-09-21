
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


#define TS 16

/***************************************************************************
 column-wised storing of matrix
 
 cache line: 
  |   @: cache hit
  V   ?: cache miss
                            N
             [B]--------+--------------+
             |          |@             |
            K|          |@             |
             |          |@             |
      K      +----------+--------------+ 
  [A]-----+  [C]-----------------------+
  |       |  |          .              |
  |       |  |          .              |
  |       |  |          .              |
 M|       |  |          .              |
  +-------+  |  ........Xi             |
  |@@@@@@@|  |          Xi+1@          |
  |@@@@@@@|  |          Xi+2@          |
  |       |  |          .              | 
  +-------+  +-------------------------+
 
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






/***************************************************************************
 myGEMM1b is much slower than myGEMM1
 because the way OpenCL dispatch work-items:
   
     within a work-group, items are executed dimension by dimension, 
     first along dim0, then dim1 and dim2.
     
     also note that work-items within a work-group will be executed
     in paralel rather than sequencially (power of GPU).
     
     but work-groups may be executed seqencially or to some extent parallel 
     
 cache line: 
  |   @: cache hit
  V   ?: cache miss
                          N
             [B]--------+--------------+
             |          | @    @       |
            K|          | @    @       |
             |          | @    @       |
      K      +----------+--------------+ 
  [A]-----+  [C]-----------------------+
  |       |  |          .              |
  |       |  |          .              |
  |       |  |          .              |
 M|       |  |          .              |
  +-------+  |  ........Xi Xi+1 Xi+2...|
  |???????|  |          ?  ?    ?      |
  |       |  |                         |
  |       |  |                         | 
  +-------+  +-------------------------+


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








/***************************************************************************
 This is a memory-demonding computation:
   for each element in C: 
   		mem access:	  2K loads + 1 store
 		computation:   K (Mutiply & Accumulate)
 		
   so it's 0.5 instruction/memory access, and is very low
   so we need to focus on memory access optimization first. 
 
   let focus on one work-group, which is TS-by-TS size
   requires K-by-TS submatrix of A and K-by-TS submatrx from B
   which is way bigger than GPU-cache capacity, so when we
   walk through TS-by-TS work-items column-by-column, submatrix
   A will be cache-out and cache-in many times so is less efficient
   also cache for A may emit cacheline of B
   
   so let's try to manage memory access manually by synchronize the
   memory access pattern of all threads in group.
   
                            N
             [B]--------+---+----------+
             |          |   |          |
            K|          |   |          |
             |          |   |          |
      K      +----------+---+----------+ 
  [A]-----+  [C]-----------------------+
  |       |  |                         |
  |       |  |                         |
  |       |  |                         |
 M|       |  |           TS            |
  +-------+  |          +---+          |
  |       |  |        TS|   |          |
  +-------+  |          +---+          |
  |       |  |                         | 
  +-------+  +-------------------------+
 
********************************************************/
__kernel void myGEMM2(const int M, const int N, const int K,
                      const __global float* A,
                      const __global float* B,
                      __global float* C) {
    
    // Thread identifiers
    const int row = get_local_id(0); // Local row ID (max: TS)
    const int col = get_local_id(1); // Local col ID (max: TS)
    const int globalRow = TS*get_group_id(0) + row; // Row ID of C (0..M)
    const int globalCol = TS*get_group_id(1) + col; // Col ID of C (0..N)
 
    // Local memory to fit a tile of TS*TS elements of A and B
    __local float Asub[TS][TS];
    __local float Bsub[TS][TS];
 
    // Initialise the accumulation register
    float acc = 0.0f;
    
    // Loop over all tiles
    const int numTiles = K/TS;
    for (int t=0; t<numTiles; t++) {
 
        // Load one tile of A and B into local memory
        const int tiledRow = TS*t + row;
        const int tiledCol = TS*t + col;
        Asub[col][row] = A[tiledCol*M + globalRow];
        Bsub[col][row] = B[globalCol*K + tiledRow];
 
        // Synchronise to make sure the tile is loaded
        barrier(CLK_LOCAL_MEM_FENCE);
 
        // Perform the computation for a single tile
        for (int k=0; k<TS; k++) {
            acc += Asub[k][row] * Bsub[col][k];
        }
 
        // Synchronise before loading the next tile
        barrier(CLK_LOCAL_MEM_FENCE);
    }
 
    // Store the final result in C
    C[globalCol*M + globalRow] = acc;
}

