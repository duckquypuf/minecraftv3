#version 410 core

in vec3 normal;
in float textureID;
in vec2 texCoord; // Now received from Vertex Shader

out vec4 FragColour;

uniform sampler2D atlasTex;

const float atlasSize = 16.0; 

void main() {
    int id = int(textureID);
    
    float uOffset = float(id % int(atlasSize));
    float vOffset = float(id / int(atlasSize));

    // Note: If your textures look upside down, change to (1.0 - texCoord.y + vOffset)
    vec2 finalUV = (vec2(texCoord.x, 1.0 - texCoord.y) + vec2(uOffset, vOffset)) / atlasSize;

    vec4 texColor = texture(atlasTex, finalUV);

    if(texColor.a < 0.1) discard;

    vec3 sunDir = normalize(vec3(0.4, 1.0, 0.2));
    float brightness = max(dot(normalize(normal), sunDir), 0.0);
    brightness = brightness * 0.6 + 0.4; 

    FragColour = vec4(texColor.rgb * brightness, 1.0);
}