#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <CL/opencl.h>

#define M_PI 3.14159265358979323846
 
const char *kernelSource =                                     "\n" \
"__kernel void n_body(  __global double *rx,                    \n" \
"                       __global double *ry,                    \n" \
"                       __global double *vx,                    \n" \
"                       __global double *vy,                    \n" \
"                       __global double *mass,                  \n" \
"                       const unsigned long steps,              \n" \
"                       const unsigned int n)                   \n" \
"{                                                              \n" \
"    // Get thread ID                                           \n" \
"    int id = get_global_id(0);                                 \n" \
"                                                               \n" \
"    double force;                                              \n" \
"    double fx;                                                 \n" \
"    double fy;                                                 \n" \
"                                                               \n" \
"    double distance;                                           \n" \
"    double dx;                                                 \n" \
"    double dy;                                                 \n" \
"                                                               \n" \
"    double d_vx;                                               \n" \
"    double d_vy;                                               \n" \
"    if (id < n) {                                              \n" \
"        for (long step = 0; step < steps; step++){             \n" \
"            fx = 0;                                            \n" \
"            fy = 0;                                            \n" \
"            for (int m2 = 0; m2 < n; m2++){                    \n" \
"                if (m2 == id) { continue; }                    \n" \
"                dx = rx[id] - rx[m2];                          \n" \
"                dy = ry[id] - ry[m2];                          \n" \
"                                                               \n" \
"                distance = sqrt(dx*dx + dy*dy + 0.0001);       \n" \
"                force = 6.674e-11 * (mass[id] * mass[m2] / distance);  \n" \
"                fx += force * (dx / distance);                 \n" \
"                fy += force * (dy / distance);                 \n" \
"            }                                                  \n" \
"        // acceleration = force / mass                         \n" \
"        d_vx = fx / mass[id];                                  \n" \
"        d_vy = fy / mass[id];                                  \n" \
"                                                               \n" \
"        // speed = time * acceleration                         \n" \
"        vx[id] += 1 * d_vx;                                    \n" \
"        vy[id] += 1 * d_vy;                                    \n" \
"                                                               \n" \
"        // distance = time * speed                             \n" \
"        rx[id] += 1 * vx[id]; // We just did a double x/y add?!                            \n" \
"        ry[id] += 1 * vy[id]; // Oh, distance is the double-integration of acceleration!   \n" \
"                                                               \n" \
"        }                                                      \n" \
"                                                               \n" \
"    }                                                          \n" \
"}                                                              \n" \
                                                               "\n" ;
 
int main(int argc, char* argv[])
{
    // Number of bodies
    unsigned int n = 10;
    unsigned long steps = 1000;
 
    // Host vectors
    double *h_rx, *h_rx_after;  // X location
    double *h_ry, *h_ry_after;  // Y location
    double *h_vx;               // X velocity
    double *h_vy;               // Y velocity
    unsigned int *h_radius;     // Radius of the body
    double *h_mass;             // Mass of the body
 
    // Device buffers
    cl_mem d_rx;
    cl_mem d_ry;
    cl_mem d_vx;
    cl_mem d_vy;
    cl_mem d_mass;
 
    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel
 
    // Allocate memory for each vector on host
    h_rx = (double*)calloc(n, sizeof(double));
    h_rx_after = (double*)calloc(n, sizeof(double));
    h_ry = (double*)calloc(n, sizeof(double));
    h_ry_after = (double*)calloc(n, sizeof(double));
    h_vx = (double*)calloc(n, sizeof(double));
    h_vy = (double*)calloc(n, sizeof(double));
    h_radius = (int*)calloc(n, sizeof(unsigned int));
    h_mass = (double*)calloc(n, sizeof(double));
 
    // Initialize vectors on host
    srand(time(NULL));
    for(int i = 0; i < n; i++ )
    {
        h_rx[i] = (rand() % 201) - 100;
        h_ry[i] = (rand() % 201) - 100;
        h_vx[i] = 0;
        h_vy[i] = 0;
        h_radius[i] = (rand() % 9)+1;
        h_mass[i] = h_radius[i] * h_radius[i] * M_PI;
    }
 
    size_t globalSize, localSize;
    cl_int err;
 
    // Number of work items in each local work group
    localSize = 1;
 
    // Number of total work items - localSize must be devisor
    globalSize = ceil(n/(float)localSize)*localSize;
 
    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
 
    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
 
    // Create a context  
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
 
    // Create a command queue 
    queue = clCreateCommandQueue(context, device_id, 0, &err);
 
    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1, (const char **) & kernelSource, NULL, &err);
    if (err != CL_SUCCESS){
        fprintf(stderr, "Reading source for kernel failed\n");
        return err;
    }
 
    // Build the program executable 
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS){
        fprintf(stderr, "Building kernel failed\n");

        size_t len = 0;
        cl_int ret = CL_SUCCESS;
        ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
        char *buffer = calloc(len, sizeof(char));
        ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
        fprintf(stderr, "Build log:\n\n%s\n", buffer);

        return err;
    }
 
    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "n_body", &err);
    if (err != CL_SUCCESS){
        fprintf(stderr, "Creating kernel failed\n");
        return err;
    }
 
    // Create the input and output arrays in device memory for our calculation
    d_rx = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(double), NULL, NULL);
    d_ry = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(double), NULL, NULL);
    d_vx = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(double), NULL, NULL);
    d_vy = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(double), NULL, NULL);
    d_mass = clCreateBuffer(context, CL_MEM_READ_ONLY, n*sizeof(double), NULL, NULL);
 
    // Write our data set into the input array in device memory
    err  = clEnqueueWriteBuffer(queue, d_rx, CL_TRUE, 0, n*sizeof(double), h_rx, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_ry, CL_TRUE, 0, n*sizeof(double), h_ry, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_vx, CL_TRUE, 0, n*sizeof(double), h_vx, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_vy, CL_TRUE, 0, n*sizeof(double), h_vy, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_mass, CL_TRUE, 0, n*sizeof(double), h_mass, 0, NULL, NULL);
    if (err != CL_SUCCESS){
        fprintf(stderr, "Enqueing data buffer writes failed\n");
        return err;
    }
 
    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_rx);        // rx
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_ry);        // ry
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_vx);        // vx
    err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_vy);        // vy
    err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_mass);      // mass
    err |= clSetKernelArg(kernel, 5, sizeof(unsigned long), &steps);// steps
    err |= clSetKernelArg(kernel, 6, sizeof(unsigned int), &n);     // n
    if (err != CL_SUCCESS){
        fprintf(stderr, "Setting kernel arguments failed\n");
        return err;
    }
 
    // Execute the kernel over the entire range of the data set  
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    if (err != CL_SUCCESS){
        fprintf(stderr, "Executing kernel failed\n");
        return err;
    }

    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);
 
    // Read the results from the device
    clEnqueueReadBuffer(queue, d_rx, CL_TRUE, 0, n*sizeof(double), h_rx_after, 0, NULL, NULL );
    clEnqueueReadBuffer(queue, d_ry, CL_TRUE, 0, n*sizeof(double), h_ry_after, 0, NULL, NULL );
 
    // Sum up vector c and print result divided by n, this should equal 1 within error
    for(int i=0; i<n; i++){
        printf("Body %d: (%10.5f, %10.5f)\n", i,h_rx[i],h_ry[i]);
        printf("     -> (%10.5f, %10.5f)\n\n", h_rx_after[i],h_ry_after[i]);
    }
 
    // Release OpenCL resources
    clReleaseMemObject(d_rx);
    clReleaseMemObject(d_ry);
    clReleaseMemObject(d_vx);
    clReleaseMemObject(d_vy);
    clReleaseMemObject(d_mass);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
 
    // Release host memory
    free(h_rx);
    free(h_ry);
    free(h_rx_after);
    free(h_ry_after);
    free(h_vx);
    free(h_vy);
    free(h_radius);
    free(h_mass);
 
    return 0;
}
