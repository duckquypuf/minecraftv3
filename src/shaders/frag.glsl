#version 410 core

in vec3  normal;
in float textureID;
in vec2  texCoord;
flat in int flags;
in vec2  colourmapUV;

out vec4 FragColour;

uniform sampler2D atlasTex;
uniform sampler2D grassmapTex;   // slot 1 — always bound
uniform sampler2D colourmapTex;  // slot 2 — always bound
uniform vec3 sunDir;

const int FLAG_LIQUID    = 1 << 0;
const int FLAG_GRASSMAP  = 1 << 1;
const int FLAG_COLOURMAP = 1 << 2;
const float atlasSize    = 16.0;

in float overlayTextureID;

void main() {
    // Base texture
    int id = int(textureID);
    vec2 finalUV = (vec2(texCoord.x, 1.0 - texCoord.y)
                 + vec2(float(id % int(atlasSize)), float(id / int(atlasSize))))
                 / atlasSize;

    vec4 texColor = texture(atlasTex, finalUV);
    if (texColor.a < 0.1) discard;

    // Overlay — sampled and tinted by grassmap before blending onto base
    int oid = int(overlayTextureID);
    if (oid >= 0) {
        vec2 overlayUV = (vec2(texCoord.x, 1.0 - texCoord.y)
                       + vec2(float(oid % int(atlasSize)), float(oid / int(atlasSize))))
                       / atlasSize;

        vec4 overlay = texture(atlasTex, overlayUV);

        // Tint the overlay by the grassmap colour before compositing
        if ((flags & FLAG_GRASSMAP) != 0)
            overlay *= texture(grassmapTex, colourmapUV);
        else if ((flags & FLAG_COLOURMAP) != 0)
            overlay *= texture(colourmapTex, colourmapUV);

        // Alpha composite overlay on top of base
        texColor = mix(texColor, overlay, overlay.a);
    } else {
        // No overlay — apply tint directly to base (e.g. grass top, leaves)
        if ((flags & FLAG_GRASSMAP) != 0)
            texColor *= texture(grassmapTex, colourmapUV);
        else if ((flags & FLAG_COLOURMAP) != 0)
            texColor *= texture(colourmapTex, colourmapUV);
    }

    // sunDir.y is positive at day, negative at night
    // 1. Create a snapped Day/Night factor
    // This goes from 0.0 to 1.0 very quickly as the sun rises
    float dayFactor = smoothstep(-0.15, 0.1, sunDir.y);

    // 2. Define Ambient levels
    // Day ambient is high (0.7) so blocks stay very bright
    float ambient = mix(0.1, 0.7, dayFactor);

    // 3. Minecraft-style Directional Shading
    // Even without the sun, blocks in voxel engines look better if 
    // the top is brightest and the bottom is darkest.
    float faceShading = 0.8; // Default for sides
    if (normal.y > 0.5)  faceShading = 1.0; // Top face
    if (normal.y < -0.5) faceShading = 0.5; // Bottom face
    if (abs(normal.z) > 0.5) faceShading = 0.9; // Front/Back variation

    // 4. Direct Sun Light
    // We add a bit of direct sunlight on top of the ambient
    float directLight = max(dot(normalize(normal), normalize(sunDir)), 0.0);
    
    // Combine everything
    // During day: (0.7 * faceShading) + (direct * 0.3) -> Max 1.0
    // During night: (0.1 * faceShading) + (0.0) -> Max 0.1
    float brightness = (ambient * faceShading) + (directLight * 0.3 * dayFactor);

    // Clamp to ensure we don't over-brighten beyond white
    brightness = clamp(brightness, 0.0, 1.0);

    float alpha = ((flags & FLAG_LIQUID) != 0) ? 0.8 : 1.0;
    FragColour = vec4(texColor.rgb * brightness, alpha);
}