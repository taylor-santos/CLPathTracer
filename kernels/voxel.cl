
#ifdef USE_DOUBLES
#    define vec_t     double
#    define ivec_t    long
#    define MAX_VEC_T DBL_MAX
#    define PVEC      "l"
#else
#    define vec_t     float
#    define ivec_t    int
#    define MAX_VEC_T FLT_MAX
#    define PVEC      ""
#endif

#define VOXEL_DEPTH 10u

#define CONCAT2(a, b) a##b
#define CONCAT(a, b)  CONCAT2(a, b)

#define vec2 CONCAT(vec_t, 2)
#define vec3 CONCAT(vec_t, 3)
#define vec4 CONCAT(vec_t, 4)
#define vec8 CONCAT(vec_t, 8)

#define ivec3 CONCAT(ivec_t, 3)

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

#define print(args)             \
    do {                        \
        if (debug) printf args; \
    } while (false)

#define print2(args)            \
    do {                        \
        if (debug) printf args; \
    } while (false)

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

char constant opposite[9][4] = {
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
    vec3  orig;
    vec3  dir;
    vec3  invdir;
    char3 sign;
} Ray;

Ray
new_Ray(vec3 orig, vec3 dir) {
    vec3 invdir = 1 / dir;
    return (Ray){orig, dir, invdir, -convert_char3(invdir < 0)};
}

bool
hit_AABB(vec3 bounds[], Ray r, vec_t *p_tmin, vec_t *p_tmax) {
    vec_t tymin, tymax, tzmin, tzmax;

    *p_tmin = (bounds[r.sign.x].x - r.orig.x) * r.invdir.x;
    *p_tmax = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin   = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax   = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if ((*p_tmin > tymax) || (tymin > *p_tmax)) { return false; }
    if (tymin > *p_tmin) { *p_tmin = tymin; }
    if (tymax < *p_tmax) { *p_tmax = tymax; }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

    if ((*p_tmin > tzmax) || (tzmin > *p_tmax)) { return false; }
    if (tzmin > *p_tmin) { *p_tmin = tzmin; }
    if (tzmax < *p_tmax) { *p_tmax = tzmax; }
    return *p_tmax > 0;
}

vec2
next_AABB(vec3 bounds[], Ray r) {
    vec_t tymin, tymax, tzmin, tzmax, tmin, tmax;

    tmin  = (bounds[r.sign.x].x - r.orig.x) * r.invdir.x;
    tmax  = (bounds[1 - r.sign.x].x - r.orig.x) * r.invdir.x;
    tymin = (bounds[r.sign.y].y - r.orig.y) * r.invdir.y;
    tymax = (bounds[1 - r.sign.y].y - r.orig.y) * r.invdir.y;

    if (tymin > tmin) { tmin = tymin; }
    if (tymax < tmax) { tmax = tymax; }

    tzmin = (bounds[r.sign.z].z - r.orig.z) * r.invdir.z;
    tzmax = (bounds[1 - r.sign.z].z - r.orig.z) * r.invdir.z;

    if (tzmin > tmin) { tmin = tzmin; }
    if (tzmax < tmax) { tmax = tzmax; }
    return (vec2){tmin, tmax};
}

char4
box_subdivs(vec3 spt, vec3 invdir, vec_t dt, vec3 *pdists) {
    char4 res     = -1;
    vec3  dists   = ((vec3)0.5 - spt) * invdir;
    ivec3 valids  = isgreaterequal(dists, 0) * isless(dists, (vec3)dt);
    ivec3 crosses = valids * (ivec3){1, 2, 3};
    dists += convert_vec3(-!valids) * MAX_VEC_T;
    int tmp;
    if (dists.y < dists.x) {
        if (dists.z < dists.y) {
            crosses.xz = crosses.zx;
            *pdists    = dists.zyx;
        } else {
            if (dists.z < dists.x) {
                crosses.xyz = crosses.yzx;
                *pdists     = dists.yzx;
            } else {
                crosses.xy = crosses.yx;
                *pdists    = dists.yxz;
            }
        }
    } else {
        if (dists.y < dists.z) {
            *pdists = dists;
        } else {
            if (dists.z < dists.x) {
                crosses.xyz = crosses.zxy;
                *pdists     = dists.zxy;
            } else {
                crosses.yz = crosses.zy;
                *pdists    = dists.xzy;
            }
        }
    }
    ivec3 up = -(spt >= (vec_t)0.5);
    res.x    = (char)dot(convert_vec3(up), (vec3){1, 2, 4}) + 1;
    res.y    = opposite[res.x][crosses.x];
    res.z    = opposite[res.y][crosses.y];
    res.w    = opposite[res.z][crosses.z];
    return res - (char4)1;
}

/* https://stackoverflow.com/a/9493060 */
float
hue2rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

color3
hsl2rgb(color3 hsl) {
    float r, g, b;
    float h = hsl.x, s = hsl.y, l = hsl.z;
    if (s == 0) {
        r = g = b = l;
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r       = hue2rgb(p, q, h + 1.0f / 3.0f);
        g       = hue2rgb(p, q, h);
        b       = hue2rgb(p, q, h - 1.0f / 3.0f);
    }
    return new_color(r, g, b);
}

color3
rgb2hsl(color3 rgb) {
    float h, s, l;
    float r = rgb.x, g = rgb.y, b = rgb.z;
    float max = (r > g && r > b) ? r : (g > b) ? g : b;
    float min = (r < g && r < b) ? r : (g < b) ? g : b;

    l = (max + min) / 2.0f;

    if (max == min) {
        h = s = 0.0f;
    } else {
        float d = max - min;
        s       = (l > 0.5f) ? d / (2.0f - max - min) : d / (max + min);

        if (r > g && r > b) h = (g - b) / d + (g < b ? 6.0f : 0.0f);

        else if (g > b)
            h = (b - r) / d + 2.0f;

        else
            h = (r - g) / d + 4.0f;

        h /= 6.0f;
    }
    return new_color(h, s, l);
}

bool
voxel_march(Ray r, vec4 bounds, global Voxel *voxels, color3 *col, bool debug) {
    struct {
        vec4  bounds;
        char4 subd;
        vec3  dists;
        uint  voxel_index;
        uchar iter;
    } stack[VOXEL_DEPTH + 1];
    int   stack_index = 0;
    vec_t tmin, tmax;
    if (!hit_AABB(
            (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
            r,
            &tmin,
            &tmax)) {
        return false;
    }
    vec3 pt  = r.orig + tmin * r.dir;
    vec3 spt = (pt - bounds.xyz) / bounds.w;
    vec4 dists;
    stack[0].subd =
        box_subdivs(spt, r.invdir, (tmax - tmin) / bounds.w, &stack[0].dists);
    stack[0].voxel_index = 0;
    stack[0].bounds      = bounds;
    stack[0].iter        = 0;
    int count            = 0;
    do {
        count++;
        if (stack_index > VOXEL_DEPTH) {
            *col = new_color(1, 0, 0);
            return true;
        }

        char child_index        = stack[stack_index].subd.x;
        stack[stack_index].subd = stack[stack_index].subd.yzwx;

        if (stack[stack_index].iter == 4 || child_index == -1) {
            stack_index--;
            continue;
        }
        stack[stack_index].iter++;

        uint voxel_index = stack[stack_index].voxel_index;
        if (voxels[voxel_index].type == LEAF) {
            *col  = voxels[voxel_index].leaf.color.xyz;
            int G = count % 256;
            int R = count / 256 % 256;
            R     = (256 - R) % 256;
            int B = count / 256 / 256 % 256;
            B     = (256 - B) % 256;
            *col  = new_color(R / 255.0, G / 255.0, B / 255.0);
            return true;
        }

        uint child_voxel = voxels[voxel_index].internal.children[child_index];

        if (child_voxel == 0) { continue; }

        bounds = stack[stack_index].bounds;
        bounds.w /= 2;
        bounds.xyz += corner[child_index].xyz * bounds.w;

        stack[stack_index + 1].voxel_index = child_voxel;
        stack[stack_index + 1].bounds      = bounds;
        stack[stack_index + 1].iter        = 0;

        if (!hit_AABB(
                (vec3[]){bounds.xyz, bounds.xyz + bounds.w},
                r,
                &tmin,
                &tmax)) {
            continue;
        }
        pt                          = r.orig + tmin * r.dir;
        spt                         = (pt - bounds.xyz) / bounds.w;
        stack[stack_index + 1].subd = box_subdivs(
            spt,
            r.invdir,
            (tmax - tmin) / bounds.w,
            &stack[stack_index + 1].dists);

        stack_index++;
    } while (stack_index >= 0);
    return false;
}

color3
trace_ray(Ray r, global Voxel *voxels, bool debug) {
    vec4   bounds = {0, 0, 0, 1 << VOXEL_DEPTH};
    color3 col    = convert_color((r.dir + 1) / 2);
    voxel_march(r, bounds, voxels, &col, debug);
    if (debug) {
        color3 hsl = rgb2hsl(col);
        hsl.x      = fmod(hsl.x + 0.5f, 1.0f);
        hsl.z      = fmod(hsl.z + 0.5f, 1.0f);
        color3 rgb = hsl2rgb(hsl);
        return hsl2rgb(hsl);
    }
    return col;
}

kernel void
render(write_only image2d_t image, global vec4 cam[4], global Voxel *voxels) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX    = get_global_size(0);
    const uint resY    = get_global_size(1);
    bool       debug   = x_coord == resX / 2 && y_coord == resY / 2; /*
                  pow((float)x_coord - resX / 2, 2) + pow((float)y_coord - resY / 2, 2)
                  <=         16.0f;*/
    const vec3 origin =
        new_vec3(cam[0].z / cam[3].z, cam[1].z / cam[3].z, cam[2].z / cam[3].z);

    const vec3 ncp =
        mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, -1));
    const vec3 fcp =
        mul(cam,
            new_vec3(x_coord - (vec_t)resX / 2, y_coord - (vec_t)resY / 2, 1));
    const vec3 dir = normalize((fcp - ncp).xyz);
    Ray        r   = new_Ray(origin, dir);
    color3     col = trace_ray(r, voxels, debug);
    write_imagef(image, (int2){x_coord, y_coord}, (color4){col, 1});
}