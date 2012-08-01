__kernel void magnitude(__global float2 *spectra)
{
    int idx = get_global_id(0);

    spectra[idx].x = sqrt(spectra[idx].x*spectra[idx].x +
        spectra[idx].y*spectra[idx].y);
}

__kernel void sum(__global float2 *spectra, __const int channels,
    __const int spc, __const int dim, __const int b)
{
    int idx = get_global_id(0);
    int a = (idx/(dim/2))*dim + idx % (dim/2);
    int p = ((a/dim)^b)*dim + a % dim;

    for (int c = 0; c < channels; c++)
    {
        spectra[c*spc + a].x += spectra[c*spc + p].x;
        spectra[c*spc + a].y += spectra[c*spc + p].y;
    }
}

__kernel void reorder(__global const float2 *summed, __global float2 *spec,
    __const int spc, __const int dim)
{
    int idx = get_global_id(0);
    int a = (idx/(dim/2))*dim + idx % (dim/2);
    int p = (a/dim)*spc + a % dim;

    spec[idx].x += summed[p].x;
    spec[idx].y += summed[p].y;
}
