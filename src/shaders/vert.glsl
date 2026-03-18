#version 410 core

layout (location = 0) in ivec3 aPos;
layout (location = 1) in ivec3 aNormal;
layout (location = 2) in int   aTextureID;
layout (location = 3) in int   aColourmapX;
layout (location = 4) in int   aColourmapY;
layout (location = 5) in int   aFlags;

out vec3  normal;
out float textureID;
out vec2  texCoord;
flat out int flags;
out vec2  colourmapUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const vec2 uvs[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

void main() {
    gl_Position  = projection * view * model * vec4(vec3(aPos), 1.0);
    normal       = vec3(aNormal);
    textureID    = float(aTextureID);
    texCoord     = uvs[gl_VertexID % 6];
    flags        = aFlags;
    colourmapUV  = vec2(aColourmapX, aColourmapY) / 256.0;
}