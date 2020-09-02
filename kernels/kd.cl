#define vec_t float

#define CONCAT2(a, b) a ## b
#define CONCAT(a, b) CONCAT2(a, b)

#define vec2 CONCAT(vec_t, 2)
#define vec3 CONCAT(vec_t, 3)
#define vec4 CONCAT(vec_t, 4)
#define color float3
#define color4 float4

#define new_vec2(x, y) (vec2){x, y}
#define new_vec3(x, y, z) (vec3){x, y, z}
#define new_vec4(x, y, z, w) (vec4){x, y, z, w}
#define new_color(x, y, z) (float3){x, y, z}
#define convert_color(vec) convert_float3(vec)
#define convert_vec3(vec) CONCAT(convert_, vec3)(vec)

#define EPS 0.0000

typedef vec4 matrix[4];

typedef struct __attribute__((__packed__)) Object {
    vec4 position;
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

typedef struct __attribute__((__packed__)) kdnode {
    vec3 min, max;
    enum {
        KD_SPLIT, KD_LEAF
    } type;
    union {
        struct {
            vec_t value;
            KD_AXIS axis;
            int children[2];
        } split;
        struct {
            int tris;
            int tri_count;
            int ropes[6];
        } leaf;
    };
} kdnode;

typedef struct Hit {
    float dist;
    global Object *obj;
} Hit;

typedef struct Ray {
    vec3 orig;
    vec3 dir;
    vec3 invdir;
    int3 sign;
} Ray;

Ray
new_Ray(vec3 orig, vec3 dir) {
    vec3 invdir = 1 / dir;
    return (Ray){
            orig, dir, invdir, (int3){
                    invdir.x < 0, invdir.y < 0, invdir.z < 0
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

    if ((*tmin > tymax) || (tymin > *tmax)) {
        return false;
    }
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

    if ((*tmin > tzmax) || (tzmin > *tmax)) {
        return false;
    }
    if (tzmin > *tmin) {
        *tmin = tzmin;
        *near = 4 + r.sign.z;
    }
    if (tzmax < *tmax) {
        *tmax = tzmax;
        *far = 5 - r.sign.z;
    }
    return *tmax > 0;
}

void
traverse_AABB(vec3 bounds[], Ray r, vec_t *tmin, vec_t *tmax, KD_SIDE *far) {
    vec_t tymin, tymax, tzmin, tzmax;

    *far = 1 - r.sign.x;
    *tmin = (bounds[r.sign.x].x - r.orig.x) * r.invdir.x;
    *tmax = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if (tymin > *tmin) {
        *tmin = tymin;
    }
    if (tymax < *tmax) {
        *tmax = tymax;
        *far = 3 - r.sign.y;
    }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

    if (tzmin > *tmin) {
        *tmin = tzmin;
    }
    if (tzmax < *tmax) {
        *tmax = tzmax;
        *far = 5 - r.sign.z;
    }
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
    if (c <= 0) {
        return false;
    }
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
        vec_t *t,
        vec2 *uv) {
    vec3 v0v1 = v1 - v0;
    vec3 v0v2 = v2 - v0;
    vec3 pvec = cross(dir, v0v2);
    vec_t det = dot(v0v1, pvec);
    if (det < EPS) {
        return false;
    }
    vec_t invDet = 1 / det;
    vec3 tvec = start - v0;
    uv->x = dot(tvec, pvec) * invDet;
    if (uv->x < 0 || uv->x > 1) {
        return false;
    }
    vec3 qvec = cross(tvec, v0v1);
    uv->y = dot(dir, qvec) * invDet;
    if (uv->y < 0 || uv->x + uv->y > 1) {
        return false;
    }
    *t = dot(v0v2, qvec) * invDet;
    return *t > 0;
}

int
get_leaf(global kdnode *kd_tree, int index, vec3 point) {
    kdnode curr = kd_tree[index];
    if (curr.type == KD_LEAF) {
        return index;
    }
    vec_t v;
    switch (curr.split.axis) {
        case KD_X:
            v = (-curr.min.x + point.x) / (curr.max.x - curr.min.x);
            break;
        case KD_Y:
            v = (-curr.min.y + point.y) / (curr.max.y - curr.min.y);
            break;
        case KD_Z:
            v = (-curr.min.z + point.z) / (curr.max.x - curr.min.z);
            break;
    }
    if (v < curr.split.value) {
        return get_leaf(kd_tree, curr.split.children[0], point);
    }
    return get_leaf(kd_tree, curr.split.children[1], point);
}

void
print_vec2(vec2 vec) {
    printf("{%f, %f}", vec.x, vec.y);
}

void
print_vec3(vec3 vec) {
    printf("{%.3f, %.3f, %.3f}", vec.x, vec.y, vec.z);
}

typedef union vec_arr {
    vec_t scalar[3];
    vec3 vector;
} vec_arr;

color
trace_ray(Ray r,
        global Object *objects,
        int objcount,
        global vec4 *verts,
        global vec4 *norms,
        global int3 *tris,
        global int *tri_indices,
        global kdnode *kd_tree,
        int depth,
        color col,
        float str,
        bool isPrinter) {
    vec_t tmin, tmax, minHit;
    KD_SIDE near, far;
    if (depth > 0 && hit_AABB((vec3[]){
            kd_tree[0].min, kd_tree[0].max
    }, r, &tmin, &tmax, &near, &far)) {
        vec_arr p1;
        p1.vector = r.orig;
        if (tmin > 0) {
            p1.vector += tmin * r.dir;
        }
        int index = 0, didHit = 0;
        vec_t minHit = 0;
        vec3 normal = 0;
        int count = 0;
        while (index != -1) {
            count++;
            while (kd_tree[index].type == KD_SPLIT) {
                int axis = kd_tree[index].split.axis;
                int cond = p1.scalar[axis] > kd_tree[index].split.value;
                index = kd_tree[index].split.children[cond];
                count++;
            }
            count += kd_tree[index].leaf.tri_count;
            if (kd_tree[index].leaf.tris != -1) {
                for (int i = 0; i < kd_tree[index].leaf.tri_count; i++) {
                    int a = kd_tree[index].leaf.tris + i;
                    int b = tri_indices[a];
                    int3 t1 = tris[3*b+0],
                         t2 = tris[3*b+1],
                         t3 = tris[3*b+2];
                    vec3 v1 = verts[t1.x].xyz, v2 = verts[t2.x].xyz,
                            v3 = verts[t3.x].xyz;
                    vec_t t = 0;
                    vec2 uv;
                    if (hit_triangle(v1, v2, v3, r.orig, r.dir, &t, &uv)) {
                        if (!didHit || t <= minHit) {
                            didHit = 1;
                            minHit = t;

                            //normal = norms[t1.y].xyz;
                            if (t1.y >= 0) {
                                vec3 A = new_vec3((t1.x % 256) / 255.0,
                                        ((t1.x / 256) % 256) / 255.0,
                                        ((t1.x / 256 / 256) % 256) / 255.0),
                                    B = new_vec3((t2.x % 256) / 255.0,
                                        ((t2.x / 256) % 256) / 255.0,
                                        ((t2.x / 256 / 256) % 256) / 255.0),
                                    C = new_vec3((t3.x % 256) / 255.0,
                                        ((t3.x / 256) % 256) / 255.0,
                                        ((t3.x / 256 / 256) % 256) / 255.0);
                                normal = normalize(norms[t1.y].xyz *
                                        (1.0f - uv.x - uv.y) +
                                        norms[t2.y].xyz * uv.x +
                                        norms[t3.y].xyz * uv.y);
                            } else {
                                normal = normalize(cross(v2 - v1, v3 - v1));
                            }
                        }
                    }
                }
            }
            traverse_AABB((vec3[]){
                    kd_tree[index].min, kd_tree[index].max
            }, r, &tmin, &tmax, &far);
            /*
            col = (1-str)*col + str*(1+new_color(
                (far == KD_LEFT) - (far == KD_RIGHT),
                (far == KD_DOWN) - (far == KD_UP),
                (far == KD_BACK) - (far == KD_FRONT)
            ))/2;
            str *= 0.8;
            */
            if (didHit && tmin + 0.001 > minHit) {
                break;
            }
            index = kd_tree[index].leaf.ropes[far];
            p1.vector = r.orig + tmax * r.dir;
            if (index == -1) {
                break;
            }
        }
        //int G = count % 256;
        //int R = count / 256 % 256;
        //R = (256 - R) % 256;
        //int B = count / 256 / 256 % 256;
        //B = (256 - B) % 256;
        if (didHit) {
            return convert_color((normal + 1) / 2)/* / 2 +
                new_color(R / 255.0, G / 255.5, B / 255.5) / 2*/;

            vec3 newOrig = r.orig + r.dir * minHit;
            vec3 newDir = normalize(r.dir - 2 * dot(r.dir, normal) * normal);
            newOrig += newDir * 0.0001f;
            Ray newRay = new_Ray(newOrig, newDir);
            col = (1 - str) * col + str * convert_color((normal + 1) / 2);
            str *= 0.2f;
            //return convert_color((normal + 1) / 2);
            return trace_ray(newRay,
                    objects,
                    objcount,
                    verts,
                    norms,
                    tris,
                    tri_indices,
                    kd_tree,
                    depth - 1,
                    col,
                    str,
                    isPrinter);
        }
        //return new_color(R / 255.0, G / 255.5, B / 255.5)/2;
    }
    return (1-str)*col + str;
}

kernel void
render(write_only image2d_t image,
        global vec4 cam[4],
        global struct Object *objects,
        int objcount,
        global vec4 *verts,
        global vec4 *norms,
        global int3 *tris,
        global int *tri_indices,
        global kdnode *kd_tree) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX = get_global_size(0);
    const uint resY = get_global_size(1);
    write_imagef(image, (int2){
            x_coord, y_coord
    }, (color4){
        0, 1, 0, 1
    });
    const vec3 origin = new_vec3(cam[0].z / cam[3].z,
            cam[1].z / cam[3].z,
            cam[2].z / cam[3].z);

    const vec3 ncp = mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, -1)

    );
    const vec3 fcp = mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, 1)

    );
    const vec3 dir = normalize((fcp - ncp).xyz);
    Ray r = new_Ray(origin, dir);
    write_imagef(image, (int2){
            x_coord, y_coord
    }, (color4){
            trace_ray(r,
                    objects,
                    objcount,
                    verts,
                    norms,
                    tris,
                    tri_indices,
                    kd_tree,
                    2,
                    0,
                    1.0,
                    x_coord == resX / 2 && y_coord == resY / 2), 1.0
    });
}
