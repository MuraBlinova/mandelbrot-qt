#version 330 core

in vec2 fragCoord;
out vec4 out_col;

uniform float u_centerX;
uniform float u_centerY;
uniform float u_zoom;
uniform int u_maxIterations;
uniform vec2 u_resolution;

vec3 palette(float t) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 0.7, 0.4);
    vec3 d = vec3(0.00, 0.15, 0.20);
    return a + b * cos(6.28318 * (c * t + d));
}

void main() {
    const float threshold = 256.0;
    const float threshold2 = threshold * threshold;
    
    vec2 uv = fragCoord;
    float aspect = u_resolution.x / u_resolution.y;
    
    float sizeX = 2.0 / u_zoom;
    float sizeY = sizeX / aspect;
    
    float x0 = u_centerX + uv.x * sizeX;
    float y0 = u_centerY + uv.y * sizeY;
    
    float x = x0;
    float y = y0;
    
    int iter = 0;
    for (iter = 0; iter < u_maxIterations; iter++) {
        float xPrev = x;
        x = x * x - y * y + x0;
        y = 2.0 * xPrev * y + y0;
        
        if ((x * x + y * y) > threshold2) {
            break;
        }
    }
    
    float result = float(iter);
    
    if (iter != u_maxIterations) {
        result = result - log(log(sqrt(x * x + y * y)) / log(threshold)) / log(2.0);
    }
    
    result = result / float(u_maxIterations);
    
    vec3 color = palette(result);
    
    if (iter == u_maxIterations) {
        color = vec3(0.0);
    }
    
    out_col = vec4(color, 1.0);
}
