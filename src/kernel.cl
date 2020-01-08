#define float3(x, y, z) (float3){x, y, z}
#define float4(x, y, z, w) (float4){x, y, z, w}

typedef float4 matrix[4];

float3
mul(const matrix M, float3 X) {
    return float3(
        dot(M[0].xyz, X) + M[0].w,
        dot(M[1].xyz, X) + M[1].w,
        dot(M[2].xyz, X) + M[2].w
        )/(dot(M[3].xyz, X) + M[3].w);
}

float
mod(float a, float b) {
    return fmod(fmod(a, b) + b, b);
}

bool
hit_sphere(float3 center, float radius, float3 start, float3 dir, float *t){
    float3 oc = start - center;
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, oc);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    *t = (-b - sqrt(discriminant)) / (2.0*a);
    return discriminant >= 0;
}

float3
trace_ray(float3 start, float3 dir) {
    float3 center = float3(0,-2,15);
    float dist;
    if (hit_sphere(center, 5.0f, start, dir, &dist)) {
        float3 pos = start + dist * dir;
        float3 normal = normalize(pos - center);
        if (mod(atan2(normal.x, normal.y), 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.x, normal.z), 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.y, normal.x), 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.y, normal.z), 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.z, normal.x), 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.z, normal.y), 2*M_PI/6.0) < 0.01f) {
            return 0;
        }
        return (normal + 1) / 2;
    }
    return (dir+1)/2;
}

__kernel void
render(__write_only image2d_t image,
       global float4 cameraMatrix[4]) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX    = get_global_size(0);
    const uint resY    = get_global_size(1);
    const float3 ncp = mul(cameraMatrix,
        float3(x_coord - (float)resX/2,y_coord - (float)resY / 2,
            -1));
    const float3 fcp = mul(cameraMatrix,
        float3(x_coord - (float)resX/2, y_coord - (float)resY / 2, 1));
    const float3 dir = normalize((fcp - ncp).xyz);
    write_imagef(image,
        (int2){x_coord, y_coord},
        (float4){
            trace_ray(ncp, dir),
            1.0
        }
    );
}
