#include <stdio.h>
#include <CL/opencl.h>

#include "main.h"
#include "cl_abstractions.h"
#include "cl_error.h"
#include "spectrum.h"

cl_kernel *zero_kernel;
cl_kernel *add_kernel;

void spectrum_initialise(cl_vars *cl)
{
    cl_int      err_ret;
    cl_program  *program;

    // Create the program
    program = malloc(sizeof(cl_program));
    cl_create_program(cl, program, "spectrum.cl");

    // Create the kernels
    zero_kernel = malloc(sizeof(cl_kernel));
    cl_create_kernel(cl, program, zero_kernel, "zero_spectrum");
    add_kernel = malloc(sizeof(cl_kernel));
    cl_create_kernel(cl, program, add_kernel, "add_spectrum");
}

void zero_spectrum(ga_settings *settings, cl_vars *cl, cl_mem dev_spectrum)
{
    cl_int      err_ret;

    size_t  global_work_size[1];
    size_t  local_work_size[1];

    // Set work size
    int nt = MIN(settings->output_length, cl->max_work_size);
    global_work_size[0] = settings->output_length;
    local_work_size[0] = nt;

    // Set kernel arguments
    err_ret = clSetKernelArg(*zero_kernel, 0, sizeof(cl_mem),
        (void *)&dev_spectrum);
    check_error(__FILE__, __LINE__, err_ret);

    // Execute kernel
    err_ret = clEnqueueNDRangeKernel(cl->queue, *zero_kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);
}

void add_spectrum(ga_settings *settings, cl_vars *cl, cl_mem dev_a,
    cl_mem dev_b)
{
    cl_int      err_ret;

    size_t  global_work_size[1];
    size_t  local_work_size[1];

    // Set work size
    int nt = MIN(settings->output_length, cl->max_work_size);
    global_work_size[0] = settings->output_length;
    local_work_size[0] = nt;

    // Set kernel arguments
    err_ret = clSetKernelArg(*add_kernel, 0, sizeof(cl_mem), (void *)&dev_a);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*add_kernel, 1, sizeof(cl_mem), (void *)&dev_b);
    check_error(__FILE__, __LINE__, err_ret);

    // Execute kernel
    err_ret = clEnqueueNDRangeKernel(cl->queue, *add_kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);
}
