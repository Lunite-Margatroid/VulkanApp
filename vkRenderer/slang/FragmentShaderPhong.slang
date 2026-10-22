#include "CommonVertexOutput.slang"
#include "ShadeLib.slang"
#include "BRDFLib.slang"
#include "LightDef.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target
{
    LocalSurfaceInfo sSurface = GenLocalSurface(sVertOut);

    FragInfo sFragInfo;
    FragInfoInit(sFragInfo);
    Lighting(sSurface, sFragInfo);

    return sFragInfo.cColorAcc;
}