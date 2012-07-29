void spectrum_initialise(cl_vars *cl);
void zero_spectrum(ga_settings *settings, cl_vars *cl, cl_mem dev_spectrum);
void add_spectrum(ga_settings *settings, cl_vars *cl, cl_mem dev_a,
    cl_mem dev_b);
