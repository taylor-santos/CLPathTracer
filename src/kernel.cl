#define float3(x, y, z) (float3){x, y, z}
#define float4(x, y, z, w) (float4){x, y, z, w}

typedef float4 matrix[4];

typedef struct __attribute__ ((packed)) Object {
    float3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            float radius;
        } sphere;
    };
} Object;

typedef struct Hit {
    float dist;
    Object *obj
} Hit;

float3
mul(const matrix M, float3 X) {
    return float3(dot(M[0].xyz, X) + M[0].w,
        dot(M[1].xyz, X) + M[1].w,
        dot(M[2].xyz, X) + M[2].w) / (dot(M[3].xyz, X) + M[3].w);
}

float
mod(float a, float b) {
    return fmod(fmod(a, b) + b, b);
}

inline bool
solveQuadratic(float a, float b, float c, float *x0, float *x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) {
        return false;
    } else if (discr == 0) {
        *x0 = *x1 = -0.5 * b / a;
    } else {
        float q = (b > 0)
            ? -0.5 * (b + sqrt(discr))
            : -0.5 * (b - sqrt(discr));
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
hit_sphere(float3 center, float radius2, float3 start, float3 dir, float *t) {
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
trace_ray(float3 start,
    float3 dir,
    Object *objects,
    int objcount,
    int depth,
    float str,
    Object *ignore) {
    if (depth == 0) {
        return 0;
    }
    Hit minHit;
    int didHit = 0;
    for (int i = 0; i < objcount; i++) {
        if (objects + i == ignore) {
            continue;
        }
        Object obj = objects[i];
        float dist;
        switch (obj.type) {
            case OBJ_SPHERE:
                if (hit_sphere(obj.position,
                    obj.sphere.radius * obj.sphere.radius,
                    start,
                    dir,
                    &dist)) {
                    if (!didHit || dist < minHit.dist) {
                        didHit = 1;
                        minHit.dist = dist;
                        minHit.obj = &objects[i];
                    }
                }
        }
    }
    if (didHit) {
        float3 pos = start + minHit.dist * dir;
        float3 normal = normalize(pos - minHit.obj->position);
        dir = dir - 2 * dot(dir, normal) * normal;
        start = pos;
        return trace_ray(start,
            dir,
            objects,
            objcount,
            depth - 1,
            str * 1,
            minHit.obj);
    }

    return str * (dir + 1) / 2;
}

__kernel void
render(__write_only image2d_t image, global float4 cam[4]) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX = get_global_size(0);
    const uint resY = get_global_size(1);
    const float3 origin =
        float3(cam[0].z / cam[3].z, cam[1].z / cam[3].z, cam[2].z / cam[3].z);
    const float3 ncp = mul(cam,
        float3(x_coord - (float)resX / 2, y_coord - (float)resY / 2, -1));
    const float3 fcp = mul(cam,
        float3(x_coord - (float)resX / 2, y_coord - (float)resY / 2, 1));
    const float3 dir = normalize((fcp - ncp).xyz);
    Object spheres[5];
    spheres[0] = (Object){
        float3(5, -5, 15),
        OBJ_SPHERE, .sphere = {
            5
        }
    };
    spheres[1] = (Object){
        float3(-5, -5, 15),
        OBJ_SPHERE, .sphere = {
            5
        }
    };
    spheres[2] = (Object){
        float3(5, 5, 15),
        OBJ_SPHERE, .sphere = {
            5
        }
    };
    spheres[3] = (Object){
        float3(-5, 5, 15),
        OBJ_SPHERE, .sphere = {
            5
        }
    };
    spheres[4] = (Object){
        float3(0, 0, 15),
        OBJ_SPHERE, .sphere = {
            2
        }
    };
    write_imagef(image, (int2){
        x_coord,
        y_coord
    }, (float4)
    {
        trace_ray(origin, dir, spheres, 5, 5000, 1, NULL), 1.0
    }
    );
}
