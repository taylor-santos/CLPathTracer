#define vec_t       float
#define VOXEL_DEPTH 3u

#define CONCAT2(a, b) a##b
#define CONCAT(a, b)  CONCAT2(a, b)

#define vec2   CONCAT(vec_t, 2)
#define vec3   CONCAT(vec_t, 3)
#define vec4   CONCAT(vec_t, 4)
#define color3 float3
#define color4 float4

#define new_vec2(x, y) \
    (vec2) {           \
        x, y           \
    }
#define new_vec3(x, y, z) \
    (vec3) {              \
        x, y, z           \
    }
#define new_vec4(x, y, z, w) \
    (vec4) {                 \
        x, y, z, w           \
    }
#define new_color(r, g, b) \
    (float3) {             \
        r, g, b            \
    }
#define new_color4(r, g, b, a) \
    (float4) {                 \
        r, g, b, a             \
    }
#define convert_color(vec) convert_float3(vec)
#define convert_vec3(vec)  CONCAT(convert_, vec3)(vec)

#define print \
    if (debug) printf

typedef struct __attribute__((__packed__)) Voxel {
    enum { INTERNAL, LEAF } type;
    union {
        struct {
            uint children[8];
        } internal;
        struct {
            color4 color;
        } leaf;
    };
} Voxel;

typedef enum AXIS { AXIS_X, AXIS_Y, AXIS_Z } AXIS;

typedef enum SIDE {
    SIDE_LEFT  = 0,
    SIDE_RIGHT = 1,
    SIDE_DOWN  = 2,
    SIDE_UP    = 3,
    SIDE_BACK  = 4,
    SIDE_FRONT = 5
} SIDE;

int constant opposite[9][4] = {
    {0},
    {0, 2, 3, 5},
    {0, 1, 4, 6},
    {0, 4, 1, 7},
    {0, 3, 2, 8},
    {0, 6, 7, 1},
    {0, 5, 8, 2},
    {0, 8, 5, 3},
    {0, 7, 6, 4}};

vec4 constant corner[8] = {
    (vec4)(0, 0, 0, 0),
    (vec4)(1, 0, 0, 0),
    (vec4)(0, 1, 0, 0),
    (vec4)(1, 1, 0, 0),
    (vec4)(0, 0, 1, 0),
    (vec4)(1, 0, 1, 0),
    (vec4)(0, 1, 1, 0),
    (vec4)(1, 1, 1, 0)};

typedef vec4 matrix[4];

vec3
mul(const matrix M, vec3 X) {
    return new_vec3(
               dot(M[0].xyz, X) + M[0].w,
               dot(M[1].xyz, X) + M[1].w,
               dot(M[2].xyz, X) + M[2].w) /
           (dot(M[3].xyz, X) + M[3].w);
}

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
        orig,
        dir,
        invdir,
        (int3){invdir.x < 0, invdir.y < 0, invdir.z < 0}};
}

bool
hit_AABB(
    vec3   bounds[],
    Ray    r,
    vec_t *p_tmin,
    vec_t *p_tmax,
    SIDE * p_near,
    SIDE * p_far) {
    vec_t tymin, tymax, tzmin, tzmax;

    *p_near = r.sign.x;
    *p_far  = 1 - r.sign.x;
    *p_tmin = (bounds[r.sign.x].x - r.orig.x) * r.invdir.x;
    *p_tmax = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin   = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax   = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if ((*p_tmin > tymax) || (tymin > *p_tmax)) { return false; }
    if (tymin > *p_tmin) {
        *p_tmin = tymin;
        *p_near = 2 + r.sign.y;
    }
    if (tymax < *p_tmax) {
        *p_tmax = tymax;
        *p_far  = 3 - r.sign.y;
    }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

    if ((*p_tmin > tzmax) || (tzmin > *p_tmax)) { return false; }
    if (tzmin > *p_tmin) {
        *p_tmin = tzmin;
        *p_near = 4 + r.sign.z;
    }
    if (tzmax < *p_tmax) {
        *p_tmax = tzmax;
        *p_far  = 5 - r.sign.z;
    }
    return *p_tmax > 0;
}

int4
box_subdivs(vec3 spt, vec3 invdir, vec_t dt) {
    int4 res     = -1;
    vec3 dists   = ((vec3)0.5 - spt) * invdir;
    int3 valids  = isgreaterequal(dists, 0) * isless(dists, (vec3)dt);
    int3 crosses = valids * (int3){1, 2, 3};
    dists += convert_vec3(-!valids) * MAXFLOAT;
    int tmp;
    if (dists.y < dists.x) {
        if (dists.z < dists.y) {
            crosses.xz = crosses.zx;
        } else {
            if (dists.z < dists.x) {
                crosses.xyz = crosses.yzx;
            } else {
                crosses.xy = crosses.yx;
            }
        }
    } else {
        if (dists.y < dists.z) {
        } else {
            if (dists.z < dists.x) {
                crosses.xyz = crosses.zxy;
            } else {
                crosses.yz = crosses.zy;
            }
        }
    }
    int3 up = -(spt >= (vec_t)0.5);
    res.x   = dot(convert_float3(up), (float3){1, 2, 4}) + 1;
    res.y   = opposite[res.x][crosses.x];
    res.z   = opposite[res.y][crosses.y];
    res.w   = opposite[res.z][crosses.z];
    return res - 1;
}

bool
voxel_march2(
    Ray    r,
    vec4   bounds,
    global Voxel *voxels,
    color3 *      col,
    bool          debug) {
    print("------START------\n");
    struct {
        union {
            int  arr[4];
            int4 vec;
        } subd;
        int  index;
        int  voxel;
        vec4 bounds;
    } state[VOXEL_DEPTH + 1];
    vec_t tmin, tmax;
    SIDE  near, far;
    int   curr = 0;
    vec3  pt, spt;

    if (!hit_AABB(
            (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
            r,
            &tmin,
            &tmax,
            &near,
            &far)) {
        return false;
    }
    pt                = r.orig + tmin * r.dir;
    spt               = (pt - bounds.xyz) / bounds.w;
    int4 subd         = box_subdivs(spt, r.invdir, (tmax - tmin) / bounds.w);
    state[0].subd.vec = subd;
    state[0].index    = 0;
    state[0].voxel    = 0;
    state[0].bounds   = bounds;

    int d = 0;
    while (curr >= 0) {
        print("Entering %d\n", curr);
        if (d > 10) {
            *col = new_color(1, 0, 0);
            return true;
        }
        d++;
        int index = state[curr].index; // subd element index (x, y, z, w)
        int vi    = state[curr].voxel; // voxels array index
        int ci    = state[curr].subd.arr[index]; // child index (0-7)
        int nextv;
        print(" Node %d is", vi);
        if (voxels[vi].type == INTERNAL) {
            print(" INTERNAL\n");
            print(
                " Node's children are [%d, %d, %d, %d, %d, %d, %d, %d]\n",
                voxels[vi].internal.children[0],
                voxels[vi].internal.children[1],
                voxels[vi].internal.children[2],
                voxels[vi].internal.children[3],
                voxels[vi].internal.children[4],
                voxels[vi].internal.children[5],
                voxels[vi].internal.children[6],
                voxels[vi].internal.children[7]);
        } else {
            print(" LEAF\n");
        }
        print(" Subd is ");
        print("%v4i\n", state[curr].subd.vec);
        print(
            " %d %d %d %d\n",
            state[curr].subd.arr[0],
            state[curr].subd.arr[1],
            state[curr].subd.arr[2],
            state[curr].subd.arr[3]);
        print(" Index is %d (%d)\n", index, state[curr].subd.arr[index]);
        if (ci == 0) {
            print(" CURRENT CHILD IS EMPTY, BACKING UP...\n");
            curr--;
            state[curr].index++;
            return false;
            continue;
        }
        nextv = voxels[vi].internal.children[ci];
        print(" Next child is %d\n", nextv);
        if (voxels[nextv].type == LEAF) {
            *col = voxels[nextv].leaf.color.xyz;
            return true;
        }
        bounds = state[curr].bounds;
        bounds.w /= 2;
        bounds.xyz += corner[ci].xyz * bounds.w;
        if (!hit_AABB(
                (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
                r,
                &tmin,
                &tmax,
                &near,
                &far)) {
            return false;
        }
        pt  = r.orig + tmin * r.dir;
        spt = (pt - bounds.xyz) / bounds.w;
        state[curr + 1].subd.vec =
            box_subdivs(spt, r.invdir, (tmax - tmin) / bounds.w);
        state[curr + 1].index  = 0;
        state[curr + 1].voxel  = nextv;
        state[curr + 1].bounds = bounds;
        curr++;
    }
    return false;
    /*
    int v = 0;
    do {
        if (!hit_AABB(
                (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
                r,
                &tmin,
                &tmax,
                &near,
                &far)) {
            *col = new_color(1, 0, 0);
            return true;
        }
        pt        = r.orig + tmin * r.dir;
        spt       = (pt - bounds.xyz) / bounds.w;
        int4 subd = box_subdivs(spt, r.invdir, (tmax - tmin) / bounds.w);
        state[curr].subd.vec = subd;
        state[curr].index    = 0;
        state[curr].voxel    = v;
        state[curr].bounds   = bounds;
        bounds.w /= 2;
        bounds.xyz += corner[subd.x].xyz * bounds.w;
        v = voxels[v].internal.children[subd.x];
        if (v == 0) { break; }
        curr++;
    } while (voxels[v].type == INTERNAL);
    curr--;
    while (curr >= 0) {
        Voxel v = voxels[state[curr].voxel];
        if (voxels[v.internal.children[state[curr].subd.arr[state[curr].index]]]
                .type == LEAF) {
            *col =
                voxels[v.internal
                           .children[state[curr].subd.arr[state[curr].index]]]
                    .leaf.color.xyz;
            return true;
        }
        break;
    }
     */
    return false;
}

bool
voxel_march(
    Ray          r,
    vec4         bounds,
    unsigned int index,
    global Voxel *voxels,
    int           depth,
    color3 *      col,
    bool          debug) {
    vec_t tmin, tmax;
    SIDE  near, far;
    union {
        int  arr[4];
        int4 vec;
    } subd;
    if (hit_AABB(
            (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
            r,
            &tmin,
            &tmax,
            &near,
            &far)) {
        if (voxels[index].type == LEAF) {
            *col = voxels[index].leaf.color.xyz;
            return true;
        }
        vec3 pt  = r.orig + tmin * r.dir;
        vec3 spt = (pt - bounds.xyz) / bounds.w;
        subd.vec = box_subdivs(spt, r.invdir, (tmax - tmin) / bounds.w);
        for (int i = 0; i < 4; i++) {
            if (subd.arr[i] == -1) { break; }
            int next_index = voxels[index].internal.children[subd.arr[i]];
            if (next_index == 0) { continue; }
            vec4 newbounds = (vec4){
                bounds.xyz + corner[subd.arr[i]].xyz * bounds.w / 2,
                bounds.w / 2};
            if (voxel_march(
                    r,
                    newbounds,
                    next_index,
                    voxels,
                    depth - 1,
                    col,
                    debug)) {
                return true;
            }
        }
        return false;
    }
    return false;
}

color3
trace_ray(Ray r, global Voxel *voxels, bool debug) {
    vec4   bounds = {0, 0, 0, 1 << VOXEL_DEPTH};
    color3 col    = (r.dir + 1) / 2;
    // voxel_march(r, bounds, 0, voxels, VOXEL_DEPTH, &col, debug);
    voxel_march2(r, bounds, voxels, &col, debug);
    if (debug) return 1;
    return col;
}

kernel void
render(write_only image2d_t image, global vec4 cam[4], global Voxel *voxels) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX    = get_global_size(0);
    const uint resY    = get_global_size(1);
    write_imagef(image, (int2){x_coord, y_coord}, (color4){0, 1, 0, 1});
    const vec3 origin =
        new_vec3(cam[0].z / cam[3].z, cam[1].z / cam[3].z, cam[2].z / cam[3].z);

    const vec3 ncp =
        mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, -1));
    const vec3 fcp =
        mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, 1)

        );
    const vec3 dir = normalize((fcp - ncp).xyz);
    Ray        r   = new_Ray(origin, dir);
    write_imagef(
        image,
        (int2){x_coord, y_coord},
        (color4){
            trace_ray(r, voxels, x_coord == resX / 2 && y_coord == resY / 2),
            1});
}