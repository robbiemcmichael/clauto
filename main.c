#include <stdio.h>
#include <string.h>
#include <time.h>
#include <CL/opencl.h>

#include "main.h"
#include "options.h"
#include "data_handling.h"
#include "cl_abstractions.h"
#include "cl_error.h"
#include "convert.h"
#include "fft.h"
#include "sum.h"
#include "spectrum.h"

#define TIMER_DIV (CLOCKS_PER_SEC/1000)

int main(int argc, char *argv[])
{
    cl_int      err_ret;

    // TODO: temporary timers
    clock_t t_start = clock();

    // Process the command-line options
    ga_settings *settings = malloc(sizeof(ga_settings));
    memset(settings, 0, sizeof(ga_settings));
    options(argc, argv, settings);

    // Create variables needed for OpenCL
    cl_vars *cl = malloc(sizeof(cl_vars));
    cl->device_id = settings->device_id;

    // Create the context and command queue
    cl_initialise(cl);
    
    // Allocate memory on the host
    unsigned int *host_input = malloc(settings->bytes);
    cl_float2 *host_output = malloc(settings->output_length*sizeof(cl_float2));

    // Initialise input method
    input_initialise(settings);

    // Initialise kernels
    convert_initialise(settings, cl);
    fft_initialise(settings, cl);
    sum_initialise(cl);
    spectrum_initialise(cl);

    // Create device memory objects
    cl_mem dev_input = clCreateBuffer(cl->context, CL_MEM_READ_ONLY,
        settings->bytes, NULL, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);
    cl_mem dev_data = clCreateBuffer(cl->context, CL_MEM_READ_WRITE,
        settings->n*sizeof(cl_float2), NULL, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);
    cl_mem dev_spectrum = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY,
        settings->output_length*sizeof(cl_float2), NULL, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);
    cl_mem dev_output = clCreateBuffer(cl->context, CL_MEM_WRITE_ONLY,
        settings->output_length*sizeof(cl_float2), NULL, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);

    zero_spectrum(settings, cl, dev_spectrum);
    zero_spectrum(settings, cl, dev_output);

    // TODO: temporary timers
    fprintf(stderr, "Initialisation overhead = %f\n\n", (float)(clock() - t_start)/TIMER_DIV);

    float t_module[5] = {0};
    clock_t t_loop;
    clock_t start;
    clock_t stop;

    t_loop = clock();
    int loops = 0;
    for (int p = 0; p < settings->loops || settings->loops == 0; p++)
    {
        start = clock();

        // Read in the data
        int r_bytes = read_data(settings, host_input, settings->bytes);

        if (r_bytes != settings->bytes)
        {
            // Number of bytes read does not match number of bytes required
            fprintf(stderr, "Unable to read %d bytes (only read %d bytes)\n\n",
                settings->bytes, r_bytes);

            // Indicates EOF (with some data unused), break out of main loop
            break;
        }

        clFinish(cl->queue);
        stop = clock();
        t_module[0] += (float)(stop - start)/TIMER_DIV;
        start = clock();

        // Transfer input data to device
        err_ret = clEnqueueWriteBuffer(cl->queue, dev_input, CL_TRUE, 0,
            settings->bytes, host_input, 0, NULL, NULL);
        check_error(__FILE__, __LINE__, err_ret);

        clFinish(cl->queue);
        stop = clock();
        t_module[1] += (float)(stop - start)/TIMER_DIV;
        start = clock();

        // Execute convert module
        convert_module(settings, cl, dev_input, dev_data);

        clFinish(cl->queue);
        stop = clock();
        t_module[2] += (float)(stop - start)/TIMER_DIV;
        start = clock();

        // Execute FFT module
        fft_module(settings, cl, dev_data);

        clFinish(cl->queue);
        stop = clock();
        t_module[3] += (float)(stop - start)/TIMER_DIV;
        start = clock();

        // Execute the sum module
        sum_module(settings, cl, dev_data, dev_spectrum);

        clFinish(cl->queue);
        stop = clock();
        t_module[4] += (float)(stop - start)/TIMER_DIV;

        loops++;
    }

    fprintf(stderr, "-- Timing information for %d loops:\n", loops);
    fprintf(stderr, "Read = %f\n", t_module[0]);
    fprintf(stderr, "Transfer = %f\n", t_module[1]);
    fprintf(stderr, "Convert = %f\n", t_module[2]);
    fprintf(stderr, "FFT = %f\n", t_module[3]);
    fprintf(stderr, "Sum = %f\n", t_module[4]);
    fprintf(stderr, "-- Loop total = %f\n\n", (float)(clock() - t_loop)/TIMER_DIV);

    // TODO: This will be moved into an if statement in the loop
    add_spectrum(settings, cl, dev_spectrum, dev_output);
    zero_spectrum(settings, cl, dev_spectrum);

    // Copy result back to host
    err_ret = clEnqueueReadBuffer(cl->queue, dev_output, CL_TRUE, 0,
        settings->output_length*sizeof(cl_float2), host_output, 0, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);

    // Block until the output has been transferred to the host
    clFinish(cl->queue);

    // Print the result
    for (int i = 0; i < settings->output_length; i++)
    {
        if (i % (settings->bins/2) == 0)
        {
            printf("\n");
        }

        float *elem = (float *)(&host_output[i]);
        printf("%f\n", *elem);
    }

    // Release buffers
    err_ret = clReleaseMemObject(dev_input);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clReleaseMemObject(dev_data);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clReleaseMemObject(dev_output);
    check_error(__FILE__, __LINE__, err_ret);

    // Free allocated memory on host
    free(host_input);
    free(host_output);

    // TODO: temporary timers
    fprintf(stderr, "Total execution time = %f\n\n", (float)(clock() - t_start)/TIMER_DIV);

    return 0;
}
