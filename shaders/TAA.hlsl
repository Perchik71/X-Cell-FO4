// AMD Cauldron code
// 
// Copyright(c) 2020 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define RADIUS          1
#define GROUP_SIZE      16
#define TILE_DIM        (2 * RADIUS + GROUP_SIZE)

Texture2D ColorBuffer : register(t0);
RWTexture2D<float4> OutputBuffer : register(u0);
SamplerState PointerSampler : register(s0);

groupshared float3 Tile[TILE_DIM * TILE_DIM];

/**********************************************************************
MIT License

Copyright(c) 2019 MJP

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
********************************************************************/
float3 SampleHistoryCatmullRom(in float2 uv, in float2 texelSize)
{
    // Source: https://gist.github.com/TheRealMJP/c83b8c0f46b63f3a88a5986f4fa982b1
    // License: https://gist.github.com/TheRealMJP/bc503b0b87b643d3505d41eab8b332ae

    // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
    // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
    // location [1, 1] in the grid, where [0, 0] is the top left corner.
    float2 samplePos = uv / texelSize;
    float2 texPos1 = floor(samplePos - 0.5f) + 0.5f;

    // Compute the fractional offset from our starting texel to our original sample location, which we'll
    // feed into the Catmull-Rom spline function to get our filter weights.
    float2 f = samplePos - texPos1;

    // Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
    // These equations are pre-expanded based on our knowledge of where the texels will be located,
    // which lets us avoid having to evaluate a piece-wise function.
    float2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
    float2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
    float2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
    float2 w3 = f * f * (-0.5f + 0.5f * f);

    // Work out weighting factors and sampling offsets that will let us use bilinear filtering to
    // simultaneously evaluate the middle 2 samples from the 4x4 grid.
    float2 w12 = w1 + w2;
    float2 offset12 = w2 / (w1 + w2);

    // Compute the final UV coordinates we'll use for sampling the texture
    float2 texPos0 = texPos1 - 1.0f;
    float2 texPos3 = texPos1 + 2.0f;
    float2 texPos12 = texPos1 + offset12;

    texPos0 *= texelSize;
    texPos3 *= texelSize;
    texPos12 *= texelSize;

    float3 result = float3(0.0f, 0.0f, 0.0f);

    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos0.x, texPos0.y), 0.0f).xyz * w0.x * w0.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos12.x, texPos0.y), 0.0f).xyz * w12.x * w0.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos3.x, texPos0.y), 0.0f).xyz * w3.x * w0.y;

    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos0.x, texPos12.y), 0.0f).xyz * w0.x * w12.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos12.x, texPos12.y), 0.0f).xyz * w12.x * w12.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos3.x, texPos12.y), 0.0f).xyz * w3.x * w12.y;

    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos0.x, texPos3.y), 0.0f).xyz * w0.x * w3.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos12.x, texPos3.y), 0.0f).xyz * w12.x * w3.y;
    result += ColorBuffer.SampleLevel(PointerSampler, float2(texPos3.x, texPos3.y), 0.0f).xyz * w3.x * w3.y;

    return max(result, 0.0f);
}

float3 Reinhard(in float3 hdr)
{
    return hdr / (hdr + 1.0f);
}

float3 Tap(in float2 pos)
{
    return Tile[int(pos.x) + TILE_DIM * int(pos.y)];
}

[numthreads(GROUP_SIZE, GROUP_SIZE, 1)]
void main(uint3 globalID : SV_DispatchThreadID, uint3 localID : SV_GroupThreadID, uint localIndex : SV_GroupIndex, uint3 groupID : SV_GroupID)
{
    int3 dims;

    // Populate private memory
    ColorBuffer.GetDimensions(0, dims.x, dims.y, dims.z);
    const float2 texelSize = 1.0f / float2(dims.xy);
    const float2 uv = (globalID.xy + 0.5f) * texelSize;
    const float2 tilePos = localID.xy + RADIUS + 0.5f;

    // Populate local memory
    if (localIndex < TILE_DIM * TILE_DIM / 4)
    {
        const int2 anchor = groupID.xy * GROUP_SIZE - RADIUS;

        const int2 coord1 = anchor + int2(localIndex % TILE_DIM, localIndex / TILE_DIM);
        const int2 coord2 = anchor + int2((localIndex + TILE_DIM * TILE_DIM / 4) % TILE_DIM, (localIndex + TILE_DIM * TILE_DIM / 4) / TILE_DIM);
        const int2 coord3 = anchor + int2((localIndex + TILE_DIM * TILE_DIM / 2) % TILE_DIM, (localIndex + TILE_DIM * TILE_DIM / 2) / TILE_DIM);
        const int2 coord4 = anchor + int2((localIndex + TILE_DIM * TILE_DIM * 3 / 4) % TILE_DIM, (localIndex + TILE_DIM * TILE_DIM * 3 / 4) / TILE_DIM);

        const float2 uv1 = (coord1 + 0.125f) * texelSize;
        const float2 uv2 = (coord2 + 0.125f) * texelSize;
        const float2 uv3 = (coord3 + 0.125f) * texelSize;
        const float2 uv4 = (coord4 + 0.125f) * texelSize;

        const float3 color0 = ColorBuffer.SampleLevel(PointerSampler, uv1, 0.0f).xyz;
        const float3 color1 = ColorBuffer.SampleLevel(PointerSampler, uv2, 0.0f).xyz;
        const float3 color2 = ColorBuffer.SampleLevel(PointerSampler, uv3, 0.0f).xyz;
        const float3 color3 = ColorBuffer.SampleLevel(PointerSampler, uv4, 0.0f).xyz;

        Tile[localIndex] = Reinhard(color0);
        Tile[localIndex + TILE_DIM * TILE_DIM / 4] = Reinhard(color1);
        Tile[localIndex + TILE_DIM * TILE_DIM / 2] = Reinhard(color2);
        Tile[localIndex + TILE_DIM * TILE_DIM * 3 / 4] = Reinhard(color3);
    }
    GroupMemoryBarrierWithGroupSync();
    
    // Iterate the neighboring samples
    if (any(int2(globalID.xy) >= dims.xy))
        return; // out of bounds
    
    
    const float3 center = Tap(tilePos); // retrieve center value
    OutputBuffer[globalID.xy] = float4(center, 1.0f);
}
