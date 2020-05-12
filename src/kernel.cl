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

#define EPS 0.00000001

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
            int children[2];
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
    if (tzmax < *tmax) {
        *tmax = tzmax;
        *far = 5 - r.sign.z;
    }
    return *tmax > 0;
}

void
hit_AABB2(vec3 bounds[], Ray r, vec_t *tmax, KD_SIDE *far) {
    vec_t tymin, tymax, tzmin, tzmax;

    *far = 1 - r.sign.x;
    *tmax = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if (tymax < *tmax) {
        *tmax = tymax;
        *far = 3 - r.sign.y;
    }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

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

int
get_leaf(global kdnode *kd_tree, int index, vec3 point) {
    kdnode curr = kd_tree[index];
    if (curr.type == KD_LEAF) return index;
    vec_t v;
    switch(curr.split.axis) {
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
    printf("{%f, %f, %f}", vec.x, vec.y, vec.z);
}
/*
bool
traverse_kd(
        int index,
        vec3 p1,
        vec3 p2,
        vec3 dir,
        vec3 invDir,
        KD_SIDE near,
        KD_SIDE far,
        global vec4 *verts,
        global int3 *tris,
        global kdnode *kd_tree,
        color *col,
        float *str,
        bool isPrinter) {
    if (kd_tree[index].type == KD_LEAF) {
        return true;
    }
    vec_t d, v = kd_tree[index].split.value;
    int index1, index2;
    KD_SIDE side;
    bool l1, l2;
    vec3 dir1 = dir, dir2 = dir;
    switch (kd_tree[index].split.axis) {
        case KD_X:
            d = (v - p1.x) * invDir.x;
            if (p1.x <= v) {
                p1.x /= v;
                dir1.x /= v;
                index1 = kd_tree[index].split.children[0];
                index2 = kd_tree[index].split.children[1];
                side = KD_RIGHT;
                l1 = true;
            } else {
                p1.x = (p1.x - v) / (1 - v);
                dir1.x = (dir1.x - v) / (1 - v);
                index1 = kd_tree[index].split.children[1];
                index2 = kd_tree[index].split.children[0];
                side = KD_LEFT;
                l1 = false;
            }
            dir1 = normalize(dir1);
            if (p2.x <= v) {
                p2.x /= v;
                dir2.x /= v;
                l2 = true;
            } else {
                p2.x = (p2.x - v) / (1 - v);
                dir2.x = (dir2.x - v) / (1 - v);
                l2 = false;
            }
            dir2 = normalize(dir2);
            if (l1 && l2) {
                return traverse_kd(kd_tree[index].split.children[0], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            } else if (!l1 && !l2) {
                return traverse_kd(kd_tree[index].split.children[1], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            }
            break;
        case KD_Y:
            d = (v - p1.y) * invDir.y;
            if (p1.y <= v) {
                p1.y /= v;
                dir1.y /= v;
                index1 = kd_tree[index].split.children[0];
                index2 = kd_tree[index].split.children[1];
                side = KD_UP;
                l1 = true;
            } else {
                p1.y = (p1.y - v) / (1 - v);
                dir1.y = (dir1.y - v) / (1 - v);
                index1 = kd_tree[index].split.children[1];
                index2 = kd_tree[index].split.children[0];
                side = KD_DOWN;
                l1 = false;
            }
            dir1 = normalize(dir1);
            if (p2.y <= v) {
                p2.y /= v;
                dir2.y /= v;
                l2 = true;
            } else {
                p2.y = (p2.y - v) / (1 - v);
                dir2.y = (dir2.y - v) / (1 - v);
                l2 = false;
            }
            dir2 = normalize(dir2);
            if (l1 && l2) {
                return traverse_kd(kd_tree[index].split.children[0], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            } else if (!l1 && !l2) {
                return traverse_kd(kd_tree[index].split.children[1], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            }
            break;
        default:
            d = (v - p1.z) * invDir.z;
            if (p1.z <= v) {
                p1.z /= v;
                dir1.z /= v;
                index1 = kd_tree[index].split.children[0];
                index2 = kd_tree[index].split.children[1];
                side = KD_FRONT;
                l1 = true;
            } else {
                p1.z = (p1.z - v) / (1 - v);
                dir1.z = (dir1.z - v) / (1 - v);
                index1 = kd_tree[index].split.children[1];
                index2 = kd_tree[index].split.children[0];
                side = KD_BACK;
                l1 = false;
            }
            dir1 = normalize(dir1);
            if (p2.z <= v) {
                p2.z /= v;
                dir2.z /= v;
                l2 = true;
            } else {
                p2.z = (p2.z - v) / (1 - v);
                dir2.z = (dir2.z - v) / (1 - v);
                l2 = false;
            }
            dir2 = normalize(dir2);
            if (l1 && l2) {
                return traverse_kd(kd_tree[index].split.children[0], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            } else if (!l1 && !l2) {
                return traverse_kd(kd_tree[index].split.children[1], p1, p2,
                        dir1, 1/dir1, near, far, verts, tris, kd_tree, col,
                        str, isPrinter);
            }
            break;
    }
    *col = (1-*str) * (*col) + *str * (1 + new_color(
        (side == KD_LEFT) - (side == KD_RIGHT),
        (side == KD_DOWN) - (side == KD_UP),
        (side == KD_BACK) - (side == KD_FRONT)
    )/2);
    *str *= 0.5;
    traverse_kd(index1, p1, p1 + dir * d, dir1, 1/dir1, near,
            side, verts, tris, kd_tree, col, str,
            isPrinter);
    traverse_kd(index2, p1 + dir * d, p2, dir2, 1/dir2,
            1+side-2*(side%2), far, verts, tris, kd_tree, col, str,
            isPrinter);
    return true;
}
 */

typedef union vec_arr {
    vec_t scalar[3];
    vec3 vector;
} vec_arr;

color
trace_ray(Ray r,
        global Object *objects,
        int objcount,
        global vec4 *verts,
        global int3 *tris,
        global kdnode *kd_tree,
        int depth,
        global Object *ignore,
        bool isPrinter) {
    vec_t tmin, tmax, minHit;
    KD_SIDE near, far;
    if (hit_AABB(
            (vec3[]){
                kd_tree[0].min,
                kd_tree[0].max
            }, r, &tmin, &tmax, &near, &far)) {
        vec_arr p1;
        p1.vector = r.orig;
        if (tmin > 0) {
            p1.vector += tmin * r.dir;
        }
        int index = 0, didHit = 0;
        vec_t minHit = 0;
        vec3 normal = 0;
        color col = 0;
        float str = 1;
        while (index != -1) {
            while (kd_tree[index].type == KD_SPLIT) {
                int axis = kd_tree[index].split.axis;
                int cond = p1.scalar[axis] > kd_tree[index].split.value;
                index = kd_tree[index].split.children[cond];
            }
            for (int i = 0; i < kd_tree[index].leaf.tri_count; i++) {
                int3 tri = tris[kd_tree[index].leaf.tris + i];
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
            if (didHit) break;
            if (!hit_AABB((vec3[]){
                    kd_tree[index].min,
                    kd_tree[index].max
                }, r, &tmin, &tmax, &near, &far)) {
                return new_color(1,0,0);
            }
            //hit_AABB2((vec3[]){
            //        kd_tree[index].min,
            //        kd_tree[index].max
            //    }, r, &tmax, &far);
            col = (1 - str)*col + str*(1 + new_color(
                    (far == KD_LEFT) - (far == KD_RIGHT),
                    (far == KD_DOWN) - (far == KD_UP),
                    (far == KD_BACK) - (far == KD_FRONT)
            )) / 2;
            str *= 1.0/(1.0 + kd_tree[index].leaf.tri_count/100.0);
            index = kd_tree[index].ropes[far];
            p1.vector = r.orig + tmax * r.dir;
        }
        if (didHit) {
            return (1-str)*col + str*(normal+1)/2;
        }
        return (1-str)*col;
    }
    return 0;
}
/*
color
trace_ray(Ray r,
        global Object *objects,
        int objcount,
        global vec4 *verts,
        global int3 *tris,
        global kdnode *kd_tree,
        int depth,
        global Object *ignore,
        bool isPrinter) {
    if (isPrinter) {
        int index = 0;
        KD_SIDE near, far;
        vec_t tmin, tmax;
        if (hit_AABB(
            (vec3[]){
                kd_tree[index].min,
                kd_tree[index].max
            }, r, &tmin, &tmax, &near, &far)) {
            vec3 hit = r.orig;
            vec3 ext = kd_tree[index].max - kd_tree[index].min;
            if (tmin > 0) {
                hit += r.dir * tmin;
            }
            hit = (hit - kd_tree[index].min) / ext;
            printf("%d\n", near);
            while (kd_tree[index].type != KD_LEAF){
                if (kd_tree[index].split.axis == near/2) {
                    switch(near%2) {
                        case 0:
                            index = kd_tree[index].split.children[0];
                            break;
                        case 1:
                            index = kd_tree[index].split.children[1];
                            break;
                    }
                } else {
                    switch(kd_tree[index].split.axis) {
                        case KD_X:
                            if (hit.x < kd_tree[index].split.value) {
                                hit.x /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                hit.x = (hit.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                        case KD_Y:
                            if (hit.y < kd_tree[index].split.value) {
                                hit.y /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                hit.y = (hit.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                        default: // KD_Z
                            if (hit.z < kd_tree[index].split.value) {
                                hit.z /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                hit.z = (hit.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                    }

                }
                printf("\t%d ", index);
                print_vec3(hit);
                printf("\n");
            }
            printf("\n");
            ext = kd_tree[index].max - kd_tree[index].min;
            vec3 dir = normalize(r.dir / ext);
            vec3 inv = 1/dir;
        }
        return 1;
    }


    vec_t minHit;
    vec3 normal;
    int didHit = 0;
    vec_t tmin, tmax;
    KD_SIDE near, far;
    int index = 0;
    kdnode curr = kd_tree[0]; // Root
    int triCount = 0;
    color col = 0;
    vec_t str = 1;
    if (hit_AABB((vec3[]){curr.min, curr.max}, r, &tmin, &tmax, &near, &far)) {
        tmax = tmin;
        vec3 point = r.orig;
        if (tmin > 0) {
            point += r.dir * tmin;
        }
        while (index != -1) {
            curr = kd_tree[index];
            while (curr.type != KD_LEAF) {
                if (kd_tree[index].split.axis == near/2) {
                    switch(near%2) {
                        case 0:
                            index = kd_tree[index].split.children[0];
                            break;
                        case 1:
                            index = kd_tree[index].split.children[1];
                            break;
                    }
                } else {
                    switch(kd_tree[index].split.axis) {
                        case KD_X:
                            if (point.x < kd_tree[index].split.value) {
                                point.x /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                point.x = (point.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                        case KD_Y:
                            if (point.y < kd_tree[index].split.value) {
                                point.y /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                point.y = (point.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                        default: // KD_Z
                            if (point.z < kd_tree[index].split.value) {
                                point.z /= kd_tree[index].split.value;
                                index = kd_tree[index].split.children[0];
                            } else {
                                point.z = (point.x - kd_tree[index].split.value) /
                                        (1 - kd_tree[index].split.value);
                                index = kd_tree[index].split.children[1];
                            }
                            break;
                    }

                }
                curr = kd_tree[index];
            }
            return new_color((vec_t)index/255, 0, 0);
            //index = get_leaf(kd_tree, index, point);
            //curr = kd_tree[index];
            triCount += curr.leaf.tri_count;
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
            if (didHit) {
                //dist += (minHit - tmax) * (vec_t)curr.leaf.tri_count;
                break;
            }
            //far = hit_AABB2((vec3[]){curr.min, curr.max}, r, &tmax);
            if (!hit_AABB((vec3[]){curr.min, curr.max}, r, &tmin, &tmax, &near,
                &far)) {
                break;
            }
            //dist += (tmax - tmin) * (vec_t)curr.leaf.tri_count;
            point = r.orig + r.dir * tmax;
            col = (1-str)*col + str * (1 + new_color(
                (far == KD_LEFT) - (far == KD_RIGHT),
                (far == KD_DOWN) - (far == KD_UP),
                (far == KD_BACK) - (far == KD_FRONT)
            ))/2;
            str *= 0.5;
            index = curr.ropes[far];
        }
    }
    if (didHit) {
        vec3 norm = r.dir - 2 * dot(r.dir, normal) * normal;
        return (1-str)*col + str*convert_color((norm + 1)/2);
    }
    return (1-str)*col;
   // return (1 - d)*new_color(1, 0, 0);
}
 */

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
        trace_ray(r,
            objects,
            objcount,
            verts,
            tris,
            kd_tree,
            500,
            NULL,
            x_coord == resX/2 && y_coord == resY/2), 1.0
    }
    );
}
