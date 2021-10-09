//-----------------------------------
// Global variables
//-----------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : WORLD;
float4x4 gViewInverse : VIEWINVERSE;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

static const float3 gLightDirection = { 0.577f,-0.577f,0.577f };
static const float PI = 3.14;
static const float gLightIntensity = 2.0f;
static const float gShininess = 25.0f;

// Global States
RasterizerState gCullBack
{
	CullMode = back;
	FrontCounterClockwise = true;
};

RasterizerState gCullFront
{
	CullMode = front;
	FrontCounterClockwise = true;
};

RasterizerState gCullNone
{
	CullMode = none;
	FrontCounterClockwise = true;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//-----------------------------------
//	Input/Output Structs
//-----------------------------------

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITIONT;
	float3 Color : COLOR;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//------------------------------------
//	Vertex Shader
//------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = float4(input.Position, 1.f);
	output.Position = mul(output.Position, gWorldViewProj);
	output.Color = input.Color;
	output.Tex = input.Tex;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	output.WorldPosition = float4(input.Position, 1.f);
	output.WorldPosition = mul(output.WorldPosition, gWorld);
	return output;
}

//------------------------------------
//	Pixel Shader
//------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; //Border, Mirror, Clamp, Wrap
	AddressV = Wrap;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap; //Border, Mirror, Clamp, Wrap
	AddressV = Wrap;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap; //Border, Mirror, Clamp, Wrap
	AddressV = Wrap;
};

float4 GetDiffuse(VS_OUTPUT input, SamplerState samplerState)
{
	return float4(gDiffuseMap.Sample(samplerState, input.Tex));
}

float3 GetNormal(VS_OUTPUT input, SamplerState samplerState) : SV_TARGET
{
	float3 binormal = cross(input.Normal, input.Tangent);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);

	float3 sampledNormal = gNormalMap.Sample(samplerState, input.Tex).xyz;
	sampledNormal = 2.0f * sampledNormal - float3(1.0f, 1.0f, 1.0f);

	return mul(sampledNormal, tangentSpaceAxis);
}

float GetPhong(VS_OUTPUT input, float3 vertexNormal) : SV_TARGET
{
	float dotLightNormal = dot(-vertexNormal, gLightDirection);
	//if dot product is negative, default back to zero to evade IF-statement
	dotLightNormal = clamp(dotLightNormal, 0.0f, 1.0f);
	return gLightIntensity * dotLightNormal;
}

float4 GetSpecularColor(VS_OUTPUT input, float3 vertexNormal, SamplerState samplerState) : SV_TARGET
{
	float glossiness = gGlossinessMap.Sample(samplerState, input.Tex).r;
	float4 specularColor = gSpecularMap.Sample(samplerState, input.Tex);

	float3 reflected = normalize(reflect(gLightDirection, vertexNormal));
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
	float cosAlpha = dot(reflected, -viewDirection);
	cosAlpha = clamp(cosAlpha, 0.0f, 1.0f);
	float specularReflection = pow(cosAlpha, glossiness * gShininess);

	return specularColor * specularReflection;
}

float4 PS(VS_OUTPUT input, SamplerState samplerState) : SV_TARGET
{
	float3 vertexNormal = GetNormal(input, samplerState);
	float phong = GetPhong(input, vertexNormal);

	float4 diffuseColor = GetDiffuse(input, samplerState) * phong;
	float4 specularColor = GetSpecularColor(input, vertexNormal, samplerState);
	return diffuseColor + specularColor;
}

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{
	return PS(input, samPoint);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{
	return PS(input, samLinear);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
	return PS(input, samAnisotropic);
}

//------------------------------------
//	Technique
//------------------------------------
technique11 PointTechniqueCullBack
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Point() ) );
	}
}

technique11 LinearTechniqueCullBack
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Linear() ) );
	}
}

technique11 AnisotropicTechniqueCullBack
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Anisotropic() ) );
	}
}

technique11 PointTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Point() ) );
	}
}

technique11 LinearTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Linear() ) );
	}
}

technique11 AnisotropicTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Anisotropic() ) );
	}
}

technique11 PointTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Point() ) );
	}
}

technique11 LinearTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Linear() ) );
	}
}

technique11 AnisotropicTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS_Anisotropic() ) );
	}
}

