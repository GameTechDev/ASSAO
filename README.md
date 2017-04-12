# Adaptive Screen Space Ambient Occlusion

Demo solution is in /ASSAO.sln and requires Visual Studio 2015 to build

Pre-built demo binary is in /Projects/ASSAO/ASSAO.exe

All files required to port the effect to your codebase are in https://github.com/GameTechDev/ASSAO/tree/master/Projects/ASSAO/ASSAO (ASSAO.h, ASSAODX11.cpp and ASSAO.hlsl) and the complete usage example is in https://github.com/GameTechDev/ASSAO/blob/master/Projects/ASSAO/ASSAOWrapperDX11.cpp
If you want to embed the shader source into the compiled binary, check out the 'USE_EMBEDDED_SHADER' in the ASSAOWrapperDX11 line 35 - in that case, you would only need to include ASSAO.h, ASSAOdX11.cpp and ASSAO_shader.c (generated when you build sample Release) to your project.

For details please go to: https://software.intel.com/en-us/articles/adaptive-screen-space-ambient-occlusion

For any questions please feel free to contact the author at filip.strugar@intel.com