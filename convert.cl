#define HI_MAG 3.3359

__kernel void convert_2bit_4chan(__global const unsigned char *input,
    __global float2 *data, int spc)
{
	int idx = get_global_id(0);

    // TODO: is this faster than just accessing directly from input?
    unsigned char dd = input[idx];

    // TODO: Restricted to 2-bit VLBA encoding scheme for now
    float lut[4] = {-HI_MAG, 1.0, -1.0, HI_MAG}; // VLBA
    //float lut[4] = {1.0, -1.0, HI_MAG, -HI_MAG}; // AT

    // Loop over each channel
    for (int c = 0; c < 4; c++)
    {
        data[c*spc + idx].x = lut[(dd >> (2*c)) & 0x03];
        data[c*spc + idx].y = 0;
    }
}
