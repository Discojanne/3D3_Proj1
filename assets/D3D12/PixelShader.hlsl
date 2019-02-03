struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
//#ifdef NORMAL
//    float4 norm		: NORMAL;
//#endif  
//#ifdef TEXTCOORD
//    float2 uv       : TEXTCOORD;
//#else
//    float4 color : COLOR;
//#endif
    
};

float4 main( VSOut input ) : SV_TARGET0
{
	return input.color;
}