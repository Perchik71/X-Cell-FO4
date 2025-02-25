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

        const float2 uv1 = (coord1 + 0.5f) * texelSize;
        const float2 uv2 = (coord2 + 0.5f) * texelSize;
        const float2 uv3 = (coord3 + 0.5f) * texelSize;
        const float2 uv4 = (coord4 + 0.5f) * texelSize;

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
    const float3 center = Tap(tilePos); // retrieve center value
    OutputBuffer[globalID.xy] = float4(center, 1.0f);
}
