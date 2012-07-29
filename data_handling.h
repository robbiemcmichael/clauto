void input_initialise(ga_settings *settings);
int read_data(ga_settings *settings, unsigned int *h_data, int n_bytes);
int read_header();
int read_data_file(FILE *fp, unsigned int *h_data, int n_bytes);
