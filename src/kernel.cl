#define vec_type float

#define CONCAT2(a, b) a ## b
#define CONCAT(a, b) CONCAT2(a, b)

#define vec3 CONCAT(vec_type, 3)
#define vec4 CONCAT(vec_type, 4)
#define color float3
#define color4 float4

#define new_vec3(x, y, z) (vec3){x, y, z}
#define new_vec4(x, y, z, w) (vec4){x, y, z, w}
#define new_color(x, y, z) (float3){x, y, z}
#define convert_color(vec) convert_float3(vec)

#define EPS 0.000001

typedef vec4 matrix[4];

typedef struct __attribute__ ((packed)) Object {
    vec3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            vec_type radius;
        } sphere;
    };
} Object;

typedef struct Hit {
    float dist;
    __global Object *obj;
} Hit;

vec3
mul(const matrix M, vec3 X) {
    return new_vec3(dot(M[0].xyz, X) + M[0].w,
        dot(M[1].xyz, X) + M[1].w,
        dot(M[2].xyz, X) + M[2].w) / (dot(M[3].xyz, X) + M[3].w);
}

vec_type
mod(vec_type a, vec_type b) {
    return fmod(fmod(a, b) + b, b);
}

bool
solveQuadratic(vec_type a, vec_type b, vec_type c, vec_type *x0, vec_type *x1) {
    vec_type discr = b * b - 4 * a * c;
    if (discr < 0) {
        return false;
    } else if (discr == 0) {
        *x0 = *x1 = -0.5 * b / a;
    } else {
        vec_type q = (b > 0)
            ? -0.5 * (b + sqrt(discr))
            : -0.5 * (b - sqrt(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1) {
        vec_type tmp = *x0;
        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

bool
hit_sphere(vec3 center, vec_type radius2, vec3 start, vec3 dir, vec_type *t) {
    vec_type t0, t1;
    vec3 L = start - center;
    vec_type a = dot(dir, dir);
    vec_type b = 2 * dot(dir, L);
    vec_type c = dot(L, L) - radius2;
    if (c <= 0) return false;
    if (!solveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }
    if (t0 > t1) {
        vec_type tmp = t0;
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

bool
hit_triangle(vec3 v0,
    vec3 v1,
    vec3 v2,
    vec3 start,
    vec3 dir,
    vec_type *t) {
    vec3 v0v1 = v1 - v0;
    vec3 v0v2 = v2 - v0;
    vec3 pvec = cross(dir, v0v2);
    vec_type det = dot(v0v1, pvec);
    if (det < EPS) return false;
    vec_type invDet = 1/det;
    vec3 tvec = start - v0;
    vec_type u = dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return false;
    vec3 qvec = cross(tvec, v0v1);
    vec_type v = dot(dir, qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
    *t = dot(v0v2, qvec) * invDet;
    return *t > 0;
}

color
trace_ray(vec3 start,
        vec3 dir,
        global Object *objects,
        int objcount,
        global vec4 *verts,
        int vertcount,
        global int3 *tris,
        int tricount,
        int depth,
        vec_type str,
        global Object *ignore) {
    vec_type minHit;
    vec3 normal;
    int didHit = 0;
    for (int i = 0; i < tricount; i++) {
        vec3 v1 = verts[tris[i].x].xyz,
             v2 = verts[tris[i].y].xyz,
             v3 = verts[tris[i].z].xyz;
        vec_type t;
        if (hit_triangle(v1.xyz, v2.xyz, v3.xyz, start, dir, &t)) {
            if (!didHit || t < minHit) {
                didHit = 1;
                minHit = t;
                normal = normalize(cross(v2-v1, v3-v1));
            }
        }
    }
    if (didHit) {
        dir = dir - 2 * dot(dir, normal) * normal;
    }
    return convert_color(str * (dir + 1) / 2);
    /*
    if (str < 0.01) {
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
            str * 0.9,
            minHit.obj);
    }

    return convert_float3(str * (dir + 1) / 2);
     */
}

kernel void
render(write_only image2d_t image,
        global vec4 cam[4],
        global struct Object *objects,
        int objcount,
        global vec4 *verts,
        int vertcount,
        global int3 *tris,
        int tricount) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX = get_global_size(0);
    const uint resY = get_global_size(1);
    const vec3 origin = new_vec3(
        cam[0].z / cam[3].z,
        cam[1].z / cam[3].z,
        cam[2].z / cam[3].z
    );
    const vec3 ncp = mul(cam,
        new_vec3(
            x_coord - (vec_type)resX / 2,
            y_coord - (vec_type)resY / 2,
            -1
        )
    );
    const vec3 fcp = mul(cam,
        new_vec3(
            x_coord - (vec_type)resX / 2,
            y_coord - (vec_type)resY / 2,
            1
        )
    );
    const vec3 dir = normalize((fcp - ncp).xyz);
    write_imagef(image, (int2){
        x_coord,
        y_coord
    }, (color4)
    {
        trace_ray(origin, dir, objects, objcount, verts, vertcount, tris,
            tricount, 500, 1, NULL), 1.0
    }
    );
}
