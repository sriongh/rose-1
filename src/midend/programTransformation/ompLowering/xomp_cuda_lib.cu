/* A simple lib to facilitate CUDA coding 

Extensions to the xomp interface

Liao 4/11/2012
*/

//#include "cuda_lib.h"
#include "libxomp.h"
//----------------------------------------------------
// Device xomp_cuda_property retrieving functions

extern struct DDE_data * DDE_head;
extern struct DDE_data * DDE_tail;

cudaDeviceProp* xomp_cuda_prop = NULL; 
bool xomp_verbose = false;

// this can be called multiple times. But the xomp_cuda_prop variable will only be set once
cudaDeviceProp * xomp_getCudaDeviceProp()
{
  if (xomp_cuda_prop == NULL )
  {
    xomp_cuda_prop = (cudaDeviceProp *) malloc(sizeof(cudaDeviceProp));
    assert (xomp_cuda_prop != NULL);
    int count;
    cudaGetDeviceCount (&count);
    assert (count>=1); // must have at least one GPU here
    cudaGetDeviceProperties  (xomp_cuda_prop, 0);
  }
  return xomp_cuda_prop;
}

void xomp_print_gpu_info()
{
  int max_threads_per_block = xomp_getCudaDeviceProp()->maxThreadsPerBlock;
  int max_blocks_per_grid_x = xomp_getCudaDeviceProp()->maxGridSize[0];
  int global_memory_size =    xomp_getCudaDeviceProp()->totalGlobalMem;
  int shared_memory_size =    xomp_getCudaDeviceProp()->sharedMemPerBlock;
  int registers_per_block =   xomp_getCudaDeviceProp()->regsPerBlock;

  printf ("Found a GPU with \n\tmax threads per block=%d, \n\tmax blocks for Grid X dimension=%d\n\
      \tglobal mem bytes =%d, \n\tshared mem bytes =%d, \n\tregs per block = %d\n",
      max_threads_per_block, max_blocks_per_grid_x, global_memory_size ,  shared_memory_size,
      registers_per_block);
}
// A helper function to probe physical limits based on GPU Compute Capability numbers
// Reference: http://developer.download.nvidia.com/compute/cuda/CUDA_Occupancy_calculator.xls
size_t xomp_get_maxThreadBlocksPerMultiprocessor()
{
  int major, minor; 
  major = xomp_getCudaDeviceProp()-> major;
  minor = xomp_getCudaDeviceProp()-> minor;
  if (major <= 2) //1.x and 2.x: 8 blocks per multiprocessor
    return 8;
  else if (minor <= 5)
    return 16;
  else
  {
   printf("Error: xomp_get_maxThreadBlocksPerMultiprocessor(): unhandled Compute Capability numbers%d.%d \n", major, minor);
   assert (false);
  }
  assert (false);
  return 0;
}

// max thread per block, useful for 1-D problem
// The goal is to maximize GPU occupancy for each multiprocessor : physical max warps 
// Reference: http://developer.download.nvidia.com/compute/cuda/CUDA_Occupancy_calculator.xls
//
// Two physical limits are considered for now
//  1) max-active-threads per multiprocessor 
//  2) max active thread blocks per multiprocessor
// So for 1-D block, max threads per block = maxThreadsPerMultiProcessor /  maxBlocks per multiprocessor
size_t xomp_get_maxThreadsPerBlock()
{
  // this often causes oversubscription to the cores supported by GPU SM processors
  //return xomp_getCudaDeviceProp()->maxThreadsPerBlock;
  //return 128;
  // 2.0: 1536/8= 192 threads per block
  // 3.5 2048/16 = 128
  return xomp_getCudaDeviceProp()->maxThreadsPerMultiProcessor / xomp_get_maxThreadBlocksPerMultiprocessor();
}

/*
* In order to ensure best performance, we setup max_block limitation here, so that each core in the GPU works on only one threads.
* Use XOMP_accelerator_loop_default() runtime to support input data size that exceeds max_block*xomp_get_maxThreadsPerBlock().  
*/
size_t xomp_get_max1DBlock(size_t s)
{
#if 1  
  size_t block_num = s/xomp_get_maxThreadsPerBlock();
  if (s % xomp_get_maxThreadsPerBlock()!= 0)
     block_num ++;
  //return block_num;     

  size_t max_block = xomp_getCudaDeviceProp()->multiProcessorCount* xomp_get_maxThreadBlocksPerMultiprocessor();

  return block_num<max_block? block_num: max_block; 

  /* max threads per multiprocessor / threads-per-block  * num_multiprocessor */
  //return xomp_getCudaDeviceProp()->multiProcessorCount*(xomp_getCudaDeviceProp()->maxThreadsPerMultiProcessor /xomp_get_maxThreadsPerBlock()) ;
  //return xomp_getCudaDeviceProp()->maxThreadsPerMultiProcessor /xomp_get_maxThreadsPerBlock() ;
#else
  return xomp_getCudaDeviceProp()->multiProcessorCount* xomp_get_maxThreadBlocksPerMultiprocessor();
#endif
}

// Get the max number threads for one dimension (x or y) of a 2D block
// Two factors are considered: the total number of threads within the 2D block must<= total threads per block
//  x * y <= maxThreadsPerBlock 512 or 1024
// each dimension: the number of threads must <= maximum x/y-dimension
//    x <= maxThreadsDim[0],  1024
//    y <= maxThreadsDim[1], 1024 
//  maxThreadsDim[0] happens to be equal to  maxThreadsDim[1] so we use a single function to calculate max segments for both dimensions
size_t xomp_get_max_threads_per_dimesion_2D ()
{

  int max_threads_per_block = xomp_getCudaDeviceProp()->maxThreadsPerBlock;
  // we equalize the number of threads in each dimension
  int max_threads_per_2d_dimension = (int)(sqrt((float)max_threads_per_block));  
  assert (max_threads_per_2d_dimension*max_threads_per_2d_dimension<= max_threads_per_block);

  // our assumption is that dim[0] == dim[1] so we handle x and y in one function
  assert ( xomp_getCudaDeviceProp()->maxThreadsDim[0] == xomp_getCudaDeviceProp()->maxThreadsDim[1]);   
  assert (max_threads_per_2d_dimension <= xomp_getCudaDeviceProp()->maxThreadsDim[0]);
  return max_threads_per_2d_dimension;
}

// return the max number of segments for a dimension (either x or y) of a 2D block
// we define the number of segments to be  SIZE_of_Dimension_x/max_threads_x_dimension
size_t xomp_get_maxSegmentsPerDimensionOf2DBlock(size_t dimension_size)
{
  // For simplicity, we don't yet consider the factor of warp size for now
  // TODO: block size should be divisible by the warp size??
  // e.g. max threads per block is 1024, then max number of tiles per dimension in a 2D block is 1024^0.5 = 32 threads
  size_t max_threads_per_2d_dimension = xomp_get_max_threads_per_dimesion_2D ();
  size_t block_num_x_or_y =  dimension_size/max_threads_per_2d_dimension;
  if (dimension_size % max_threads_per_2d_dimension != 0)
     block_num_x_or_y ++;

  return block_num_x_or_y;
}

/*-----------------------------------------------------
   Device memory allocation functions 
*/
void* xomp_deviceMalloc(size_t size)
{
  void * devPtr;
  cudaError_t rt =  cudaMalloc(&devPtr, size);
  if ( (size !=0) && (rt == cudaSuccess) )
  {
    return devPtr;
  }
  else
  {
    fprintf(stderr, "Error: cudaMalloc() failed to allocate the requested %d bytes!\n",size );
    assert (false);
    return NULL; // it is a bad idea to silently return a NULL pointer
  }
}

// A host version
void* xomp_hostMalloc(size_t size)
{
  assert (size>0);
  void* hostPtr;
  hostPtr = (char*) malloc (size);
  if (hostPtr == NULL)
  {
    fprintf(stderr, "Error: malloc() failed to allocate the requested %d bytes!\n",size );
    assert (hostPtr != NULL);
  }
  return hostPtr; 
}

// memory copy from src to dest, return the pointer to dest. NULL pointer if anything is wrong 
void * xomp_memcpyHostToDevice (void *dest, const void * src, size_t n)
{
  assert (dest != NULL);
  assert (src != NULL);
  if (xomp_verbose)
    printf("xomp_memcpyHostToDevice(): dest=%p src =%p size=%d\n",dest, src, n);
  cudaError_t rt = cudaMemcpy (dest, src, n, cudaMemcpyHostToDevice);    
  if (rt == cudaSuccess)
    return dest;
  else
  {
    fprintf(stderr, "Error: cudaMemcpy() failed to copy memory from Host %p to Device %p, for %d bytes!\n",src, dest, n);
    assert (false);
    return NULL; // it is a bad idea to silently return a NULL pointer
  }
}

void * xomp_memcpyDeviceToHost (void *dest, const void * src, size_t n)
{
  assert (dest != NULL);
  assert (src != NULL);
  if (xomp_verbose)
    printf("xomp_memcpyDeviceToHost(): dest=%p src =%p size=%d\n",dest, src, n);
  cudaError_t rt = cudaMemcpy (dest, src, n, cudaMemcpyDeviceToHost);    
  if (rt == cudaSuccess)
    return dest;
  else
  {
    fprintf(stderr, "Error: cudaMemcpy() failed to copy memory from Device %p to Host %p, for %d bytes!\n",src, dest, n);
    fprintf(stderr, "Error message is =%s\n",cudaGetErrorString(rt));
    assert (false);
    return NULL; // it is a bad idea to silently return a NULL pointer
  }
}

// copy a dynamically allocated host source array to a linear dest address on a GPU device. 
// The dimension information of the source array is given by: int dimensions[dimension_size], with known element size. 
// bytes_copied reports the total bytes copied by this function.  
// Liao 4/25/2012
void * xomp_memcpyDynamicHostToDevice (void *dest, const void * src, int * dimensions, size_t dimension_size, size_t element_size, size_t *bytes_copied)
{
  assert (dest != NULL);
  assert (src != NULL);
  if  (dimension_size == 1)  // down to the final, inner-most dimension 
  {
    // this is a firm, bottom count of bytes copied
    *bytes_copied = element_size* dimensions[0]; // number of elements * element_size
    xomp_memcpyHostToDevice (dest, src, *bytes_copied);
  }
  else 
  { 
    assert (dimension_size>=2);
    // 2-D or more: arrays of sub-arrays, copy each sub-array separately
    void ** array2 = (void**) src ; // re-interpret src to be array of arrays
    size_t sub_array_count = dimensions[0];  // top dimension, how many sub-arrays

    // prepare dimension information for sub-arrays
    dimensions ++;// go to the next dimension
    dimension_size --; // sub-array has one less dimension
    char* new_dest = (char*) dest; // byte addressable for dest
    size_t total_subarray_bytes_copied =0; 
    int i; 
    for (i = 0; i< sub_array_count; i++) // for each sub-arrays
    {
      size_t subarray_bytes_copied = 0; 
      void* sub_array_src = (void*) (array2[i]);  // get start address for each sub-array
      assert (sub_array_src != NULL);
      // recursively call to copy each sub-array
      xomp_memcpyDynamicHostToDevice (new_dest, (void*)sub_array_src, dimensions, dimension_size, 
          element_size, &subarray_bytes_copied);

      total_subarray_bytes_copied += subarray_bytes_copied; // recursively accumulate the bytes copied, instead of calculating directly.
      new_dest += subarray_bytes_copied ; // update the dest offset here
    }
    *bytes_copied = total_subarray_bytes_copied; 
  }
  return dest; 
}


// copy linear src memory to dynamically allocated destination, with dimension information given by
// int dimensions[dimension_size]
// the source memory has total n continuous memory, with known size for each element
// the total bytes copied by this function is reported by bytes_copied
// test code: mallocArray-xomp.cu
void * xomp_memcpyDynamicDeviceToHost (void *dest, int * dimensions, size_t dimension_size, const void * src, size_t element_size, size_t *bytes_copied)
{
  // int dimensions[1] = {10}; dimension_size =1; 
  // a[10]: 
  if  (dimension_size == 1)  // down to the final dimension 
  {
    // this is a firm, bottom count of bytes copied
    *bytes_copied = element_size* dimensions[0]; // number of elements * element_size
    xomp_memcpyDeviceToHost (dest, src, *bytes_copied);
  }
  else 
  { 
    int i; 
    assert (dimension_size>=2);
    // 2-D or more: arrays of sub-arrays, copy each sub-array separately
    void ** array2 = (void**) dest; // re-interpret dest to be array of arrays
    size_t sub_array_count = dimensions[0];  // current dimension, how many sub-arrays

    // prepare dimension information for sub-arrays
    dimensions ++;// go to the next dimension
    dimension_size --;
    char* new_src = (char*) src; // byte addressable for src
    size_t total_subarray_bytes_copied =0; 
    for (i = 0; i< sub_array_count; i++) // for each sub-arrays
    {
      size_t subarray_bytes_copied =0; 
      void* sub_array_dest = (void*) (array2[i]);  // get start address for each sub-array

      xomp_memcpyDynamicDeviceToHost ((void*)sub_array_dest,  dimensions, dimension_size, 
          new_src, element_size, &subarray_bytes_copied);

      total_subarray_bytes_copied += subarray_bytes_copied; // recursively accumulate the bytes copied, instead of calculating directly.
      new_src += subarray_bytes_copied ; // update the source offset here
    }
    *bytes_copied = total_subarray_bytes_copied; 
  }
  return dest; 
}

void * xomp_memcpyDeviceToDevice (void *dest, const void * src, size_t n)
{
    cudaError_t rt = cudaMemcpy (dest, src, n, cudaMemcpyDeviceToDevice);    
  if (rt == cudaSuccess)
    return dest;
  else
  {
    fprintf(stderr, "Error: cudaMemcpy() failed to copy from Device %p to Device %p for requested %d bytes!\n", src, dest, n );
    assert (false);
    return NULL; // it is a bad idea to silently return a NULL pointer
  }
}

void * xomp_memcpyHostToHost (void *dest, const void * src, size_t n) // same as memcpy??
{
    cudaError_t rt = cudaMemcpy (dest, src, n, cudaMemcpyHostToHost);    
  if (rt == cudaSuccess)
    return dest;
  else
  {
    fprintf(stderr, "Error: cudaMemcpy() failed to copy from Host %p to Host %p for requested %d bytes!\n", src, dest, n );
    assert (false);
    return NULL; // it is a bad idea to silently return a NULL pointer
  }

}

//------------------------------------------------------
// free the device memory pointed by a pointer, return false in case of failure, otherwise return true
bool xomp_freeDevice(void* devPtr)
{
  cudaError_t rt = cudaFree (devPtr);
  if (rt == cudaSuccess)
    return true;
  else
    return false;
}

// free the host memory pointed by a pointer, return false in case of failure, otherwise return true
bool xomp_freeHost(void* hostPtr)
{
  cudaError_t rt = cudaFreeHost(hostPtr);
  if (rt == cudaSuccess)
    return true;
  else
    return false;
}

//------------------------------------------------------
// data set size checking functions
#if 0
  // make sure the length of the array can be mapped to the cuda threads
  assert (SIZE <= max_blocks_per_grid_x* max_threads_per_block);
  // make sure the data will fit into the device memory (shared memory)
  printf("matrix-vector multiplication with size=%d\n", SIZE);
  // one matrix and two vectors
  int mem_required = SIZE*SIZE*sizeof(float) + SIZE* sizeof(float) *2;
  if (global_memory_size > 0) //sometimes the number is too large and it overflows to be a negative integer
    assert (mem_required  <= global_memory_size);
#endif

//------------------------------------------------------
#if 0
double xomp_time_stamp()
{
  struct timeval t;
  double time;
  gettimeofday(&t, NULL);
  time = t.tv_sec + 1.0e-6*t.tv_usec;
  return time;
}
#endif
//------------------------------------------------------
// Host side helper functions
//--- a helper function to allocate 2-D arrays
/* Allocate a multi-dimensional array
 *
 * Input parameters:
 *  int *dimensions:  an integer array storing the size of each dimension
 *  size_t dimension_num: the number of dimensions
 *  size_t esize: the size of an array element
 *
 * return:
 *  the pointer to the allocated array
 * */
void * xomp_mallocArray(int * dimensions, size_t dimension_num, size_t esize)
{
  int i;
  void * array = NULL;

//  if (xomp_verbose)
//    printf("xomp_xomp_mallocArray(): dimensions=%p dimension =%d element size=%d\n",dimensions, dimension_num, esize);
  //  Handle 1-D array: do element-wise malloc
  if ( dimension_num == 1)
  {
    array = (void *) malloc(dimensions[dimension_num - 1]*esize);
    if(array == NULL)
    {
      fprintf(stderr, "out of memory\n");
      abort();
    }
  }
  else // two and more dimensions to be allocated: reduce it to be a 2-step allocation
  {
    // 1st step: allocate the first dimension 
    //           by treating it as allocating a 1-D array of arrays (pointer)
    void ** array2 = NULL;
    array2 = (void **) xomp_mallocArray(dimensions, 1 ,sizeof (void *));
    size_t prev_dim_size = dimensions[0];// number of elements of the first dimension

    // 2nd step: allocate the remaining N -1 dimension arrays, each is an element of the first array 
    // peel off the 1st(previous) dimension, focus on the rest dimensions
     dimensions ++;
    // each of element is an array has a smaller dimension number
     dimension_num --;
    for(i = 0; i < prev_dim_size ; i++)
    {
      array2[i] = xomp_mallocArray (dimensions, dimension_num, esize);
    }
    // return the pointer to the first dimension
    array = (void *) array2;
  }
  return array;
}


/* Free a pointer to a multi-dimensional array
 * int * dimensions: store the sizes of each dimension
 * size_t dimension_num: the number of dimensions
 *
 * */
void xomp_freeArrayPointer (void* array, int * dimensions, size_t dimension_num)
{

  int i;
  // 1-D case, call free() directly
  if (dimension_num == 1)
  {
    free (array);
  }
  else
  { // 2-D or more, iterate through higher dimension and try to free inner arrays
    int prev_dim_size = dimensions [0];
    // step into one dimension
    dimensions ++;
    dimension_num --;
    for (i =0; i< prev_dim_size ; i++)
    {
      xomp_freeArrayPointer (((void **)array)[i], dimensions, dimension_num);
    }
  }
}

#if 0
/*  reduction minus is handled the same way as reduction plus since we just replace the reduction variable with its local copy for each thread
    The associated statement is intact except for the variable replacement : e.g. a-=5 becomes local_a -= 5;
    in the end of each thread accumulates thread local negative values.
    At the block level, we just simply add them all to be the block level negative values
*/
/* we have to encode the type into function name since C function signature does not include parameter list! */
#define XOMP_INNER_BLOCK_REDUCTION_DEF(dtype) \
__device__ void xomp_inner_block_reduction_##dtype(dtype local_value, dtype * grid_level_results, int reduction_op) \
{ \
  /* __shared__ float* sdata[gridDim.x]; not compilable */ \
  /* block size of data, size is specified by the kernel launch parameter (3rd one) */ \
  /* shared data has to have different names for different types. Cannot reuse name across types. */ \
  extern __shared__ dtype sdata_##dtype[];  \
  sdata_##dtype[threadIdx.x] = local_value;  \
  __syncthreads(); \
  /* blockDim.x is the block size */ \
  int isEvenSize = (blockDim.x % 2 ==0); \
  /* contiguous range pattern: half folding and add */ \
  for(int offset = blockDim.x / 2; \
      offset > 0;    /* folding and add */ \
      offset >>= 1) /* offset shrinks half each time */ \
  { \
    if(threadIdx.x < offset)  \
    { \
      /* add a partial sum upstream to our own */ \
      switch (reduction_op){ \
        case XOMP_REDUCTION_PLUS: \
        case XOMP_REDUCTION_MINUS: \
            sdata_##dtype[threadIdx.x] += sdata_##dtype[threadIdx.x + offset]; \
            break; \
         /*  TODO add support for more operations*/ \
         default:  \
            { \
              /* TODO: add assertion or set cudaError with an error code */ \
              /* cannot call a host function */ \
              /* fprintf (stderr, "Error. xomp_inner_block_reduction() unhandled reduction operation:%d\n",reduction_op); */ \
              /* assert (false); */ \
             } \
      } /* end switch */ \
    } \
    /* remember to handle the left element */ \
    if ((threadIdx.x == 0) && !isEvenSize) \
    { \
      switch (reduction_op){ \
        case XOMP_REDUCTION_PLUS: \
        case XOMP_REDUCTION_MINUS: \
          sdata_##dtype[0]+= sdata_##dtype[2*offset];  \
          break; \
        /* TODO add more operation support */  \
        default: \
          {  \
            /* TODO: add assertion or set cudaError with an error code */  \
            /* cannot call a host function */ \
            /* fprintf (stderr, "Error. xomp_inner_block_reduction() unhandled reduction operation:%d\n",reduction_op); */ \
            /* assert (false); */ \
          } \
      } /* end switch */ \
    } \
    isEvenSize = ( offset % 2 ==0); /* prepare next round*/ \
    /* MUST wait until all threads in the block have updated their partial sums */ \
    __syncthreads(); /* sync after each folding */ \
  } \
  /* thread 0 writes the final result to the partial sum of this thread block */ \
  if(threadIdx.x == 0) \
  { \
    grid_level_results[blockIdx.x] = sdata_##dtype[0]; \
  } \
}

XOMP_INNER_BLOCK_REDUCTION_DEF(int)
XOMP_INNER_BLOCK_REDUCTION_DEF(float)
XOMP_INNER_BLOCK_REDUCTION_DEF(double)

#undef XOMP_INNER_BLOCK_REDUCTION_DEF 

#endif

// TODO: handle more different reduction operations
// TODO : add assertion support
#define XOMP_BEYOND_BLOCK_REDUCTION_DEF(dtype) \
  dtype xomp_beyond_block_reduction_##dtype(dtype * per_block_results, int numBlocks, int reduction_op) \
{ \
  dtype result ; \
  dtype* per_block_results_cpu = (dtype *)xomp_hostMalloc (numBlocks*sizeof(dtype)); \
  xomp_memcpyDeviceToHost (per_block_results_cpu, per_block_results, sizeof(dtype)* numBlocks); \
  int r_i; \
  for (r_i =1; r_i < numBlocks; r_i++) \
  { \
     switch (reduction_op){ \
        case XOMP_REDUCTION_PLUS: \
        case XOMP_REDUCTION_MINUS: \
          per_block_results_cpu[0]+= per_block_results_cpu[r_i]; \
          break; \
        default: \
          { \
          } \
      } \
  } \
  result = per_block_results_cpu[0]; \
  xomp_freeHost(per_block_results_cpu); \
  return result; \
} 

//TODO define more types of CPU level reduction support
XOMP_BEYOND_BLOCK_REDUCTION_DEF(int)
XOMP_BEYOND_BLOCK_REDUCTION_DEF(float)
XOMP_BEYOND_BLOCK_REDUCTION_DEF(double)

#undef XOMP_BEYOND_BLOCK_REDUCTION_DEF 

/* some of the ompacc runtime API */
int omp_get_num_devices() {
  int deviceCount = 0;
  cudaGetDeviceCount(&deviceCount);
  return deviceCount;
}

//! A helper function to copy a mapped variable from src to desc
void copy_mapped_variable (struct XOMP_mapped_variable* desc, struct XOMP_mapped_variable* src)
{
  assert (src != NULL);
  assert (desc != NULL);

  desc->address = src->address; 
  desc->size= src->size; 
  desc->dev_address = src ->dev_address; 
   // we do not want to inherit the copy directions or map-type of parent DDE's variable
   // OpenMP 4.0 has the reuse enclosing data and discard map-type rule.
  //desc->copyFrom= src ->copyFrom; 
}

// create a new DDE-data node and 
// append it to the end of the tracking list, and 
// copy all variables from its parent node to be into the set of inherited variable set.
void xomp_deviceDataEnvironmentEnter()
{
  // create a new DDE node and initialize it
  struct DDE_data * data = (struct DDE_data *) malloc (sizeof (struct DDE_data));
  assert (data!=NULL);
  data->new_variable_count = 0;
  data->inherited_variable_count = 0;
  data->parent = NULL;
  data->child= NULL;

  // For simplicity, we pre-allocate the storage for the list of variables
  // TODO: improve the efficiency
  data->new_variables = (struct XOMP_mapped_variable*) malloc (XOMP_MAX_MAPPED_VARS * sizeof (struct XOMP_mapped_variable));
  data->inherited_variables = (struct XOMP_mapped_variable*) malloc (XOMP_MAX_MAPPED_VARS * sizeof (struct XOMP_mapped_variable));

  // Append the data to the list
  // Case 1: empty list, add as the first node, nothing else to do
  if (DDE_tail == NULL)
  {
    assert (DDE_head == NULL );
    DDE_head = data;
    DDE_tail = data;
    return; 
  }

  // Case 2: non-empty list
  // create double links
  data->parent = DDE_tail; 
  DDE_tail->child = data;
  // shift the tail
  DDE_tail = data;

  // copy all variables from its parent node into the inherited variable set. 
  // Both new and inherited variables of the parent node become inherited for the current node
  data->inherited_variable_count = data->parent->new_variable_count + data->parent->inherited_variable_count;
  data->inherited_variables = (struct XOMP_mapped_variable*) malloc (data->inherited_variable_count * sizeof (struct XOMP_mapped_variable));
  assert (data->inherited_variables != NULL);

  int i;
  int offset = 0;
  for (i = 0; i < data->parent->new_variable_count; i++)
  {
    struct XOMP_mapped_variable* dest_element  = data->inherited_variables + offset;
    struct DDE_data* p = data->parent;
    struct XOMP_mapped_variable* src_element  =  p->new_variables + i;

    copy_mapped_variable(dest_element, src_element);
    offset ++;
  }

  for (i = 0; i < data->parent->inherited_variable_count; i++)
  {
    //copy_mapped_variable(&((data->inherited_variables)[offset]), &( (data->parent->inherited_variables)[i]));
    copy_mapped_variable( (struct XOMP_mapped_variable*) (data->inherited_variables + offset), (struct XOMP_mapped_variable*) (data->parent->inherited_variables + i));
    offset ++;
  }
  assert (offset == data->inherited_variable_count);

}

// Check if an original  variable is already mapped in enclosing data environment, return its device variable's address if yes.
// return NULL if not
void* xomp_deviceDataEnvironmentGetInheritedVariable (void* orig_var, int size)
{
  void * dev_address = NULL; 
  assert (orig_var != NULL);
  int i; 
  // At this point, DDE list should not be empty
  // At least a call to XOMP_Device_Data_Environment_Enter() should have finished before
  assert ( DDE_tail != NULL );
  for (i = 0; i < DDE_tail->inherited_variable_count; i++)
  {
    struct XOMP_mapped_variable* cur_var = DDE_tail->inherited_variables + i; 
    if (cur_var->address == orig_var && cur_var->size == size)
    {
      dev_address = cur_var-> dev_address;
      break;
    }
  } 
  return dev_address; 
}

//! Add a newly mapped variable into the current DDE's new variable list
void xomp_deviceDataEnvironmentAddVariable (void* var_addr, int var_size, void * dev_addr, bool copyTo, bool copyFrom)
{
  // TODO: sanity check to avoid add duplicated variable or inheritable variable
  assert ( DDE_tail != NULL );
  struct XOMP_mapped_variable* mapped_var = DDE_tail->new_variables + DDE_tail->new_variable_count ;
  mapped_var-> address = var_addr; 
  mapped_var-> size = var_size; 
  mapped_var-> dev_address = dev_addr; 
  mapped_var-> copyTo= copyTo; 
  mapped_var-> copyFrom= copyFrom; 
  // now move up the offset
  DDE_tail->new_variable_count ++;
}

// All-in-one function to prepare device variable
void* xomp_deviceDataEnvironmentPrepareVariable(void* original_variable_address, int vsize, bool copy_into, bool copy_back)
{
  void* dev_var_address = NULL; 
  dev_var_address = xomp_deviceDataEnvironmentGetInheritedVariable (original_variable_address, vsize);
  if (dev_var_address == NULL)
  {
    dev_var_address = xomp_deviceMalloc(vsize);
    xomp_deviceDataEnvironmentAddVariable (original_variable_address, vsize, dev_var_address, copy_into, copy_back);

    // The spec says : reuse enclosing data and discard map-type rule.
    // So map-type only matters when no-reuse happens
    if (copy_into)
      xomp_memcpyHostToDevice(dev_var_address, original_variable_address, vsize);
  }
  assert (dev_var_address != NULL);
  return dev_var_address;
}

// Exit current DDE: copy back values if specified, deallocate memory, delete the DDE-data node from the end of the tracking list
void xomp_deviceDataEnvironmentExit()
{
  assert ( DDE_tail != NULL );

  // Deallocate mapped device variables which are allocated by this current DDE
  // Optionally copy the value back to host if specified.
  int i; 
  for (i = 0; i < DDE_tail->new_variable_count; i++)
  {
    struct XOMP_mapped_variable* mapped_var = DDE_tail->new_variables + i;
    void * dev_address = mapped_var->dev_address;
    if (mapped_var->copyFrom)
    {
       xomp_memcpyDeviceToHost(((void *)mapped_var->address),((const void *)mapped_var->dev_address), mapped_var->size);
    }
    // free after copy back!!
    xomp_freeDevice (dev_address); //TODO Will this work without type info? Looks so!
  }

  // Deallocate pre-allocated variable lists
  free (DDE_tail->new_variables);
  free (DDE_tail->inherited_variables);
  
  // Delete the node from the tail
  struct DDE_data * parent = DDE_tail->parent; 
  if (parent != NULL)
  {
    assert (DDE_tail == parent->child); 
    DDE_tail = parent; 
    free (parent->child);
    parent->child = NULL;
  }
  else // last node in the list
  {
    free (DDE_tail);
    DDE_head = NULL;
    DDE_tail = NULL;
  }  
}


