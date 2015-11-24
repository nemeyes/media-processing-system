Texture2D<float2> inTexture;
RWTexture2D<float2> outTexture;

[numthreads(2,2,1)]
void main( uint3 DTid: SV_DispatchThreadID )
{
    outTexture[ DTid.xy ] = inTexture[ DTid.xy ];
}