//a simpler one
__kernel void v1(const int M, const int N,
                      			const __global uchar* A,
                      			__global uchar* Buf)
{
    // Thread identifiers
    const int x = get_global_id(0); // Row ID of C (0..M)
    const int y = get_global_id(1); // Col ID of C (0..N)
	int R,G,B;
    int tmp1 = A[y*M + x];
    int tmp2 = A[M*N + (y>>1)*M + (x & (~1))];
    int tmp3 = A[M*N + (y>>1)*M + (x | 1)];
 	
 	tmp1 -= 16;
 	tmp2 -= 128;
 	tmp3 -= 128;
 	
 	R = ( 298 * tmp1 + 516 * tmp2 + 128) >> 8;
 	G = ( 298 * tmp1 - 100 * tmp2 - 208 * tmp3 + 128) >> 8;
 	B = ( 298 * tmp1  + 409 * tmp3 + 128) >> 8;
 
    Buf[(y*M + x)*3 + 0] = clamp(R,0,255);
    Buf[(y*M + x)*3 + 1] = clamp(G,0,255);
    Buf[(y*M + x)*3 + 2] = clamp(B,0,255);
}
