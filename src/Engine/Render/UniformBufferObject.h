#pragma once
#include <glm/glm.hpp>

struct LightInfo {
    alignas(16) glm::vec4 light_pos = { glm::vec4(0.f) };
    alignas(16) glm::vec4 light_col = { glm::vec4(0.f) };
};

struct VertexUniformBufferObject {
    alignas(16) glm::mat4 view_proj;
};

struct FragmentUniformBufferObject {
    alignas(16) LightInfo lights[32] = { LightInfo() };
    float shadow_near = 0.01f;
    float shadow_far = 20.f;
    /** 0 = textured + lighting, 1 = world-space normal visualization (RGB) */
    float render_mode = 0.f;
    float _pad_render_mode = 0.f;
};

/** Fragment shader only; per-face light_view_proj is push constants (see shadow_vert). */
struct ShadowUBOData {
    alignas(16) glm::vec4 light_pos;
    alignas(8) glm::vec2 near_far; // x = near_plane, y = far_plane (std140 padding follows)
    glm::vec2 _pad_to_32;
};
