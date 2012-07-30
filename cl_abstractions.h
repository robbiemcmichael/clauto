typedef struct
{
    cl_platform_id      platform;
    cl_uint             n_devices;
    cl_device_id        *devices;
    int                 device_id;
    size_t              max_work_size;
    cl_context          context;
    cl_command_queue    queue;
} cl_vars;

void cl_initialise(cl_vars *cl);
void cl_create_program(cl_vars *cl, cl_program *program, char *filename);
void cl_create_kernel(cl_vars *cl, cl_program *program, cl_kernel *kernel, char
    *kernel_name);
void cl_terminate(cl_vars *cl, cl_program *program, cl_kernel *kernel);
