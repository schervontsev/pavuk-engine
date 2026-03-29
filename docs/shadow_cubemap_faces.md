# Point-light shadow cube: which layer is which direction

## Vulkan / OpenGL cubemap face order

Array layer `i` in a **cube-compatible** depth image has a fixed meaning (same as `samplerCube` / `samplerCubeShadow`):

| Layer `i` | World direction (from light toward scene) | Notes |
|-----------|---------------------------------------------|--------|
| **0** | **+X** | Right in world space |
| **1** | **−X** | Left |
| **2** | **+Y** | Up |
| **3** | **−Y** | Down |
| **4** | **+Z** | Forward (engine convention; check your scene) |
| **5** | **−Z** | Back |

References: Vulkan spec (image view cubemap), OpenGL cubemap face order (identical).

## Fragment shader sampling

For a point light at `L` and shaded point `P`:

- `sampleDir = normalize(P - L)` is the direction **from light toward** `P` (same as the ray stored in that cubemap texel).

That vector must select the **correct** face: e.g. if `sampleDir ≈ (0, 1, 0)`, the GPU uses **layer 2** (+Y). So **layer 2** must have been rendered with a view whose **forward** is **+Y** — see `Renderer::UpdateShadowCubeFace`.

## Runtime: keys 1–6

Digits **1–6** toggle whether **layers 0–5** are rendered each frame (`shadowCubeFaceMask`). Disabled layers are cleared to far depth so sampling stays “lit” on those directions.

## If shadows look wrong on one axis only

In `shader.frag`, `FLIP_SHADOW_*` flip the **sample** direction component before `texture(samplerCubeShadow, …)`. Use **at most one** at a time for debugging axis flips; with correct `lookAt` per layer, all should be **0**.

## Engine convention

`glm::lookAt(lightPos, lightPos + direction, up)` with `proj[1][1] *= -1` for Vulkan NDC. Up-vectors follow the common capture layout (e.g. LearnOpenGL cubemap depth).
