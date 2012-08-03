#include <stdio.h>
#include <CL/opencl.h>

#include "main.h"
#include "cl_abstractions.h"
#include "cl_error.h"
#include "convert.h"

#define HI_MAG 3.3359

cl_kernel *convert_kernel;

void convert_initialise(ga_settings *settings, cl_vars *cl)
{
    cl_int      err_ret;
    cl_program  *program;

    // Create the program
    program = malloc(sizeof(cl_program));
    cl_create_program(cl, program, "convert.cl");

    // Allocate memory for the kernel
    convert_kernel = malloc(sizeof(cl_kernel));

    // Select the kernel based on settings
    if (settings->channels == 4)
    {
        cl_create_kernel(cl, program, convert_kernel, "convert_2bit_4chan");
    }
    else if (settings->channels == 8)
    {
        cl_create_kernel(cl, program, convert_kernel, "convert_2bit_8chan");
    }
}

void convert_module(ga_settings *settings, cl_vars *cl, cl_mem dev_input,
    cl_mem dev_data)
{
    cl_int      err_ret;
    size_t      global_work_size[1];
    size_t      local_work_size[1];
    float       lut[4];

    // Set work size
    int nt = MIN(settings->spc, cl->max_work_size);
    global_work_size[0] = settings->spc;
    local_work_size[0] = nt;

    // Create the LUT according to the encoding scheme
    if (settings->bps == 2 && settings->encoding == ENC_VLBA)
    {
        // 2-bit VLBA
        lut[0] = -HI_MAG;
        lut[1] = 1.0;
        lut[2] = -1.0;
        lut[3] = HI_MAG;
    }
    else if (settings->bps == 2 && settings->encoding == ENC_AT)
    {
        // 2-bit AT
        lut[0] = 1.0;
        lut[1] = -1.0;
        lut[2] = HI_MAG;
        lut[3] = -HI_MAG;
    }
    else
    {
        fprintf(stderr, "Unknown encoding\n");
        exit(EXIT_FAILURE);
    }

    // Set kernel arguments
    err_ret = clSetKernelArg(*convert_kernel, 0, sizeof(dev_input),
        (void *)&dev_input);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*convert_kernel, 1, sizeof(dev_data),
        (void *)&dev_data);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*convert_kernel, 2, nt*sizeof(cl_int), NULL);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*convert_kernel, 3, sizeof(lut), (void *)&lut);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clSetKernelArg(*convert_kernel, 4, sizeof(settings->spc),
        (void *)&settings->spc);
    check_error(__FILE__, __LINE__, err_ret);

    // Execute kernel
    err_ret = clEnqueueNDRangeKernel(cl->queue, *convert_kernel, 1, NULL,
        global_work_size, local_work_size, 0, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);
}
