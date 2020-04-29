#define double3(x, y, z) (double3){x, y, z}
#define double4(x, y, z, w) (double4){x, y, z, w}

typedef double4 matrix[4];

typedef struct __attribute__ ((packed)) Object {
    double3 position;
    enum {
        OBJ_SPHERE
    } type;
    union {
        struct {
            double radius;
        } sphere;
    };
} Object;

typedef struct Hit {
    double dist;
    __global Object *obj;
} Hit;

double3
mul(const matrix M, double3 X) {
    return double3(dot(M[0].xyz, X) + M[0].w,
        dot(M[1].xyz, X) + M[1].w,
        dot(M[2].xyz, X) + M[2].w) / (dot(M[3].xyz, X) + M[3].w);
}

double
mod(double a, double b) {
    return fmod(fmod(a, b) + b, b);
}

bool
solveQuadratic(double a, double b, double c, double *x0, double *x1) {
    double discr = b * b - 4 * a * c;
    if (discr < 0) {
        return false;
    } else if (discr == 0) {
        *x0 = *x1 = -0.5 * b / a;
    } else {
        double q = (b > 0)
            ? -0.5 * (b + sqrt(discr))
            : -0.5 * (b - sqrt(discr));
        *x0 = q / a;
        *x1 = c / q;
    }
    if (*x0 > *x1) {
        double tmp = *x0;
        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

bool
hit_sphere(double3 center, double radius2, double3 start, double3 dir, double *t) {
    double t0, t1;
    double3 L = start - center;
    double a = dot(dir, dir);
    double b = 2 * dot(dir, L);
    double c = dot(L, L) - radius2;
    if (c <= 0) return false;
    if (!solveQuadratic(a, b, c, &t0, &t1)) {
        return false;
    }
    if (t0 > t1) {
        double tmp = t0;
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
trace_ray(double3 start,
        double3 dir,
        global Object *objects,
        int objcount,
        int depth,
        double str,
        global Object *ignore) {
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
        double dist;
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
        double3 pos = start + minHit.dist * dir;
        double3 normal = normalize(pos - minHit.obj->position);
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
}

kernel void
render(write_only image2d_t image,
        global double4 cam[4],
        global struct Object *objects,
        int objcount) {
    const uint x_coord = get_global_id(0);
    const uint y_coord = get_global_id(1);
    const uint resX = get_global_size(0);
    const uint resY = get_global_size(1);

    const double3 origin =
        double3(cam[0].z / cam[3].z, cam[1].z / cam[3].z, cam[2].z / cam[3].z);
    const double3 ncp = mul(cam,
        double3(x_coord - (double)resX / 2, y_coord - (double)resY / 2, -1));
    const double3 fcp = mul(cam,
        double3(x_coord - (double)resX / 2, y_coord - (double)resY / 2, 1));
    const double3 dir = normalize((fcp - ncp).xyz);
    write_imagef(image, (int2){
        x_coord,
        y_coord
    }, (float4)
    {
        trace_ray(origin, dir, objects, objcount, 500, 1, NULL), 1.0
    }
    );
}
