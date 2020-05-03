#define vec_t float

#define CONCAT2(a, b) a ## b
#define CONCAT(a, b) CONCAT2(a, b)

#define vec3 CONCAT(vec_t, 3)
#define vec4 CONCAT(vec_t, 4)
#define color float3
#define color4 float4

#define new_vec3(x, y, z) (vec3){x, y, z}
#define new_vec4(x, y, z, w) (vec4){x, y, z, w}
#define new_color(x, y, z) (float3){x, y, z}
#define convert_color(vec) convert_float3(vec)
#define convert_vec3(vec) CONCAT(convert_, vec3)(vec)

#define EPS 0.000001

typedef vec4 matrix[4];

typedef struct __attribute__ ((packed)) Object {
    vec3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            vec_t radius;
        } sphere;
    };
} Object;

typedef enum KD_AXIS {
    KD_X, KD_Y, KD_Z
} KD_AXIS;

typedef enum KD_SIDE {
    KD_LEFT = 0,
    KD_RIGHT = 1,
    KD_DOWN = 2,
    KD_UP = 3,
    KD_BACK = 4,
    KD_FRONT = 5
} KD_SIDE;

typedef struct __attribute__ ((packed)) kdnode {
    int ropes[6];
    vec3 min, max;
    enum {
        KD_SPLIT, KD_LEAF
    } type;
    union {
        struct {
            vec_t value;
            KD_AXIS axis;
            int low_child;
            int high_child;
        } split;
        struct {
            int tris;
            int tri_count;
        } leaf;
    };
} kdnode;

typedef struct Hit {
    float dist;
    __global Object *obj;
} Hit;

typedef struct Ray {
    vec3 orig;
    vec3 dir;
    vec3 invdir;
    int3 sign;
} Ray;

Ray
new_Ray(vec3 orig, vec3 dir) {
    vec3 invdir = 1/dir;
    return (Ray){
        orig,
        dir,
        invdir,
        (int3){
            invdir.x < 0,
            invdir.y < 0,
            invdir.z < 0
        }
    };
}

vec3
mul(const matrix M, vec3 X) {
    return new_vec3(dot(M[0].xyz, X) + M[0].w,
        dot(M[1].xyz, X) + M[1].w,
        dot(M[2].xyz, X) + M[2].w) / (dot(M[3].xyz, X) + M[3].w);
}

vec_t
mod(vec_t a, vec_t b) {
    return fmod(fmod(a, b) + b, b);
}

bool
hit_AABB(vec3 bounds[],
        Ray r,
        vec_t *tmin,
        vec_t *tmax,
        KD_SIDE *near,
        KD_SIDE *far) {
    vec_t tymin, tymax, tzmin, tzmax;

    *near = r.sign.x;
    *far = 1 - r.sign.x;
    *tmin = (bounds[r.sign.x].x - r.orig.x) * r.invdir.x;
    *tmax = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if ((*tmin > tymax) || (tymin > *tmax))
        return false;
    if (tymin > *tmin) {
        *tmin = tymin;
        *near = 2 + r.sign.y;
    }
    if (tymax < *tmax) {
        *tmax = tymax;
        *far = 3 - r.sign.y;
    }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

    if ((*tmin > tzmax) || (tzmin > *tmax))
        return false;
    if (tzmin > *tmin) {
        *tmin = tzmin;
        *near = 4 + r.sign.z;
    }
    if (*tmin < 0) return false;
    if (tzmax < *tmax) {
        *tmax = tzmax;
        *far = 5 - r.sign.z;
    }
    return true;
}

bool
solveQuadratic(vec_t a, vec_t b, vec_t c, vec_t *x0, vec_t *x1) {
    vec_t discr = b * b - 4 * a * c;
    if (discr < 0) {
        return false;
    } else if (discr == 0) {
        *x0 = *x1 = -0.5 * b / a;
    } else {
        vec_t q = (b > 0)
            ? -0.5 * (b + sqrt(discr))
            : -0.5 * (b - sqrt(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1) {
        vec_t tmp = *x0;
        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

bool
hit_sphere(vec3 center, vec_t radius2, vec3 start, vec3 dir, vec_t *t) {
    vec_t t0, t1;
    vec3 L = start - center;
    vec_t a = dot(dir, dir);
    vec_t b = 2 * dot(dir, L);
    vec_t c = dot(L, L) - radius2;
    if (c <= 0) return false;
    if (!solveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }
    if (t0 > t1) {
        vec_t tmp = t0;
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
    vec_t *t) {
    vec3 v0v1 = v1 - v0;
    vec3 v0v2 = v2 - v0;
    vec3 pvec = cross(dir, v0v2);
    vec_t det = dot(v0v1, pvec);
    if (det < EPS) return false;
    vec_t invDet = 1/det;
    vec3 tvec = start - v0;
    vec_t u = dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return false;
    vec3 qvec = cross(tvec, v0v1);
    vec_t v = dot(dir, qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
    *t = dot(v0v2, qvec) * invDet;
    return *t > 0;
}

KD_SIDE
oppSide(Ray scaledR, KD_SIDE near, vec3 *pos) {
    vec3 d = scaledR.invdir * (1 - convert_vec3(scaledR.sign) - scaledR.orig);
    if (d.x < d.y) {
        if (d.x < d.z) {
            *pos = scaledR.orig + scaledR.dir * d.x;
            return 1 - scaledR.sign.x;
        } else {
            *pos = scaledR.orig + scaledR.dir * d.z;
            return 5 - scaledR.sign.z;
        }
    } else if (d.y < d.z) {
        *pos = scaledR.orig + scaledR.dir * d.y;
        return 3 - scaledR.sign.y;
    } else {
        *pos = scaledR.orig + scaledR.dir * d.z;
        return 5 - scaledR.sign.z;
    }
}

#define MAX_ITER 400

color
trace_ray(Ray r,
        global Object *objects,
        int objcount,
        global vec4 *verts,
        global int3 *tris,
        global kdnode *kd_tree,
        int depth,
        vec_t str,
        global Object *ignore,
        bool isPrinter) {
    vec_t minHit;
    vec3 normal;
    int didHit = 0;
    kdnode root = kd_tree[0];
    vec_t tmin, tmax;
    KD_SIDE near, far;
    int count = 0;
    if (hit_AABB((vec3[]){root.min, root.max}, r, &tmin, &tmax, &near, &far)) {
        vec3 pos = r.orig + tmin * r.dir;
        int index = 0;
        for (int iter = 0; iter <= MAX_ITER && index != -1 && !didHit; iter++) {
            if (iter == MAX_ITER) {
                if (isPrinter) {
                    printf("%d\n", index);
                }
                return new_color(1, 1, 1);
            }
            kdnode curr = kd_tree[index];
            while (curr.type != KD_LEAF) {
                vec_t v;
                switch(curr.split.axis) {
                    case KD_X:
                        v = pos.x;
                        break;
                    case KD_Y:
                        v = pos.y;
                        break;
                    case KD_Z:
                        v = pos.z;
                        break;
                }
                if (v <= curr.split.value) {
                    index = curr.split.low_child;
                } else {
                    index = curr.split.high_child;
                }
                curr = kd_tree[index];
            }
            // Transform ray so that the AABB's min is at the origin and its max
            // is at (1, 1, 1), and the ray's origin is at the intersection
            // point.
            vec3 ext = curr.max - curr.min;
            Ray scaledR = new_Ray(
                (pos - curr.min) / ext,
                r.dir / ext
            );
            count += curr.leaf.tri_count;
            for (int i = 0; i < curr.leaf.tri_count; i++) {
                int3 tri = tris[curr.leaf.tris + i];
                vec3 v1 = verts[tri.x].xyz,
                     v2 = verts[tri.y].xyz,
                     v3 = verts[tri.z].xyz;
                vec_t t;
                if (hit_triangle(v1, v2, v3, r.orig, r.dir, &t)) {
                    if (!didHit || t < minHit) {
                        didHit = 1;
                        minHit = t;
                        normal = normalize(cross(v2 - v1, v3 - v1));
                    }
                }
            }
            vec3 newPosScaled;
            far = oppSide(scaledR, near, &newPosScaled);
            if (isPrinter) {
                printf("%d(%d) ", index, far);
            }
            pos = curr.min + newPosScaled * (curr.max - curr.min);
            index = curr.ropes[far];
        }
    }
    if (isPrinter) {
        printf("\n");
        return 1;
    }
    /*
    for (int i = 0; i < tricount; i++) {
        vec3 v1 = verts[tris[i].x].xyz,
             v2 = verts[tris[i].y].xyz,
             v3 = verts[tris[i].z].xyz;
        vec_t t;
        if (hit_triangle(v1.xyz, v2.xyz, v3.xyz, start, dir, &t)) {
            if (!didHit || t < minHit) {
                didHit = 1;
                minHit = t;
                normal = normalize(cross(v2-v1, v3-v1));
            }
        }
    }
    */
    vec_t R = (count % 256) / 255.0;
    count /= 256;
    vec_t G = 0;
    if (count > 0)
        G = 1.0 - (count % 256) / 255.0;
    count /= 256;
    vec_t B = 0;
    if (count > 0)
        B = 1.0 - (count % 256) / 255.0;
    return new_color(R, G, B);
    /*
    vec3 dir = r.dir;
    if (didHit) {
        dir = dir - 2 * dot(dir, normal) * normal;
    }
    str = 1.0 / (count/100.0 + 1);
    return convert_color(str * (dir + 1) / 2);
     */
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
        global int3 *tris,
        global kdnode *kd_tree) {
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
            x_coord - (vec_t)resX / 2,
            y_coord - (vec_t)resY / 2,
            -1
        )
    );
    const vec3 fcp = mul(cam,
        new_vec3(
            x_coord - (vec_t)resX / 2,
            y_coord - (vec_t)resY / 2,
            1
        )
    );
    const vec3 dir = normalize((fcp - ncp).xyz);
    Ray r = new_Ray(origin, dir);
    write_imagef(image, (int2){
        x_coord,
        y_coord
    }, (color4)
    {
        trace_ray(r, objects, objcount, verts, tris, kd_tree,
            500, 1,
            NULL,
            x_coord == resX/2 && y_coord == resY/2), 1.0
    }
    );
}
