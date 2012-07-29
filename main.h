#define INPUT_NONE      0
#define INPUT_STDIN     1
#define INPUT_FILE      2
#define INPUT_NETWORK   3

typedef struct
{
    int     device_id;      // OpenCL device id
    int     input_type;     // Input type (stdin, file or network)
    char    *input_file;    // Input filename
    int     port;           // Port to use for network transfer
    int     loops;          // Number of loops to perform
    int     n;              // Total number of samples per loop
    int     spc;            // Samples per channel
    int     bps;            // Bits per sample
    int     channels;       // Number of channels
    int     bytes;          // Number of bytes
    int     batch_size;     // FFT batch size
    int     resolution;     // Output resolution per channel
    int     output_length;  // Total output length
} ga_settings;
