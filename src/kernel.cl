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

inline bool solveQuadratic(float a, float b, float c,
    float *x0, float *x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) *x0 = *x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1) {
        float tmp = *x0;
        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

bool
hit_sphere(float3 center, float radius2, float3 start, float3 dir, float *t){
    float t0, t1;
    float3 L = start - center;
    float a = dot(dir, dir);
    float b = 2 * dot(dir, L);
    float c = dot(L, L) - radius2;
    //if (c <= 0) return false;
    if (!solveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }
    if (t0 > t1) {
        float tmp = t0;
        t0 = t1;
        t1 = tmp;
    }
    if (t0 < 0) {
        t0 = t1;
        if (t0 < 0) {
            return false;
        }
    }
    *t = t0;
    return true;
}

float3
trace_ray(float3 start, float3 dir) {
    float3 center = float3(0,-2,15);
    float dist;
    if (hit_sphere(center, 25.0f, start, dir, &dist)) {
        float3 pos = start + dist * dir;
        float3 normal = normalize(pos - center);
        if (mod(atan2(normal.x, normal.y) + 0.005, 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.x, normal.z)+0.005, 2*M_PI/6.0) < 0.01f) {
            return float3(1,0,0);
        } else if (mod(atan2(normal.y, normal.x) + 0.005, 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.y, normal.z) + 0.005, 2*M_PI/6.0) < 0.01f) {
            return float3(0,1,0);
        } else if (mod(atan2(normal.z, normal.x) + 0.005, 2*M_PI/6.0) < 0.01f ||
            mod(atan2(normal.z, normal.y) + 0.005, 2*M_PI/6.0) < 0.01f) {
            return float3(0,0,1);
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
