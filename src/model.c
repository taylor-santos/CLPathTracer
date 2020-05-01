#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "obj_parser.h"
#include "obj_scanner.h"

struct Model {
    cl_double4 *verts;
    cl_int3 *tris;
    size_t vert_count;
    size_t tri_count;
};

Model
new_Model(void) {
    return (Model){
        new_vector(),
        new_vector(),
        0,
        0
    };
}

void
append_Model_vert(Model *model, cl_double4 vert) {
    vector_append(model->verts, vert);
    model->vert_count++;
}

void
append_Model_tri(Model *model, cl_int3 tri) {
    vector_append(model->tris, tri);
    model->tri_count++;
}

enum model_type {
    MODEL_OBJ, MODEL_NONE
};

struct filetype {
    const char *ext;
    enum model_type type;
} filetypes[] = {
    {
        ".obj",
        MODEL_OBJ
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
parse_OBJ(const char *filename) {
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
    clock_t start = clock();
    Model model = new_Model();
    if (yyparse(&model, filename, scanner)) {
        return 1;
    }
    clock_t end = clock();
    printf("%fs\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    return 0;
}

int
LoadModel(const char *filename) {

    switch (get_filetype(filename)) {
        case MODEL_OBJ:
            return parse_OBJ(filename);
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
