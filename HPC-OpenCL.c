
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include<sys/time.h>
#include<sys/resource.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <unistd.h>
#else
#include <CL/cl.h>
#endif

//#include "err_code.h"

#define NSPEEDS         9
#define FINALSTATEFILE  "final_state.dat"
#define AVVELSFILE      "av_vels.dat"

/* struct to hold the parameter values */
typedef struct {
  int    nx;            /* no. of cells in x-direction */
  int    ny;            /* no. of cells in y-direction */
  int    maxIters;      /* no. of iterations */
  int    reynolds_dim;  /* dimension for Reynolds number */
  float density;       /* density per link */
  float accel;         /* density redistribution */
  float omega;         /* relaxation parameter */
} t_param;

enum boolean { FALSE, TRUE };

/*
** function prototypes
*/

/* load params, allocate memory, load obstacles & initialise fluid particle densities */
int initialise(const char* paramfile, const char* obstaclefile,
        t_param* h_params, float** h_cells_ptr, float** h_tmp_cells_ptr, 
         int** h_obstacles_ptr, double** h_av_vels_ptr);

/*Host program functions*/
void init_context_bcp3(cl_context* context, cl_device_id* device);
char* getKernelSource(char* filename);

/*main functions*/
int write_values(const t_param params, float* h_cells, int* h_obstacles, double* h_av_vels);

/* finalise, including freeing up allocated memory */
int finalise(const t_param* h_params, float** h_cells_ptr, float** h_tmp_cells_ptr,
       int** h_obstacles_ptr, double** h_av_vels_ptr);

/*Utility functions*/
void usage(const char* exe);
void die(const char* message, const int line, const char *file);



int main(int argc, char*argv[]) 
{
  char*    paramfile = NULL;      /* name of the input parameter file */
  char*    obstaclefile = NULL;   /* name of a the input obstacle file */
  t_param  h_params;              /* struct to hold parameter values */
  float* h_cells     = NULL;      /* grid containing fluid densities */
  float* h_tmp_cells = NULL;      /* scratch space */
  double* h_partial_u = NULL;      /* array to hold partial sums for reduction */
  double* h_partial_cells = NULL;  
  int*     h_obstacles = NULL;    /* grid indicating which cells are blocked */
  double*  h_av_vels   = NULL;    /* a record of the av. velocity computed for each timestep */
  int size;                       /* size of grid to work over (should be multiple of 32? power of 2?) */
  size_t n_work_groups,work_group_size;
  double tmp;

  char* kernelsource;             /*Kernel source*/

  struct timeval timstr;        /* structure to hold elapsed time */
  struct rusage ru;             /* structure to hold CPU time--system and user */
  double tic,toc;               /* floating point numbers to calculate elapsed wallclock time */
  double usrtim;                /* floating point number to record elapsed user CPU time */
  double systim;                /* floating point number to record elapsed system CPU time */

  cl_mem d_params, d_cells, d_tmp_cells, d_obstacles;
  cl_mem d_partial_u, d_partial_cells;
  cl_int err;
  cl_context context;
  cl_device_id device;
  cl_command_queue commands1, commands2;
  cl_program program;
  cl_kernel kernel_acc, kernel_prop, kernel_coll, kernel_av;

//-----------------------------------------------------------------
// Standard LBM set up
//-----------------------------------------------------------------

  /* parse the command line */
  if(argc != 3) {
    usage(argv[0]);
  }
  else{
    paramfile = argv[1];
    obstaclefile = argv[2];
  }

  /* initialise our data structures and load values from file */
  initialise(paramfile, obstaclefile, &h_params, &h_cells, &h_tmp_cells, &h_obstacles, &h_av_vels);

  size = fmax(h_params.nx,h_params.ny);
  while ((size % 32) != 0) size++ ;

//----------------------------------------------------------------
// Start timer
//----------------------------------------------------------------

  gettimeofday(&timstr,NULL);
  tic=timstr.tv_sec+(timstr.tv_usec/1000000.0);

//-----------------------------------------------------------------
// Set up host program
//-----------------------------------------------------------------

  //find devices and create context
  init_context_bcp3(&context,&device);
  //create a command queue
  commands1 = clCreateCommandQueue(context,device,0,&err);
  //checkError(err,"Creating Command Queue 1");
  commands2 = clCreateCommandQueue(context,device,0, &err);
  //checkError(err,"Creating Command Queue 2");

//----------------------------------------------------------------
// Set up device buffers, copy to global memory
//----------------------------------------------------------------

  d_cells = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
              sizeof(float)*NSPEEDS*h_params.nx*h_params.ny, h_cells, &err);
  //checkError(err,"Creating buffer d_cells");
  d_tmp_cells = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                  sizeof(float)*NSPEEDS*h_params.nx*h_params.ny, h_tmp_cells, &err);
  //checkError(err,"Creating buffer d_tmp_cells");
  d_obstacles = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                  sizeof(int)*h_params.nx*h_params.ny, h_obstacles, &err);
  //checkError(err,"Creating buffer d_obstacles");

//----------------------------------------------------------------
// Create program and kernels
//----------------------------------------------------------------

  kernelsource = getKernelSource("d2q9-bgk.cl");
  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **) &kernelsource, NULL, &err);
  //checkError(err, "Creating program");
  free(kernelsource);
  // Build the program  
  err = clBuildProgram(program, 0, NULL,
    "-cl-mad-enable -cl-single-precision-constant -cl-fast-relaxed-math", NULL, NULL);
  /*if (err != CL_SUCCESS)
  {
      size_t len;
      char buffer[2048];

      printf("Error: Failed to build program executable!\n%s\n", err_code(err));
      clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
      printf("%s\n", buffer);
      return EXIT_FAILURE;
  }*/
  // Create the compute kernel from the program 
  kernel_acc = clCreateKernel(program, "accelerate_flow", &err);
  kernel_prop = clCreateKernel(program, "propagate", &err);
  kernel_coll = clCreateKernel(program, "collision", &err);
  kernel_av = clCreateKernel(program, "av_velocity", &err);
  //checkError(err, "Creating kernel");

//---------------------------------------------------------------
// Find work group sizes and set up reduction
//---------------------------------------------------------------

  // Find kernel work-group size
  err = clGetKernelWorkGroupInfo (kernel_av, device, CL_KERNEL_WORK_GROUP_SIZE,
          sizeof(size_t), &work_group_size, NULL);
  //checkError(err, "Getting kernel work group info");
  // Now that we know the size of the work-groups, we can set the number of
  // work-groups, the actual number of steps, and the step size
  while ((size % work_group_size) != 0) work_group_size-- ;
  work_group_size = fmin(32,work_group_size);
  n_work_groups = size/work_group_size;

  d_partial_u = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                sizeof(double)*size, NULL, &err);
  //checkError(err, "Creating buffer for d_partial_u");
  d_partial_cells = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                    sizeof(double)*size, NULL, &err);
  //checkError(err, "Creating buffer for d_partial_cells");

  h_partial_u = (double*)clEnqueueMapBuffer(commands2, d_partial_u, CL_FALSE,
    CL_MAP_READ, 0, sizeof(double)*size, 0, NULL, NULL, &err);
  //checkError(err, "Mapping to h_partial_u");
  h_partial_cells = (double*)clEnqueueMapBuffer(commands2, d_partial_cells, CL_FALSE,
    CL_MAP_READ, 0, sizeof(double)*size, 0, NULL, NULL, &err);
  //checkError(err, "Mapping to h_partial_cells");

//---------------------------------------------------------------
// Set arguments and run kernels
//---------------------------------------------------------------

  //set constant arguments
  err = clSetKernelArg(kernel_acc,0,sizeof(t_param),&h_params);
  err = clSetKernelArg(kernel_acc,2,sizeof(cl_mem),&d_obstacles);
  err = clSetKernelArg(kernel_prop,0,sizeof(t_param),&h_params);
  err = clSetKernelArg(kernel_coll,0,sizeof(t_param),&h_params);
  err = clSetKernelArg(kernel_coll,3,sizeof(cl_mem),&d_obstacles);
  err = clSetKernelArg(kernel_av,0,sizeof(t_param),&h_params);
  err = clSetKernelArg(kernel_av,2,sizeof(cl_mem),&d_obstacles);
  //checkError(err,"Setting Constant Kernel Args");

  // Run maxIters times
  for(int ii = 0; ii < h_params.maxIters; ii++) {

    //set kernel arguments
    err = clSetKernelArg(kernel_acc,1,sizeof(cl_mem),&d_cells);
    err = clSetKernelArg(kernel_prop,1,sizeof(cl_mem),&d_cells);
    err = clSetKernelArg(kernel_prop,2,sizeof(cl_mem),&d_tmp_cells);
    err = clSetKernelArg(kernel_coll,1,sizeof(cl_mem),&d_cells);
    err = clSetKernelArg(kernel_coll,2,sizeof(cl_mem),&d_tmp_cells);
    err = clSetKernelArg(kernel_av,1,sizeof(cl_mem),&d_cells);
    err = clSetKernelArg(kernel_av,3,sizeof(double)*size,
          NULL);
    err = clSetKernelArg(kernel_av,4,sizeof(double)*size,
          NULL);
    err = clSetKernelArg(kernel_av,5,sizeof(cl_mem),&d_partial_u);
    err = clSetKernelArg(kernel_av,6,sizeof(cl_mem),&d_partial_cells);
    //checkError(err,"Setting Kernel Args");

    const size_t global[2] = {size, size};
    size_t local[2] = {work_group_size, work_group_size};
    //run accelerate_flow
    err = clEnqueueNDRangeKernel(commands1,kernel_acc,1,NULL,&global[1],NULL,0,
            NULL,NULL);
    //checkError(err,"Enqueuing Kernel");

    /*err = clFinish(commands1);
    checkError(err, "Waiting for kernel to finish");*/

    //run propagate
    err = clEnqueueNDRangeKernel(commands1,kernel_prop,2,NULL,global,local,0,
            NULL,NULL);
    //checkError(err,"Enqueuing Kernel");

    /*err = clFinish(commands1);
    checkError(err, "Waiting for kernel to finish");*/

    //run collision
    err = clEnqueueNDRangeKernel(commands1,kernel_coll,2,NULL,global,local,0,
            NULL,NULL);
    //checkError(err,"Enqueuing Kernel");

    /*err = clFinish(commands1);
    checkError(err, "Waiting for kernel to finish");*/

    //run av_vels
    local[0] = size;
    local[1] = 1;
    /*const size_t local[2] = {size, 1};*/
    err = clEnqueueNDRangeKernel(commands1,kernel_av,2,NULL,global,local,
            0,NULL,NULL);
    //checkError(err,"Enqueueing av_vels Kernel");

    err = clFinish(commands1);
    //checkError(err, "Waiting for kernel to finish");

    //retrieve h_partial_u
    err = clEnqueueReadBuffer(commands2, d_partial_u, CL_FALSE, 0,
            sizeof(double)*size, h_partial_u, 0, NULL, NULL);
    //checkError(err, "Reading back d_partial_u");

    //retrieve h_partial_cells
    err = clEnqueueReadBuffer(commands2, d_partial_cells, CL_FALSE, 0,
            sizeof(double)*size, h_partial_cells, 0, NULL, NULL);
    //checkError(err, "Reading back d_partial_cells");

    err = clFinish(commands2);
    //checkError(err, "Waiting for asynchronous reads to finish");

    tmp = 0.0;
    h_av_vels[ii] = 0.0;
    for (int jj = 0; jj<(size); jj++) {
      h_av_vels[ii] += h_partial_u[jj];
      tmp += h_partial_cells[jj];
    }
    h_av_vels[ii] = h_av_vels[ii]/(double)tmp;
  }

  //retrieve h_cells
  err = clEnqueueReadBuffer(commands2, d_cells, CL_TRUE, 0,
            sizeof(float)*NSPEEDS*h_params.nx*h_params.ny, h_cells, 0, NULL, NULL);
  //checkError(err, "Reading back d_cells");

  write_values(h_params,h_cells,h_obstacles,h_av_vels);

//---------------------------------------------------------------
// End Timers
//---------------------------------------------------------------

  gettimeofday(&timstr,NULL);
  toc=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  getrusage(RUSAGE_SELF, &ru);
  timstr=ru.ru_utime;        
  usrtim=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  timstr=ru.ru_stime;        
  systim=timstr.tv_sec+(timstr.tv_usec/1000000.0);

  /* write final values and free memory */
  printf("==done==\n");
  printf("Elapsed time:\t\t\t%.6lf (s)\n", toc-tic);
  printf("Elapsed user CPU time:\t\t%.6lf (s)\n", usrtim);
  printf("Elapsed system CPU time:\t%.6lf (s)\n", systim);

//----------------------------------------------------------------
// Clean up
//----------------------------------------------------------------

  finalise(&h_params, &h_cells, &h_tmp_cells, &h_obstacles, &h_av_vels);
  clReleaseMemObject(d_cells);
  clReleaseMemObject(d_tmp_cells);
  clReleaseMemObject(d_obstacles);  
  clReleaseProgram(program);
  clReleaseKernel(kernel_acc);
  clReleaseKernel(kernel_prop);
  clReleaseKernel(kernel_coll);
  clReleaseKernel(kernel_av);
  clReleaseCommandQueue(commands1);
  clReleaseCommandQueue(commands2);
  clReleaseContext(context);

  return EXIT_SUCCESS;
}


int initialise(const char* paramfile, const char* obstaclefile,
         t_param* params, float** cells_ptr, float** tmp_cells_ptr, 
         int** obstacles_ptr, double** av_vels_ptr)
{
  char   message[1024];  /* message buffer */
  FILE   *fp;            /* file pointer */
  int    ii,jj;       /* generic counters */
  int    xx,yy;          /* generic array indices */
  int    blocked;        /* indicates whether a cell is blocked by an obstacle */ 
  int    retval;         /* to hold return value for checking */
  double w0,w1,w2;       /* weighting factors */
  int pos, stride;

  /* open the parameter file */
  fp = fopen(paramfile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input parameter file: %s", paramfile);
    die(message,__LINE__,__FILE__);
  }

  /* read in the parameter values */
  retval = fscanf(fp,"%d\n",&(params->nx));
  if(retval != 1) die ("could not read param file: nx",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->ny));
  if(retval != 1) die ("could not read param file: ny",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->maxIters));
  if(retval != 1) die ("could not read param file: maxIters",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->reynolds_dim));
  if(retval != 1) die ("could not read param file: reynolds_dim",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->density));
  if(retval != 1) die ("could not read param file: density",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->accel));
  if(retval != 1) die ("could not read param file: accel",__LINE__,__FILE__);
  retval = fscanf(fp,"%f\n",&(params->omega));
  if(retval != 1) die ("could not read param file: omega",__LINE__,__FILE__);

  /* and close up the file */
  fclose(fp);

  /* 
  ** Allocate memory.
  **
  ** Remember C is pass-by-value, so we need to
  ** pass pointers into the initialise function.
  **
  ** NB we are allocating a 1D array, so that the
  ** memory will be contiguous.  We still want to
  ** index this memory as if it were a (row major
  ** ordered) 2D array, however.  We will perform
  ** some arithmetic using the row and column
  ** coordinates, inside the square brackets, when
  ** we want to access elements of this array.
  **
  ** Note also that we are using a structure to
  ** hold an array of 'speeds'.  We will allocate
  ** a 1D array of these structs.
  */

  /* main grid */
  *cells_ptr = (float*)malloc(sizeof(float)*(params->ny*params->nx)*NSPEEDS);
  if (*cells_ptr == NULL) 
    die("cannot allocate memory for cells",__LINE__,__FILE__);

  /* 'helper' grid, used as scratch space */
  *tmp_cells_ptr = (float*)malloc(sizeof(float)*(params->ny*params->nx)*NSPEEDS);
  if (*tmp_cells_ptr == NULL) 
    die("cannot allocate memory for tmp_cells",__LINE__,__FILE__);
  
  /* the map of obstacles */
  *obstacles_ptr = (int*)malloc(sizeof(int)*(params->ny*params->nx));
  if (*obstacles_ptr == NULL) 
    die("cannot allocate column memory for obstacles",__LINE__,__FILE__);

  /* initialise densities */
  w0 = params->density * 4.0/9.0;
  w1 = params->density      /9.0;
  w2 = params->density      /36.0;

  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      pos = ii*params->nx + jj;
      stride = params->nx * params->ny;
      /* centre */
      (*cells_ptr)[pos] = w0;
      /* axis directions */
      (*cells_ptr)[stride + pos] = w1;
      (*cells_ptr)[2*stride + pos] = w1;
      (*cells_ptr)[3*stride + pos] = w1;
      (*cells_ptr)[4*stride + pos] = w1;
      /* diagonals */
      (*cells_ptr)[5*stride + pos] = w2;
      (*cells_ptr)[6*stride + pos] = w2;
      (*cells_ptr)[7*stride + pos] = w2;
      (*cells_ptr)[8*stride + pos] = w2;
    }
  }

  /* first set all cells in obstacle array to zero */ 
  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      (*obstacles_ptr)[ii*params->nx + jj] = 0;
    }
  }

  /* open the obstacle data file */
  fp = fopen(obstaclefile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input obstacles file: %s", obstaclefile);
    die(message,__LINE__,__FILE__);
  }

  /* read-in the blocked cells list */
  while( (retval = fscanf(fp,"%d %d %d\n", &xx, &yy, &blocked)) != EOF) {
    /* some checks */
    if ( retval != 3)
      die("expected 3 values per line in obstacle file",__LINE__,__FILE__);
    if ( xx<0 || xx>params->nx-1 )
      die("obstacle x-coord out of range",__LINE__,__FILE__);
    if ( yy<0 || yy>params->ny-1 )
      die("obstacle y-coord out of range",__LINE__,__FILE__);
    if ( blocked != 1 ) 
      die("obstacle blocked value should be 1",__LINE__,__FILE__);
    /* assign to array */
    (*obstacles_ptr)[yy*params->nx + xx] = blocked;
  }
  
  /* and close the file */
  fclose(fp);

  /* 
  ** allocate space to hold a record of the avarage velocities computed 
  ** at each timestep
  */
  *av_vels_ptr = (double*)malloc(sizeof(double)*params->maxIters);

  return EXIT_SUCCESS;
}



int write_values(const t_param params, float* cells, int* obstacles, double* av_vels)
{
  FILE* fp;                     /* file pointer */
  int ii,jj,kk, pos, stride;    /* generic counters */
  const double c_sq = 1.0/3.0;  /* sq. of speed of sound */
  double local_density;         /* per grid cell sum of densities */
  double pressure;              /* fluid pressure in grid cell */
  double u_x;                   /* x-component of velocity in grid cell */
  double u_y;                   /* y-component of velocity in grid cell */
  double u;                     /* norm--root of summed squares--of u_x and u_y */

  fp = fopen(FINALSTATEFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }

  stride = params.nx * params.ny;

  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj);
      /* an occupied cell */
      if(obstacles[ii*params.nx + jj]) {
        u_x = u_y = u = 0.0;
        pressure = params.density * c_sq;
      }
      /* no obstacle */
      else {
        local_density = 0.0;
        for(kk=0;kk<NSPEEDS;kk++) {
          local_density += cells[pos+kk*stride];
        }
        /* compute x velocity component */
        u_x = (cells[pos+(1*stride)] + cells[pos+(5*stride)] + cells[pos+(8*stride)]
          - (cells[pos+(3*stride)] + cells[pos+(6*stride)] + cells[pos+(7*stride)]))
          / local_density;
        /* compute y velocity component */
        u_y = (cells[pos+(2*stride)] + cells[pos+(5*stride)] + cells[pos+(6*stride)]
          - (cells[pos+(4*stride)] + cells[pos+(7*stride)] + cells[pos+(8*stride)]))
          / local_density;
        /* compute norm of velocity */
        u = sqrt((u_x * u_x) + (u_y * u_y));
        /* compute pressure */
        pressure = local_density * c_sq;
      }
      /* write to file */
      fprintf(fp,"%d %d %.12E %.12E %.12E %.12E %d\n",jj,ii,u_x,u_y,u,pressure,obstacles[ii*params.nx + jj]);
    }
  }

  fclose(fp);

  fp = fopen(AVVELSFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }
  for (ii=0;ii<params.maxIters;ii++) {
    fprintf(fp,"%d:\t%.12E\n", ii, av_vels[ii]);
  }

  fclose(fp);

  return EXIT_SUCCESS;
}



//Find and use device assigned by BCP3 queue
void init_context_bcp3(cl_context* context, cl_device_id* device)
{
    //
    // Get the index into the device array allocated by the queue on BCP3
    //
    char* path = getenv("PBS_GPUFILE");
    if (path == NULL)
    {
        fprintf(stderr, "Error: PBS_GPUFILE environment variable not set by queue\n");
        exit(-1);
    }
    FILE* gpufile = fopen(path, "r");
    if (gpufile == NULL)
    {
        fprintf(stderr, "Error: PBS_GPUFILE not found\n");
        exit(-1);
    }

    size_t max_length = 100;
    char* line = (char *)malloc(sizeof(char)*max_length);

    size_t len = getline(&line, &max_length, gpufile);

    int device_index = (int) strtol(&line[len-2], NULL, 10);

    free(line);
    fclose(gpufile);

    cl_int err;

    // Query platforms
    cl_uint num_platforms;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (err != CL_SUCCESS) {fprintf(stderr, "Error counting platforms: %d\n", err); exit(-1);}

    cl_platform_id platforms[num_platforms];
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (err != CL_SUCCESS) {fprintf(stderr, "Error getting platforms: %d\n", err); exit(-1);}

    // Get devices for each platform - stop when found a GPU
    cl_uint max_devices = 8;
    cl_device_id devices[max_devices];
    cl_uint num_devices;

    int i;
    for (i = 0; i < num_platforms; i++)
    {
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, max_devices, devices, &num_devices);
        if (err == CL_SUCCESS && num_devices > 0)
            break;
    }
    if (num_devices == 0)
    {
        fprintf(stderr, "Error: no GPUs found");
        exit(-1);
    }

    // Assign the device id
    *device = devices[device_index];

    // Create the context
    const cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[i], 0};
    *context = clCreateContext(properties, 1, device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {fprintf(stderr, "Error creating context: %d\n", err); exit(-1);}
}



char * getKernelSource(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open kernel source file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    int len = ftell(file) + 1;
    rewind(file);

    char *source = (char *)calloc(sizeof(char), len);
    if (!source)
    {
        fprintf(stderr, "Error: Could not allocate memory for source string\n");
        exit(EXIT_FAILURE);
    }
    fread(source, sizeof(char), len, file);
    fclose(file);
    return source;
}



int finalise(const t_param* params, float** h_cells_ptr, float** h_tmp_cells_ptr,
       int** obstacles_ptr, double** av_vels_ptr)
{
  /* 
  ** free up allocated memory
  */
  free(*h_cells_ptr);
  *h_cells_ptr = NULL;

  free(*h_tmp_cells_ptr);
  *h_tmp_cells_ptr = NULL;

  free(*obstacles_ptr);
  *obstacles_ptr = NULL;

  free(*av_vels_ptr);
  *av_vels_ptr = NULL;

  return EXIT_SUCCESS;
}


void die(const char* message, const int line, const char *file)
{
  fprintf(stderr, "Error at line %d of file %s:\n", line, file);
  fprintf(stderr, "%s\n",message);
  fflush(stderr);
  exit(EXIT_FAILURE);
}



void usage(const char* exe)
{
  fprintf(stderr, "Usage: %s <paramfile> <obstaclefile>\n", exe);
  exit(EXIT_FAILURE);
}
