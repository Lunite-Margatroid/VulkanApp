# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A Vulkan renderer built as a learning project. C++20, namespace `LT`, code comments are in Chinese. All sources are UTF-8 (`/utf-8` compile flag on Windows; `gbk_to_utf8.py` exists for converting legacy files).

Dependencies: Vulkan SDK, SDL3, glm, VulkanMemoryAllocator, OpenImageIO, and Slang (shaders are compiled to SPIR-V **at runtime**, not build time).

## Build & Run

CMake 3.15+, Visual Studio 17 2022 generator on Windows (multi-config). Each dependency's location is passed as `-DXXX_INSTALLED_DIR=...`; they default to `$ENV{VULKAN_SDK}` (OIIO defaults to `C:/Program Files/OpenImageIO` and is also tried via `find_package` first).

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 \
  -DOIIO_INSTALLED_DIR=<dir> -DVMA_INSTALLED_DIR=<dir> -DSDL3_INSTALLED_DIR=<dir>
cmake --build build --config Debug
```

- `GenSln_c.bat` holds the machine-specific configure line, but `.gitignore` ignores `*.bat`, so don't rely on it in the repo.
- Outputs land in `build/bin/<Config>/` and `build/lib/`. Post-build custom targets (`cmake/cpydll.cmake`, `cpyslang.cmake`, `cpyasset.cmake`) copy runtime DLLs, `vkRenderer/slang/` shaders, and `TestAsset/` next to the executables.
- **Run from the output directory**: `sandbox.exe` loads shaders from `./slang` and assets from `./TestAsset` relative to the CWD. VS debugger working dir is already set to `$(OutDir)` in `sandbox/CMakeLists.txt`.
- Debug builds enable `VK_LAYER_KHRONOS_validation` (see `sandbox/sandbox.cpp`).
- `vkRenderer/CMakeLists.txt` collects sources with `file(GLOB)` — re-run CMake after adding/removing files.
- There is no unit-test framework or ctest. `SlangTest` is a manual console test for Slang compilation/reflection.

## Architecture

### Modules (each is a CMake target)

| Target | Type | Role |
|---|---|---|
| `logger` | static lib | `LOG_*` printf macros, `RENDERER_ASSERT` (throws `LT::ExceptionAssert`) |
| `SlangCompiler` | static lib | `LT::SlangCompiler` singleton: runtime compilation of `.slang` modules to SPIR-V, reflection into `ShaderModuleInfo` |
| `vkRenderer` | static lib | The renderer. Only `core/` and `util/` are public include dirs; everything under `renderer/` is private |
| `sandbox` | executable | SDL3 window + `LT::Engine` demo app; owns the frame loop |
| `SlangTest` | executable | Manual Slang test program |

### Global state and the frame loop

- `vkContext` is a singleton owning all global Vulkan state: instance, physical device, device, queues, command pool, descriptor pool, and `SwapChain`. It exposes static accessors (`GetCmdQueue()`, `GetCmdBuffer(i)`, `GetDescriptorPool()`, `BeginSingleTimeCmdBuffer()`, ...). It has two init paths: create its own instance/surface from extensions + `HWND`, or adopt an externally created `vk::Instance`/`vk::SurfaceKHR` (the sandbox uses the latter; instance + surface are created with SDL3 there).
- `Engine` is the app-facing facade: cameras, resize, pause/resume, `DrawFrame()`. The actual event/swap loop lives in `sandbox/sandbox.cpp`, not in the library.
- Double buffering: 2 flight frames (`RENDERER_DEFAULT_FLIGHT_FRAME_NUM` in `vkContext.h`). Per-flight-frame resources: const buffers, descriptor sets, fences, semaphores. `FrameInfo` carries `imageIndex` / `frameIndex` / `indexInFlight`.
- The synchronization model is documented in `doc/syncLogic.drawio`.

### Pipeline abstraction (active WIP — the most recent commits)

- The older path, `renderer/pipeline/Pipeline.*`, built `VkPipeline` state directly. It now delegates to `GraphicPass` (`m_pGraphicPass`): `Pipeline::DrawFrame()` records/submits through it.
- New hierarchy under `renderer/pipeline/`:
  - `renderpass/RenderPass` — base class; static image-layout-transition helper.
  - `renderpass/GraphicPass : RenderPass` — shader modules compiled via Slang at init; descriptor-set layouts split by `BindingSpace` (`eVertexShader` / `eFragmentShader` / `eVertAndFragShader`, vector index = space); `RecordCommand(RecordCommandInfo)` / `Submit(GraphicSubmitInfo)` operate on per-flight-frame semaphores/fences.
  - `renderpass/ComputePass : RenderPass`
  - `stage/IRenderStage` + `RenderStageOpaqueForward` — stubs; intended structure is stages → passes → entities.
  - `entity/IEntity` + `EntityRender`/`EntityRenderMesh`, `material/IMaterial` + `MaterialMainTexture`, `mesh/IMesh` + `MeshStatic`.
- `RenderPassFlag` (uint64 bitfield in `renderpass/RenderPass.hpp`) is the central design: bits encode vertex channels (0–10), back-face cull/front-face (11–12), primitive topology (13–16), polygon mode (17–18), line width (19–22), blend (23). The flag drives **both** Vulkan pipeline state and Slang preprocessor macros (`GenGraphicPPMacroDesc`), so shader variants are compiled to match the pass config.
- GPU resources are referenced by ID handles through manager singletons: `BufferManager` (`BufferID`; `ConstBuffer`/`VertexBuffer`/`IndexBuffer`/`StagingBuffer`), `ImageManager` (`ImageID`), `MeshManager`, `MaterialManager`, `SamplerManager`. Comments state resource ownership (e.g. "该实例由当前实例管理" = owned by this instance).

### Conventions

- `core/EngineCommon.h` must be included **before** glm: it defines `GLM_FORCE_DEPTH_ZERO_TO_ONE` and `GLM_FORCE_LEFT_HANDED`, plus shared color/axis constants.
- The library uses non-RAII `vulkan.hpp` handles; `vulkan_raii` appears only in `sandbox.cpp` for its own instance/surface.
- `README.md` is outdated (says SDL2; the code uses SDL3).
