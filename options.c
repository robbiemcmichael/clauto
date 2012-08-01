#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "main.h"
#include "options.h"

/*
 * Processes the command-line options.
 */
void options(int argc, char *argv[], ga_settings *settings)
{
    int c;
    int fail = 0;

    // Default settings
    settings->device_id = -1;
    settings->input_type = INPUT_NONE;

    for (;;)
    {
        // Create the options struct
        static struct option long_options[] =
        {
            {"device", required_argument, NULL, 'd'},
            {"stdin", no_argument, NULL, 256},
            {"file", required_argument, NULL, 257},
            {"port", required_argument, NULL, 'p'},
            {"spc", required_argument, NULL, 'a'},
            {"batchsize", required_argument, NULL, 'b'},
            {"bins", required_argument, NULL, 'n'},
            {"loops", required_argument, NULL, 'g'},
            {"encoding", required_argument, NULL, 'e'},
            {"channels", required_argument, NULL, 'c'},
            {NULL, 0, NULL, 0}
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "d:p:a:b:n:g:e:c:", long_options,
            &option_index);

        // No more options, exit the loop
        if (c == -1)
        {
            break;
        }

        // Set the variables in the opt struct
        switch (c)
        {
            case 'd':
                settings->device_id = atoi(optarg);
                break;

            case 256:
                if (settings->input_type != INPUT_NONE)
                {
                    fprintf(stderr, "Input type has been specified multiple "
                        "times\n");
                    exit(EXIT_FAILURE);
                }
                settings->input_type = INPUT_STDIN;
                break;

            case 257:
                if (settings->input_type != INPUT_NONE)
                {
                    fprintf(stderr, "Input type has been specified multiple "
                        "times\n");
                    exit(EXIT_FAILURE);
                }
                settings->input_type = INPUT_FILE;
                settings->input_file = malloc(strlen(optarg)+1);
                strcpy(settings->input_file, optarg);
                break;

            case 'p':
                if (settings->input_type != INPUT_NONE)
                {
                    fprintf(stderr, "Input type has been specified multiple "
                        "times\n");
                    exit(EXIT_FAILURE);
                }
                settings->input_type = INPUT_NETWORK;
                settings->port = atoi(optarg);
                break;

            case 'a':
                settings->spc = 1 << atoi(optarg);
                break;

            case 'b':
                settings->batch_size = 1 << atoi(optarg);
                break;

            case 'n':
                settings->bins = 1 << atoi(optarg);
                break;

            case 'g':
                settings->loops = atoi(optarg);
                break;

            case 'e':
                if (strcmp(optarg, "vlba") == 0 || strcmp(optarg, "VLBA") == 0)
                {
                    settings->encoding = ENC_VLBA;
                }
                else if (strcmp(optarg, "at") == 0 ||
                    strcmp(optarg, "AT") == 0)
                {
                    settings->encoding = ENC_AT;
                }
                else
                {
                    fprintf(stderr, "Encoding must be one of: vlba, at\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'c':
                settings->channels = atoi(optarg);
                break;

            case '?':
            default:
                fail = 1;
                break;
        }
    }

    // In the event of invalid options, print an error and exit
    if (fail)
    {
        fprintf(stderr, "Invalid command-line options supplied\n");
        exit(EXIT_FAILURE);
    }

    // If there is an extra argument
    if (optind == argc-1)
    {
        // If the input type has already been specified
        if (settings->input_type != INPUT_NONE)
        {
            fprintf(stderr, "Input type has been specified multiple times\n");
            exit(EXIT_FAILURE);
        }

        // Take the extra argument as the input filename
        settings->input_type = INPUT_FILE;
        settings->input_file = malloc(strlen(argv[optind])+1);
        strcpy(settings->input_file, argv[optind]);
    }
    else if(optind < argc-1)
    {
        // No support for multiple input files
        fprintf(stderr, "Only one input file may be specified\n");
        exit(EXIT_FAILURE);
    }

    if (settings->input_type == INPUT_NONE)
    {
        // If no input type specified, assume input type is stdin
        settings->input_type = INPUT_STDIN;
    }

    // Ensure spc, batch_size and bins are all specified
    if (settings->spc != 0 && settings->batch_size != 0 && settings->bins != 0)
    {
        // If all three are specified, but incorrectly
        if (settings->spc != (settings->bins)*(settings->batch_size))
        {
            fprintf(stderr, "Samples per channel != bins * batch size\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (settings->spc == 0)
    {
        // Determine the number of samples per channel
        settings->spc = (settings->bins)*(settings->batch_size);
    }
    else if (settings->batch_size == 0)
    {
        // Determine the batch size
        settings->batch_size = (settings->spc)/(settings->bins);
    }
    else if (settings->bins == 0)
    {
        // Determine the number of FFT bins
        settings->bins = (settings->spc)/(settings->batch_size);
    }

    // If one or more is still equal to zero
    if (settings->spc == 0 || settings->batch_size == 0 || settings->bins == 0)
    {
        fprintf(stderr, "Must specify at least two of options: -a, -b, -n\n");
        exit(EXIT_FAILURE);
    }

    // Ensure spc > batch_size and bins
    if (settings->spc < settings->batch_size)
    {
        fprintf(stderr, "Samples per channel must be greater than batch "
            "size\n");
        exit(EXIT_FAILURE);
    }
    else if (settings->spc < settings->bins)
    {
        fprintf(stderr, "Samples per channel must be greater than the number "
            "of FFT bins\n");
        exit(EXIT_FAILURE);
    }

    // TODO: If not specified, make sure they're determined from the header
    settings->bps = 2;
    // settings->channels = 8; (specified with -c for the moment)
    // Calculate some other useful variables (requires the above variables)
    settings->n = (settings->spc)*(settings->channels);
    settings->bytes = (settings->n)*(settings->bps)/8;
    settings->output_length = (settings->bins)/2*(settings->channels);
}
