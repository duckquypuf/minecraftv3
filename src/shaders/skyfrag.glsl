#version 410 core

in vec2 texCoord;
in vec3 rayDir;

out vec4 FragColour;

uniform sampler2D sunTex;
uniform float time; 

const float SUN_SIZE = 0.15;

void main() {
    // Reconstruct sunDir
    float angle = time * 1.5708; 
    float tilt = 0.4014; 
    vec3 sunDir = normalize(vec3(
        sin(angle) * sin(tilt),
        sin(angle) * cos(tilt),
        cos(angle)
    ));

    vec3 ray = normalize(rayDir);

    // 1. Define Color States
    // Standard Day
    vec3 zenith  = vec3(0.15, 0.45, 0.95);
    vec3 horizon = vec3(0.50, 0.70, 1.00);
    
    // Sunset/Sunrise (Red/Orange/Purple)
    vec3 sunsetHorizon = vec3(1.0, 0.4, 0.1); 
    vec3 sunsetZenith  = vec3(0.4, 0.1, 0.5); 
    
    // Night
    vec3 night = vec3(0.02, 0.02, 0.08);

    // 2. Calculate Transition Factors
    // Day Amount (Snapped for Minecraft-style brightness)
    float dayAmount = smoothstep(-0.15, 0.05, sunDir.y);
    
    // Sunset Factor: Peaks at 1.0 when sunDir.y is 0.0 (horizon)
    // Tweak the 4.0 and 5.0 values to make the sunset last longer or shorter
    float sunsetFactor = clamp(1.0 - abs(sunDir.y) * 4.0, 0.0, 1.0);

    // 3. Blend the "Day" colors with the "Sunset" colors
    vec3 currentHorizon = mix(horizon, sunsetHorizon, sunsetFactor);
    vec3 currentZenith  = mix(zenith, sunsetZenith, sunsetFactor);

    // 4. Calculate Sky Gradients
    float horizonBlend = pow(max(ray.y, 0.0), 0.5);
    vec3 daySky = mix(currentHorizon, currentZenith, horizonBlend);
    
    // 5. Final Sky transition to night
    vec3 skyColour = mix(night, daySky, dayAmount);

    // 6. Sun Disc
    float cosAngle = dot(ray, sunDir);
    float sunVisibility = smoothstep(-0.05, 0.05, sunDir.y);
    
    // Sun logic (Texture sampling)
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 sunRight = normalize(cross(sunDir, up));
    vec3 sunUp = normalize(cross(sunRight, sunDir));
    vec3 offset = ray - sunDir;
    float u = dot(offset, sunRight) / SUN_SIZE * 0.5 + 0.5;
    float v = dot(offset, sunUp) / SUN_SIZE * 0.5 + 0.5;

    vec4 sunSample = texture(sunTex, vec2(u, v));
    
    // Tint the sun slightly orange during sunset
    vec3 sunTint = mix(vec3(1.0), vec3(1.0, 0.6, 0.2), sunsetFactor);
    vec3 sunFinalColor = sunSample.rgb * sunTint;

    float dist = length(offset);
    float sunMask = step(dist, SUN_SIZE) * sunVisibility;

    vec3 finalSky = mix(skyColour, sunFinalColor, sunSample.a * sunMask);
    
    FragColour = vec4(finalSky, 1.0);
}