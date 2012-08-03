__kernel void convert_2bit_4chan(__global const unsigned char *input,
    __global float2 *data,__local unsigned int *scratch,
    __const float4 lut, __const int spc)
{
	int idx = get_global_id(0);
	int local_idx = get_local_id(0);

    // Load the time sample into local memory
    scratch[local_idx] = input[idx];

    // Interpet the LUT as an array
    float *lp = (float *)&lut;

    // Loop over each channel
    for (int c = 0; c < 4; c++)
    {
        data[c*spc + idx].x = lp[(scratch[local_idx] >> (2*c)) & 0x03];
        data[c*spc + idx].y = 0;
    }
}

__kernel void convert_2bit_8chan(__global const unsigned short *input,
    __global float2 *data, __local unsigned int *scratch,
    __const float4 lut, __const int spc)
{
	int idx = get_global_id(0);
	int local_idx = get_local_id(0);

    // Load the time sample into local memory
    scratch[local_idx] = input[idx];

    // Interpet the LUT as an array
    float *lp = (float *)&lut;

    // Loop over each channel
    for (int c = 0; c < 8; c++)
    {
        data[c*spc + idx].x = lp[(scratch[local_idx] >> (2*c)) & 0x03];
        data[c*spc + idx].y = 0;
    }
}
