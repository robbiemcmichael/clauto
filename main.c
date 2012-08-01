#include <stdio.h>
#include <string.h>
#include <sys/time.h>
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

void timer_start(struct timeval *t_start)
{
    gettimeofday(t_start, NULL);
}

void timer_stop(struct timeval t_start, char *str, double *acc)
{
    double time;
    struct timeval t_stop;

    gettimeofday(&t_stop, NULL);
    time += (double)(t_stop.tv_sec - t_start.tv_sec) +
        (double)(t_stop.tv_usec - t_start.tv_usec)/1000000;

    if (acc != NULL)
    {
        *acc += time;
    }

    if (str != NULL)
    {
        fprintf(stderr, "%s%.6lf\n", str, time);
    }
}

int main(int argc, char *argv[])
{
    cl_int      err_ret;

    // Create the first timer
    struct timeval t_init;
    timer_start(&t_init);

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

    // Print the initialisation overhead time
    fprintf(stderr, "\n");
    timer_stop(t_init, "-- Initialisation overhead: ", NULL);

    // Create the timers and accumulators for each module
    double t_module[5] = {0};
    struct timeval t_loop;
    struct timeval t_start;
    timer_start(&t_loop);

    int loops = 0;
    for (int p = 0; p < settings->loops || settings->loops == 0; p++)
    {
        timer_start(&t_start);

        // Read in the data
        int r_bytes = read_data(settings, host_input, settings->bytes);

        if (r_bytes != settings->bytes)
        {
            // Number of bytes read does not match number of bytes required
            fprintf(stderr, "Unable to read %d bytes (only read %d bytes)\n",
                settings->bytes, r_bytes);

            // Indicates EOF (with some data unused), break out of main loop
            break;
        }

        timer_stop(t_start, NULL, &t_module[0]);
        timer_start(&t_start);

        // Transfer input data to device
        err_ret = clEnqueueWriteBuffer(cl->queue, dev_input, CL_TRUE, 0,
            settings->bytes, host_input, 0, NULL, NULL);
        check_error(__FILE__, __LINE__, err_ret);

        clFinish(cl->queue);
        timer_stop(t_start, NULL, &t_module[1]);
        timer_start(&t_start);

        // Execute convert module
        convert_module(settings, cl, dev_input, dev_data);

        clFinish(cl->queue);
        timer_stop(t_start, NULL, &t_module[2]);
        timer_start(&t_start);

        // Execute FFT module
        fft_module(settings, cl, dev_data);

        clFinish(cl->queue);
        timer_stop(t_start, NULL, &t_module[3]);
        timer_start(&t_start);

        // Execute the sum module
        sum_module(settings, cl, dev_data, dev_spectrum);

        clFinish(cl->queue);
        timer_stop(t_start, NULL, &t_module[4]);

        loops++;
    }

    // Print the loop timing information
    fprintf(stderr, "-- Timing information for %d loops:\n", loops);
    fprintf(stderr, "--     Read:\t%.6lf\n", t_module[0]);
    fprintf(stderr, "--     H->D:\t%.6lf\n", t_module[1]);
    fprintf(stderr, "--     Convert:\t%.6lf\n", t_module[2]);
    fprintf(stderr, "--     FFT:\t%.6lf\n", t_module[3]);
    fprintf(stderr, "--     Sum:\t%.6lf\n", t_module[4]);
    timer_stop(t_loop, "-- Total loop time: ", NULL);

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

    // Print the total execution time
    timer_stop(t_init, "-- Total execution time: ", NULL);

    return 0;
}
