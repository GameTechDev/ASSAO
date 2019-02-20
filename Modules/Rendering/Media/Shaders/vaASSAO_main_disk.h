#ifndef __VA_INTELSSAO_MAIN_DISK_H__
#define __VA_INTELSSAO_MAIN_DISK_H__

#define INTELSSAO_MAIN_DISK_SAMPLE_COUNT (32)

// NOTE: This needs to be kept in sync with the definition in ASSAO.hlsl.
#ifndef SSAO_PARTIAL_PRECISION
#define SSAO_PARTIAL_PRECISION 1
#endif  // SSAO_PARTIAL_PRECISION

#if SSAO_PARTIAL_PRECISION
#define lpint       min16int
#define lpuint      min16uint
#define lpuint2     min16uint2
#define lpuint3     min16uint3
#define lpfloat     min16float
#define lpfloat2    min16float2
#define lpfloat3    min16float3
#define lpfloat4    min16float4
#define lpfloat2x2  min16float2x2
#else
#define lpint       int
#define lpuint      uint
#define lpuint2     uint2
#define lpuint3     uint3
#define lpfloat     float
#define lpfloat2    float2
#define lpfloat3    float3
#define lpfloat4    float4
#define lpfloat2x2  float2x2
#endif

static const lpfloat4 g_samplePatternMain[INTELSSAO_MAIN_DISK_SAMPLE_COUNT] =
{
     0.78488064,  0.56661671,  1.500000, -0.126083,
     0.26022232, -0.29575172,  1.500000, -1.064030,
     0.10459357,  0.08372527,  1.110000, -2.730563,
    -0.68286800,  0.04963045,  1.090000, -0.498827,
    -0.13570161, -0.64190155,  1.250000, -0.532765,
    -0.26193795, -0.08205118,  0.670000, -1.783245,
    -0.61177456,  0.66664219,  0.710000, -0.044234,
     0.43675563,  0.25119025,  0.610000, -1.167283,
     0.07884444,  0.86618668,  0.640000, -0.459002,
    -0.12790935, -0.29869005,  0.600000, -1.729424,
    -0.04031125,  0.02413622,  0.600000, -4.792042,
     0.16201244, -0.52851415,  0.790000, -1.067055,
    -0.70991218,  0.47301072,  0.640000, -0.335236,
     0.03277707, -0.22349690,  0.600000, -1.982384,
     0.68921727,  0.36800742,  0.630000, -0.266718,
     0.29251814,  0.37775412,  0.610000, -1.422520,
    -0.12224089,  0.96582592,  0.600000, -0.426142,
     0.11071457, -0.16131058,  0.600000, -2.165947,
     0.46562141, -0.59747696,  0.600000, -0.189760,
    -0.51548797,  0.11804193,  0.600000, -1.246800,
     0.89141309, -0.42090443,  0.600000,  0.028192,
    -0.32402530, -0.01591529,  0.600000, -1.543018,
     0.60771245,  0.41635221,  0.600000, -0.605411,
     0.02379565, -0.08239821,  0.600000, -3.809046,
     0.48951152, -0.23657045,  0.600000, -1.189011,
    -0.17611565, -0.81696892,  0.600000, -0.513724,
    -0.33930185, -0.20732205,  0.600000, -1.698047,
    -0.91974425,  0.05403209,  0.600000,  0.062246,
    -0.15064627, -0.14949332,  0.600000, -1.896062,
     0.53180975, -0.35210401,  0.600000, -0.758838,
     0.41487166,  0.81442589,  0.600000, -0.505648,
    -0.24106961, -0.32721516,  0.600000, -1.665244
};

#endif //__VA_INTELSSAO_MAIN_DISK_H__
