#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "obj_parser.h"
#include "obj_scanner.h"

struct Model {
    Vector4 *verts;
    cl_int3 *tris;
    size_t vert_count;
    size_t tri_count;
    Vector3 min, max;
};

Model *
new_Model(void) {
    Model *model = malloc(sizeof(*model));
    if (model == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *model = (Model){
            new_vector(), new_vector(), 0, 0, Vector3_zero, Vector3_zero
    };
    return model;
}

void
delete_Model(Model *model) {
    delete_vector(model->verts);
    delete_vector(model->tris);
    free(model);
}

void
append_Model_vert(Model *model, Vector4 vert) {
    vector_append(model->verts, ((Vector4){
            {
                    (float)vert.s[0],
                    (float)vert.s[1],
                    (float)vert.s[2],
                    (float)vert.s[3]
            }
    }));
    model->vert_count++;
    if (model->vert_count == 1) {
        model->min = model->max = vert;
    }
    model->min = vec_min(model->min, vert);
    model->max = vec_max(model->max, vert);
}

void
append_Model_tri(Model *model, cl_int3 tri) {
    for (int i = 0; i < 3; i++) {
        if (tri.s[i] < 0) {
            tri.s[i] += model->vert_count;
        } else {
            tri.s[i] -= 1;
        }
    }
    vector_append(model->tris, tri);
    model->tri_count++;
}

Vector4 *
Model_verts(Model *model) {
    return model->verts;
}

cl_int3 *
Model_tris(Model *model) {
    return model->tris;
}

Vector3
Model_min(Model *model) {
    return model->min;
}

Vector3
Model_max(Model *model) {
    return model->max;
}

enum model_type {
    MODEL_OBJ, MODEL_NONE
};

struct filetype {
    const char *ext;
    enum model_type type;
} filetypes[] = {
        {
                ".obj", MODEL_OBJ
        }
};

static enum model_type
get_filetype(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext) {
        for (size_t i = 0; i < sizeof(filetypes) / sizeof(*filetypes); i++) {
            if (strcmp(ext, filetypes[i].ext) == 0) {
                return filetypes[i].type;
            }
        }
    }
    return MODEL_NONE;
}

static int
parse_OBJ(const char *filename, Model *model) {
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror(filename);
        return 1;
    }
    if (yylex_init(&scanner)) {
        fprintf(stderr, "could not initialize Flex scanner.\n");
        return 1;
    }
    state = yy_create_buffer(file, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);
    return yyparse(model, filename, scanner);
}

int
LoadModel(const char *filename, Model *model) {
    switch (get_filetype(filename)) {
        case MODEL_OBJ:
            return parse_OBJ(filename, model);
        default:
            fprintf(stderr, "Unrecognized filetype: \"%s\"\n", filename);
            fprintf(stderr, "Supported filetypes are: ");
            char *sep = "";
            for (size_t i = 0;
                    i < sizeof(filetypes) / sizeof(*filetypes);
                    i++) {
                fprintf(stderr, "%s\"%s\"", sep, filetypes[i].ext);
                sep = ", ";
            }
            fprintf(stderr, "\n");
            return 1;
    }
}
