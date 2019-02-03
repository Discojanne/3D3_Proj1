struct VSIn
{
//#ifdef POSITION
//	float3 pos		: POSITION;
//#endif
//#ifdef NORMAL
//    float3 norm		: NORMAL;
//#endif  
//#ifdef TEXTCOORD
//    float2 uv       : TEXTCOORD;
//#endif

//	float3 color	: COLOR;
};

struct VSOut
{
	float4 pos		: SV_POSITION;

//#ifdef NORMAL
//    float4 norm		: NORMAL;
//#endif  
//#ifdef TEXTCOORD
//    float2 uv       : TEXTCOORD;
//#else   
//    float4 color : COLOR;
//#endif
    
};

cbuffer CB : register(b0)
{
    float4x4 world;
//#ifndef TEXTCOORD
//    float4 RGBcolor;
//#endif
}

VSOut main(VSIn input, uint index : SV_VertexID, uint instanceID : SV_InstanceID)
{
	VSOut output	= (VSOut)0;
 
//#ifdef POSITION
//    output.pos = float4(input.pos, 1.0f);
//    output.pos = mul(output.pos, world[instanceID]);
//#else   
//    output.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
//#endif
    

//#ifdef NORMAL
//    output.norm = float4(input.norm, 0.0f);
//#endif  
//#ifdef TEXTCOORD
//    output.uv = input.uv;
//#else
//    output.color = RGBcolor[instanceID];
//#endif

	return output;
}