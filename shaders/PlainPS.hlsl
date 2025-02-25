// Copyright © 2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

struct InputData
{
    float4 ClipPosition : SV_POSITION;
    float2 UV : TEXCOORD0;
};

Texture2D colorTex : register(t0);
SamplerState linearSampler : register(s0);

float4 main(InputData Data) : SV_Target
{
    return colorTex.Sample(linearSampler, Data.UV);
}