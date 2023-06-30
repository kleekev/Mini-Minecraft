#version 150

in vec2 fs_UV;

out vec4 color;

uniform sampler2D u_Texture;

uniform int u_Time;

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

float WorleyNoise(vec2 uv) {
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.025 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float random1( vec2 p ) {
    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);
}

float mySmootherStep(float a, float b, float t) {
    t = t*t*t*(t*(t*6.0 - 15.0) + 10.0);
    return mix(a, b, t);
}

vec2 rotate(vec2 p, float deg) {
    float rad = deg * 3.14159 / 180.0;
    return vec2(cos(rad) * p.x - sin(rad) * p.y,
                sin(rad) * p.x + cos(rad) * p.y);
}

float bilerpNoise(vec2 uv) {
    vec2 uvFract = fract(uv);
    float ll = random1(floor(uv));
    float lr = random1(floor(uv) + vec2(1,0));
    float ul = random1(floor(uv) + vec2(0,1));
    float ur = random1(floor(uv) + vec2(1,1));

    float lerpXL = mySmootherStep(ll, lr, uvFract.x);
    float lerpXU = mySmootherStep(ul, ur, uvFract.x);

    return mySmootherStep(lerpXL, lerpXU, uvFract.y);
}

float fbm(vec2 uv) {
    float amp = 0.5;
    float freq = 8.0;
    float sum = 0.0;
    for(int i = 0; i < 6; i++) {
        sum += bilerpNoise(uv * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

// Returns a vector in the range [-1, 1]
// based on the output of FBM
vec2 NoiseVectorFBM(vec2 uv) {
    float x = fbm(uv) * 2.0 - 1.0;
    float y = fbm(rotate(uv, 60.0)) * 2.0 - 1.0;
    return vec2(x, y);
}


const float WARP_FREQUENCY = 2.0;
const float WARP_MAGNITUDE = 1.0;
const float NOISE_FREQUENCY = 4.0;

void main()
{
    // add red tint
    vec4 baseColor = texture(u_Texture, fs_UV);
    baseColor.r += (1 - baseColor.r) * 0.8;
    // diminish other colors
    baseColor.g *= 0.65;
    baseColor.b *= 0.3;

    // add light distortion
    vec2 warp = NoiseVectorFBM(fs_UV * WARP_FREQUENCY) * WARP_MAGNITUDE;
    float noise = WorleyNoise(NOISE_FREQUENCY * fs_UV + warp);

    baseColor.r += noise * 0.5;
    baseColor.g += noise * 0.325;
    color = baseColor;
}
