#include <math.h>
#include <CL/opencl.h>

#include "main.h"
#include "cl_abstractions.h"
#include "cl_error.h"
#include "fft.h"
#include "clAppleFft.h"

clFFT_Plan plan;

/*
 * Creates the FFT plan in an initialisation routine so that so that it doesn't
 * need to be performed every loop.
 */
void fft_initialise(ga_settings *settings, cl_vars *cl)
{
    cl_int      err_ret;

    // Set the FFT dimension
    clFFT_Dim3  dim = {settings->resolution, 1, 1};

    // Create FFT plan
    plan = clFFT_CreatePlan(cl->context, dim, clFFT_1D, 
            clFFT_InterleavedComplexFormat, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);
}

/*
 * Executes the FFT using the plan.
 */
void fft_module(ga_settings *settings, cl_vars *cl, cl_mem dev_data)
{
    cl_int  err_ret;

    // Determine the number of FFTs to be performed
    int     n_fft = (settings->channels)*(settings->batch_size);

    // Execute the FFT
    err_ret = clFFT_ExecuteInterleaved(cl->queue, plan, n_fft, clFFT_Forward,
        dev_data, dev_data, 0, 0, 0);  
    check_error(__FILE__, __LINE__, err_ret);
}
