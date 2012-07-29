#include <stdio.h>
#include <CL/opencl.h>

#include "main.h"
#include "cl_abstractions.h"
#include "cl_error.h"

/*
 * Creates a context using available devices and creates the command queue.
 * Some information about the platform and available devices is printed to
 * stdout.
 */
void cl_initialise(cl_vars *cl)
{
    cl_int  err_ret;
    size_t  n_bytes;
    char    *platform_name;
    char    *platform_version;
    char    *device_name;

    // Retrieve the platform
    err_ret = clGetPlatformIDs(1, &cl->platform, NULL);
    check_error(__FILE__, __LINE__, err_ret);

    // Retrieve the platform name
    err_ret = clGetPlatformInfo(cl->platform, CL_PLATFORM_NAME, 0, NULL,
        &n_bytes);
    check_error(__FILE__, __LINE__, err_ret);
    platform_name = malloc(n_bytes);
    err_ret = clGetPlatformInfo(cl->platform, CL_PLATFORM_NAME, n_bytes,
        platform_name, NULL);
    check_error(__FILE__, __LINE__, err_ret);
    fprintf(stderr, "CL_PLATFORM_NAME = %s\n", platform_name);

    // Retrieve the platform version
    err_ret = clGetPlatformInfo(cl->platform, CL_PLATFORM_VERSION, 0, NULL,
        &n_bytes);
    check_error(__FILE__, __LINE__, err_ret);
    platform_version = malloc(n_bytes);
    err_ret = clGetPlatformInfo(cl->platform, CL_PLATFORM_VERSION, n_bytes,
        platform_version, NULL);
    check_error(__FILE__, __LINE__, err_ret);
    fprintf(stderr, "CL_PLATFORM_VERSION = %s\n", platform_version);
    fprintf(stderr, "\n");

    // Retrieve a list of devices
    err_ret = clGetDeviceIDs(cl->platform, CL_DEVICE_TYPE_GPU, 0, NULL,
        &cl->n_devices);
    check_error(__FILE__, __LINE__, err_ret);
    cl->devices = malloc(cl->n_devices*sizeof(cl_device_id));
    err_ret = clGetDeviceIDs(cl->platform, CL_DEVICE_TYPE_GPU, cl->n_devices,
        cl->devices, NULL);
    check_error(__FILE__, __LINE__, err_ret);

    // Retrieve device info
    for (int i = 0; i < cl->n_devices; i++)
    {
        err_ret = clGetDeviceInfo(cl->devices[i], CL_DEVICE_NAME, 0, NULL,
            &n_bytes);
        check_error(__FILE__, __LINE__, err_ret);
        device_name = malloc(n_bytes);
        err_ret = clGetDeviceInfo(cl->devices[i], CL_DEVICE_NAME, n_bytes,
            device_name, NULL);
        check_error(__FILE__, __LINE__, err_ret);
        fprintf(stderr, "[Device %d] CL_DEVICE_NAME = %s\n", i, device_name);
    }
    fprintf(stderr, "\n");

    // If the device was unspecified exit after listing available devices
    if (cl->device_id == -1)
    {
        exit(EXIT_SUCCESS);
    }

    // Create a context 
    cl->context = clCreateContext(NULL, cl->n_devices, cl->devices, NULL,
        NULL, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);

    // Create a command queue for the desired device
    cl->queue = clCreateCommandQueue(cl->context,
        cl->devices[cl->device_id], 0, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);

    // Free allocated memory
    free(platform_name);
    free(platform_version);
    free(device_name);
}

/*
 * Builds the program in the specified file.
 */
void cl_create_program(cl_vars *cl, cl_program *program, char *filename)
{
    FILE    *fp;
    size_t  length;
    char    *source;

    cl_int          err_ret;
    size_t          n_bytes;
    cl_build_status build_status;
    char            *build_log;

    // Open the file
    fp = fopen(filename, "r");

    // Check the return value
    if (fp == NULL)
    {
        fprintf(stderr, "%s: ", filename);
        perror("");
        exit(EXIT_FAILURE);
    }

    // Get the length of the file
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate memory and read in the source code
    source = malloc(length+1);
    fread(source, 1, length, fp);
    source[length] = '\0';

    // Create program and compile from source
    *program = clCreateProgramWithSource(cl->context, 1,
        (const char **)&source, &length, &err_ret);
    check_error(__FILE__, __LINE__, err_ret);
    clBuildProgram(*program, 0, NULL, NULL, NULL, NULL);
    check_error(__FILE__, __LINE__, err_ret);

    // Check the build status for errors
    err_ret = clGetProgramBuildInfo(*program, cl->devices[cl->device_id],
        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, 0);
    check_error(__FILE__, __LINE__, err_ret);
    
    // If the build was unsuccessful
    if (build_status != CL_BUILD_SUCCESS)
    {
        // Get the build log
        err_ret = clGetProgramBuildInfo(*program, cl->devices[cl->device_id],
            CL_PROGRAM_BUILD_LOG, 0, NULL, &n_bytes);
        check_error(__FILE__, __LINE__, err_ret);
        build_log = malloc(n_bytes);
        err_ret = clGetProgramBuildInfo(*program, cl->devices[cl->device_id],
            CL_PROGRAM_BUILD_LOG, n_bytes, build_log, NULL);
        check_error(__FILE__, __LINE__, err_ret);

        // Print the build log
        fprintf(stderr, "%s build log:\n", filename);
        fprintf(stderr, "%s\n", build_log);
        exit(EXIT_FAILURE);
    }

    // Free allocated memory and close the file
    free(source);
    fclose(fp);
}

/*
 * Creates a kernel using the supplied program.
 */
void cl_create_kernel(cl_vars *cl, cl_program *program, cl_kernel *kernel, char
    *kernel_name)
{
    cl_int err_ret;

    // Create kernel
    *kernel = clCreateKernel(*program, kernel_name, &err_ret);
    if (err_ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to create kernel \"%s\"\n", kernel_name);
    }
    check_error(__FILE__, __LINE__, err_ret);
}

/*
 * Flushes the command queue and releases the OpenCL objects.
 */
void cl_terminate(cl_vars *cl, cl_program *program, cl_kernel *kernel)
{
    cl_int err_ret;

    // Flush and finish command queue
    err_ret = clFlush(cl->queue);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clFinish(cl->queue);
    check_error(__FILE__, __LINE__, err_ret);

    // Release OpenCL objects
    err_ret = clReleaseKernel(*kernel);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clReleaseProgram(*program);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clReleaseCommandQueue(cl->queue);
    check_error(__FILE__, __LINE__, err_ret);
    err_ret = clReleaseContext(cl->context);
    check_error(__FILE__, __LINE__, err_ret);
}
