__kernel void sum(__global const float2 *data,__global float2 *spectrum,
    __const int batch_size, __const int spc, __const int bins)
{
    int idx = get_global_id(0);
    int a = (idx/(bins/2))*spc + idx%(bins/2);

    float x = 0;
    float y = 0;
    for (int s = 0; s < batch_size; s++)
    {
        int d = a + s*bins;

        // TODO: Should be able to calculate cross polarisations here
        x += sqrt(data[d].x*data[d].x + data[d].y*data[d].y);
        y += 0;
    }

    spectrum[idx].x += x;
    spectrum[idx].y += y;
}
