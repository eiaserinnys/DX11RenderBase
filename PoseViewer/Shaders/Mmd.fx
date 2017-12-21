//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer cbNeverChanges : register(b0)
{
	matrix World;
	matrix ViewProjection;
	float4 EyePos;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(float4(input.Pos.xyz, 1), World);
	output.Pos = mul(output.Pos, ViewProjection);
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	return float4(0, 0, 0, 0.1f);
}

