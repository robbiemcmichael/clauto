#define HI_MAG 3.3359

__kernel void magnitude(__global float2 *spectra)
{
    int idx = get_global_id(0);

    spectra[idx].x = sqrt(spectra[idx].x*spectra[idx].x +
        spectra[idx].y*spectra[idx].y);
}

__kernel void sum(__global float2 *spectra, int channels, int spc, int dim,
    int b)
{
    int idx = get_global_id(0);
    int p = ((idx/dim)^b)*dim + idx % dim;

    for (int c = 0; c < channels; c++)
    {
        spectra[c*spc + idx].x += spectra[c*spc + p].x;
        spectra[c*spc + idx].y += spectra[c*spc + p].y;
    }
}

__kernel void reorder(__global const float2 *summed, __global float2 *spec,
    int spc, int dim)
{
    int idx = get_global_id(0);
    int p = (idx/dim)*spc + idx % dim;

    spec[idx].x += summed[p].x;
    spec[idx].y += summed[p].y;
}
