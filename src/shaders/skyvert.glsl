#version 410 core

layout (location = 0) in vec2 aPos; // fullscreen quad, -1 to 1

out vec2 texCoord;
out vec3 rayDir;

uniform mat4 invProjection;
uniform mat4 invView; // rotation only — no translation

void main() {
    gl_Position = vec4(aPos, 0.999, 1.0); // 0.999 puts it just in front of far plane
    texCoord = aPos * 0.5 + 0.5;

    // Reconstruct view ray from clip space position
    vec4 clipPos = vec4(aPos, -1.0, 1.0);
    vec4 viewPos = invProjection * clipPos;
    viewPos = vec4(viewPos.xy, -1.0, 0.0); // direction, not position
    rayDir = (invView * viewPos).xyz;
}