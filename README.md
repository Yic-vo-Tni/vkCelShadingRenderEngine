# Hakuro Fabric

Hakuro Fabric is a Vulkan-based rendering engine prototype focused on exploring dynamic,
decoupled, and editable rendering system design.

Rather than chasing higher performance or better visual fidelity,
this project emphasizes smoother workflows, freer composition of rendering pipelines,
more flexible system-level organization, and explicit expression of rendering behavior

## Design Goals and Exploration Scope
Hakuro Fabric initially began as a renderer for MMD content.
During the process of learning and experimenting with Vulkan,
the project gradually evolved beyond the original use case
into a broader exploration of rendering system design.

Rather than pursuing a single fixed end goal, Hakuro Fabric
serves as an exploration space for investigating different
approaches to rendering workflows and system organization.
Some of these explorations may be refined and carried forward,
while others may be paused or discarded as their trade-offs
and limitations become clearer.


### Exploration Areas

Hakuro Fabric explores multiple aspects of rendering system design,
with a focus on how modern Vulkan capabilities can enable more
dynamic, decoupled, and adaptive rendering workflows.

These exploration areas are closely related and often overlap.
They are not independent features, but different layers of the
same design space.

#### 1. Rendering Workflow Expression (DER)

Exploring **Dynamic Editable Rendering (DER)**, where render nodes
are defined by behavior rather than static descriptions.

Each render node is constructed through explicit build stages
(target creation, pipeline creation, descriptor binding, and
command dispatch), allowing rendering workflows to be composed,
replaced, or reconfigured at runtime.

#### 2. System Graph and Global Scheduling

Exploring a system-level graph that extends beyond rendering,
where rendering workflows are treated as part of a larger system
graph alongside resource management, scene updates, and tool systems.

DER is considered a sub-layer within this broader system graph,
rather than an isolated rendering concept.

#### 3. Execution and Parallelism Model

Exploring unified and decoupled execution models, where window handling,
fast logic, slow logic, rendering, and submission are treated as
independently schedulable units.

This includes investigating global scheduling strategies that prevent
slow CPU-side workloads from stalling unrelated rendering progress,
enabling full decoupling between system components.

#### 4. Adaptive and AI-Assisted Workflow Decisions

Exploring how higher-level decision systems, including AI-driven
approaches, could participate in selecting, modifying, or replacing
rendering workflows at runtime based on system state, performance
characteristics, or execution feedback.

#### 5. Modern Vulkan Capabilities and Pipeline Alternatives

Exploring how newer Vulkan features and extensions—such as dynamic
rendering, descriptor buffers, and mesh shaders—can simplify or
replace traditional rendering pipelines and reduce fixed upfront
structure in rendering workflows.

### Current Capabilities

This section describes the current, observable state of the codebase,
rather than a complete or finalized feature set.

Hakuro Fabric was originally developed with MMD rendering as a concrete starting point.
As the project evolved, the renderer grew to support more general model workflows as well.
The current codebase supports model loading via Assimp, basic skeletal animation,
and VMD playback for MMD content.

The renderer already adopts a subset of modern Vulkan features, including dynamic rendering
(with local read), timeline semaphores, pipeline libraries, and ray tracing extensions.
A minimal MVP of Dynamic Editable Rendering (DER) is implemented, allowing simple render
target workflows to be replaced at runtime through dynamically loaded modules.
Command buffers are collected and built in parallel at a global level, although the
current framework is still incomplete and under active iteration.

At the system level, the engine is organized around four major execution paths:
window handling, fast logic, slow logic, and rendering.
This structure allows animation updates and resource processing to run without blocking
rendering or input handling. Basic shader hot-reloading and a lightweight temporary editor
are also present to support rapid iteration during development.

Visual quality is not a primary focus of the project at this stage.
Rendering output is intentionally minimal, consisting of basic rasterization,
ray-traced directional light shadows, and a hacked volumetric cloud implementation
adapted from Shadertoy experiments.

### Screenshots
The following screenshots illustrate the current rendering output,
which primarily serves as a validation tool rather than a visual showcase.

<img src="screenShot/01.png" width="340"/> <img src="screenShot/02.png" width="340"/>

## How to Build
This project is mainly for personal learning, so it always targets the **latest toolchains and hardware**.  
Please make sure your environment is **equal to or newer than mine**, otherwise it may fail to build.

### Environment (2025.11.9)
- Vulkan SDK: 1.4.312  *(1.3 or newer is required)*
- Vulkan Runtime (driver): 1.4.312
- CMake: 3.29  *(3.20+ should work)*
- Compiler: Clang 21.1.1  
  *(MinGW Clang 18+ work, MSVC may hit CRT issues)*
- GPU: NVIDIA GeForce RTX 3080 Ti *(RTX 20 series may work, AMD not tested)*
- OS: Windows 11

### Dependencies (2026.1.10)
Currently you need to download or build the following libraries manually:  
`mimalloc, entt, oneAPI TBB, glm, miniaudio, nlohmann, spdlog, stb, webview, imgui(docking), imguizmo, imnode, saba, bullet`.\
Place library `third/xxx`,Place built `.lib` in `third/lib` and `.dll` in `third/dll`.
Notes:
- **Bullet**: follow Saba's requirement.
- **Saba**: needs cleanup (remove internal spdlog to avoid conflicts).
- After all dependencies are resolved, you can open the project directly in CLion and build.
- Plan: Gradually migrate dependencies to **git submodules**.  
  Progress: assimp, glfw :(

> Since the main focus of the author’s study is Vulkan, many features are implemented by first selecting and integrating existing libraries.
> In some cases, even if only a small portion of functionality is needed, a relatively heavy dependency may be introduced.
> This is largely due to the author’s current experience level—for example, Boost was used in this way.

> At the moment, **oneAPI TBB may not be buildable** in the current environment.
> The current plan is to replace it with a combination of four alternative libraries.
> This issue is mainly related to the author’s preference for a **MinGW ABI + Clang** toolchain, which causes build incompatibilities with TBB.
> The replacement will be done gradually, with dependencies migrated to **git submodules** over time.

> For **MSVC**, the main issue is related to **CRT compatibility in Saba**.
> The current plan is to fork the Saba repository and refactor it internally,
> with the goal of addressing and improving these CRT-related problems.

### Validation Report (2025.9.4)
- **Errors:** none
- **Warnings:** none *(except minimized window case, expected behavior)*
- **Best Practices:** none

📌 Note: The Vulkan-related parts of the engine are carefully validated and guaranteed to be clean.  
Other subsystems (e.g. animation, editor logic, resource loading) may contain bugs or unfinished logic.  
These are secondary to my learning goal, so I may only fix them when I find a good solution.

## Implementation Status

To avoid overloading this README with engineering details,
the concrete implementation status of Hakuro Fabric is documented separately.

📄 **Full status document:** [Implementation Status & Feature Matrix](EngineStatus.md)