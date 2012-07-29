__kernel void zero_spectrum(__global float2 *spec)
{
    int idx = get_global_id(0);

    spec[idx].x = 0;
    spec[idx].y = 0;
}

__kernel void add_spectrum(__global const float2 *a, __global float2 *b)
{
    int idx = get_global_id(0);

    b[idx].x += a[idx].x;
    b[idx].y += a[idx].y;
}
