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

    float brightness = max(dot(normalize(normal), normalize(vec3(0.4, 1.0, 0.2))), 0.0) * 0.6 + 0.4;
    float alpha = ((flags & FLAG_LIQUID) != 0) ? 0.8 : 1.0;
    FragColour = vec4(texColor.rgb * brightness, alpha);
}