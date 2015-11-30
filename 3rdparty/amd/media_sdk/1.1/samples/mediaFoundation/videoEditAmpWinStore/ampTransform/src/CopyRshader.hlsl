Texture2D<float> inTexture;
RWTexture2D<float> outTexture;

[numthreads(2,2,1)]
void main( uint3 DTid: SV_DispatchThreadID )
{
    outTexture[ DTid.xy ] = inTexture[ DTid.xy ];
}