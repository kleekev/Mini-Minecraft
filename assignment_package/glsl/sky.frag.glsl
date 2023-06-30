#version 150


out vec4 color;

uniform vec3 u_CameraPos;

uniform mat4 u_ViewProj;

uniform ivec2 u_Dimensions;

uniform int u_Time;

const float PI = 3.14159265359;


// interpret uv coordinates from sphere coordinates by translating to polar coordinates
vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if (phi < 0) {
        phi += 2.0 * PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / (2.0 * PI), 1 - theta / PI);
}


// green Namek sky palette
const vec3 namekSkyPalette[4] = vec3[](vec3(172, 223, 135) / 255.0,
                                       vec3(118, 186, 27) / 255.0,
                                       vec3(76, 154, 42) / 255.0,
                                       vec3(30, 86, 49) / 255.0);

const vec3 sunColor = vec3(191, 227, 192) / 255.0;

vec3 uvToSkyColor(vec2 uv) {
    if (uv.y < 0.50) {
        return namekSkyPalette[0];
    }
    else if (uv.y < 0.60) {
        return mix(namekSkyPalette[0], namekSkyPalette[1], (uv.y - 0.50) / 0.10);
    }
    else if (uv.y < 0.70) {
        return mix(namekSkyPalette[1], namekSkyPalette[2], (uv.y - 0.60) / 0.10);
    }
    return mix(namekSkyPalette[2], namekSkyPalette[3], (uv.y - 0.70) / 0.40);
}

vec3 random3( vec3 p ) {
    return fract(
                sin(
                     vec3(
                         dot(p, vec3(127.1,311.7, 234.1)),
                         dot(p, vec3(269.5,183.3, 102.3)),
                         dot(p, vec3(456.5,234.3, 290.3))
                        )
                    )
                * 43758.5453);
}

float WorleyNoise3D(vec3 point) {
    // Tile the space
    vec3 pointInt = floor(point);
    vec3 pointFract = fract(point);
    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for (int z = -1; z <= 1; z++) {
        for(int y = -1; y <= 1; y++) {
            for(int x = -1; x <= 1; x++) {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.020 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float worleyFBM3D(vec3 point) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(point * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

void main()
{
    // project the pixel back into world space at our far clip plane
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 normalized device coordinates
    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // times far clip plane value
    p = u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_CameraPos);

    // get the uv coordinates for our ray
    vec2 uv = sphereToUV(rayDir);


    // offset sky color uv using fbm + worley
    float noise = worleyFBM3D(rayDir);

    vec3 skyColor = uvToSkyColor(uv + noise * 0.1);

    // Add a sun in the sky
    vec3 sunDir = normalize(vec3(0, 0.5, 1.0));

    float sunSize = 30;
    float sunVisibleAngle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    if (sunVisibleAngle < sunSize) {
        if (sunVisibleAngle < 5.0) {
            skyColor = sunColor;
        }
        else {
            skyColor = mix(sunColor, skyColor, (sunVisibleAngle - 5.0) / 25.0);
        }
    }


    color = vec4(skyColor, 1.0);
}
