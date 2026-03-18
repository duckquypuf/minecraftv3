#version 410 core

in vec2 texCoord;
in vec3 rayDir;

out vec4 FragColour;

uniform sampler2D sunTex;
uniform float time; // -1.0 (midnight) to 1.0 (noon)

const float SUN_SIZE = 0.03; // angular radius, tweak to taste

void main() {
    // Reconstruct sunDir from time — arc across the sky on the X/Y plane
    float angle = time * 3.14159265; // -pi to pi
    vec3 sunDir = normalize(vec3(0.0, sin(angle), cos(angle)));

    vec3 ray = normalize(rayDir);

    // Sky colour — interpolate horizon to zenith based on Y
    vec3 zenith  = vec3(0.10, 0.40, 0.85);
    vec3 horizon = vec3(0.55, 0.75, 0.95);
    vec3 night   = vec3(0.01, 0.01, 0.05);

    float horizonBlend = pow(max(ray.y, 0.0), 0.4);
    vec3 daySky   = mix(horizon, zenith, horizonBlend);
    vec3 skyColour = mix(night, daySky, clamp(time, 0.0, 1.0));

    // Sun disc — angle between ray and sunDir
    float cosAngle = dot(ray, sunDir);
    float sunEdge  = 1.0 - SUN_SIZE;

    if (cosAngle > sunEdge && sunDir.y > -0.05) {
        // UV within the sun disc for texture sampling
        vec3 up        = vec3(0.0, 1.0, 0.0);
        vec3 sunRight  = normalize(cross(sunDir, up));
        vec3 sunUp     = normalize(cross(sunRight, sunDir));

        vec3 offset = ray - sunDir;
        float u = dot(offset, sunRight) / SUN_SIZE * 0.5 + 0.5;
        float v = dot(offset, sunUp)    / SUN_SIZE * 0.5 + 0.5;

        vec4 sunSample = texture(sunTex, vec2(u, v));

        // Glow falloff at disc edge
        float discBlend = smoothstep(sunEdge, 1.0, cosAngle);
        FragColour = vec4(mix(skyColour, sunSample.rgb, sunSample.a * discBlend), 1.0);
    } else {
        FragColour = vec4(skyColour, 1.0);
    }
}