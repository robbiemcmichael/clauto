#include <CL/opencl.h>

#include "main.h"
#include "cl_abstractions.h"
#include "cl_error.h"
#include "sum.h"

cl_kernel *sum_kernel;

void sum_initialise(cl_vars *cl)
{
    cl_int      err_ret;
    cl_program  *program;

    // Create the program
    program = malloc(sizeof(cl_program));
    cl_create_program(cl, program, "sum.cl");

    // Create the kernel
    sum_kernel = malloc(sizeof(cl_kernel));
    cl_create_kernel(cl, program, sum_kernel, "sum");
}

void sum_module(ga_settings *settings, cl_vars *cl, cl_mem dev_data,
    cl_mem dev_spectrum)
{
    cl_int      err_ret;
    cl_kernel   *kernel;
    size_t      global_work_size[1];
    size_t      local_work_size[1];

    // Set work size
    int nt = MIN(settings->output_length, cl->max_work_size);
    global_work_size[0] = settings->output_length;
    local_work_size[0] = nt;

    // Set kernel arguments
    err_ret = clSetKernelArg(*sum_kernel, 0, sizeof(dev_data),
        (void *)&dev_data);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*sum_kernel, 1, sizeof(dev_spectrum),
        (void *)&dev_spectrum);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*sum_kernel, 2, sizeof(settings->batch_size),
        (void *)&settings->batch_size);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*sum_kernel, 3, sizeof(settings->spc),
        (void *)&settings->spc);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*sum_kernel, 4, sizeof(settings->bins),
        (void *)&settings->bins);

    // Execute kernel
    err_ret = clEnqueueNDRangeKernel(cl->queue, *sum_kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);
}
