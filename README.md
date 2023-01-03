# DISCONTINUATION OF PROJECT #
This project will no longer be maintained by Intel.
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.
Intel no longer accepts patches to this project.
# Adaptive Screen Space Ambient Occlusion

---
**Note:** this codebase is no longer being updated and has been superseded by https://github.com/GameTechDev/XeGTAO which is, in most use cases, better and faster screen space ambient occlusion implementation.

---


Demo solution is in /ASSAO.sln and requires Visual Studio 2015 to build

Pre-built demo binary is in /Projects/ASSAO/ASSAO.exe

All files required to port/implement the effect are in https://github.com/GameTechDev/ASSAO/tree/master/Projects/ASSAO/ASSAO (ASSAO.h, ASSAODX11.cpp and ASSAO.hlsl) and the complete usage example is in https://github.com/GameTechDev/ASSAO/blob/master/Projects/ASSAO/ASSAOWrapperDX11.cpp

To embed the shader source into the compiled binary, the 'USE_EMBEDDED_SHADER' in the ASSAOWrapperDX11 line 35 approach can be used. In that case, ASSAO.h, ASSAOdX11.cpp and ASSAO_shader.c (generated when you build sample Release) are required for implementing the effect.

For details please go to: https://software.intel.com/en-us/articles/adaptive-screen-space-ambient-occlusion

For any questions please feel free to contact the author at filip.strugar@intel.com
