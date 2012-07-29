#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "data_handling.h"

FILE *fp;

/*
 * Depending on the input method, opens the file or sets up the network socket
 */
void input_initialise(ga_settings *settings)
{
    if (settings->input_type == INPUT_FILE)
    {
        // Open the file
        fp = fopen(settings->input_file, "r");

        // Check the return value
        if (fp == NULL)
        {
            fprintf(stderr, "%s: ", settings->input_file);
            perror("");
            exit(EXIT_FAILURE);
        }
    }
    else if (settings->input_type == INPUT_NETWORK)
    {
        // TODO: Network initialisation
    }
}

/*
 * Attempts to read n_bytes from the specified input
 */
int read_data(ga_settings *settings, unsigned int *h_data, int n_bytes)
{
    int r_bytes;

    if (settings->input_type == INPUT_STDIN)
    {
        // Read the data from stdin
        r_bytes = read_data_file(stdin, h_data, n_bytes);
    }
    else if (settings->input_type == INPUT_FILE)
    {
        // Read the data from the input file
        r_bytes = read_data_file(fp, h_data, n_bytes);
    }
    else if (settings->input_type == INPUT_NETWORK)
    {
        // TODO: Read the data from the network
        // read_data_network()
    }

    return r_bytes;
}

/*
 * Reads the header from the input
 */
int read_header()
{
    return 0;
}

/*
 * Reads n_bytes from the input file
 */
int read_data_file(FILE *fp, unsigned int *h_data, int n_bytes)
{
    int r_bytes = fread(h_data, 1, n_bytes, fp);

    return r_bytes;
}
