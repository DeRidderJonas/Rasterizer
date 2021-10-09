//-----------------------------------
// Global variables
//-----------------------------------
float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;

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
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

BlendState gBlendStateNoTransparany
{
	BlendEnable[0] = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = true;

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
	float3 Color : COLOR;
	float2 Tex : TEXCOORD0;
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

float4 PS(VS_OUTPUT input, SamplerState samplerState) : SV_TARGET
{
	return GetDiffuse(input, samplerState);
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
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullBack
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullBack
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 PointTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullFront
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 PointTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullNone
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 PointTechniqueCullBackNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullBackNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullBackNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 PointTechniqueCullFrontNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullFrontNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullFrontNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullFront);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

technique11 PointTechniqueCullNoneNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Point()));
	}
}

technique11 LinearTechniqueCullNoneNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
	}
}

technique11 AnisotropicTechniqueCullNoneNoTransparancy
{
	pass P0
	{
		SetRasterizerState(gCullNone);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendStateNoTransparany, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
	}
}

