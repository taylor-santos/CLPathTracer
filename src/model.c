#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vector.h"
#include "model.h"
#include "list.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

enum model_type { MODEL_OBJ, MODEL_KD, MODEL_NONE };

struct filetype {
    const char *    ext;
    enum model_type type;
} filetypes[] = {{".obj", MODEL_OBJ}, {".kd", MODEL_KD}};

static enum model_type
get_filetype(const char *filename, char **path) {
    const char *ext = strrchr(filename, '.');
    if (ext) {
        if (path) {
            *path = calloc(ext - filename + 1, 1);
            strncpy(*path, filename, ext - filename);
        }
        for (size_t i = 0; i < sizeof(filetypes) / sizeof(*filetypes); i++) {
            if (strcmp(ext, filetypes[i].ext) == 0) {
                return filetypes[i].type;
            }
        }
    }
    return MODEL_NONE;
}

static size_t
file_length(FILE *file) {
    long int length, prev_pos;

    prev_pos = ftell(file);
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    if ((length = ftell(file)) < 0) {
        perror("ftell");
        exit(EXIT_FAILURE);
    }
    if (fseek(file, prev_pos, SEEK_SET) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    return length;
}

static void
read_file(char *buffer, size_t file_len, FILE *file) {
    buffer[file_len] = '\0';
    if (fread(buffer, 1, file_len, file) != file_len && ferror(file) != 0) {
        fprintf(stderr, "error reading from file\n");
        exit(EXIT_FAILURE);
    }
}

static int
tinyOBJ_parse(const char *filename, const char *path, kd *tree) {
    printf("Parsing OBJ file...\n");
    clock_t start = clock();
    FILE *  file  = fopen(filename, "r");
    if (file == NULL) {
        perror(filename);
        return 1;
    }
    size_t len         = file_length(file);
    char * file_buffer = malloc(len + 1);
    if (file_buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    read_file(file_buffer, len, file);
    fclose(file);
    unsigned int        flags = TINYOBJ_FLAG_TRIANGULATE;
    tinyobj_attrib_t    attrib;
    tinyobj_shape_t *   shapes = NULL;
    size_t              num_shapes;
    tinyobj_material_t *materials = NULL;
    size_t              num_materials;
    int                 ret = tinyobj_parse_obj(
        &attrib,
        &shapes,
        &num_shapes,
        &materials,
        &num_materials,
        file_buffer,
        len,
        flags);
    free(file_buffer);
    if (ret != TINYOBJ_SUCCESS) { return 1; }
    Vector3 *verts = new_list(sizeof(*verts) * attrib.num_vertices);
    for (unsigned int i = 0; i < attrib.num_vertices; i++) {
        vector_append(
            verts,
            Vector3(
                attrib.vertices[3 * i + 0],
                attrib.vertices[3 * i + 1],
                attrib.vertices[3 * i + 2]));
    }
    cl_int3 *tris = new_list(sizeof(*tris) * attrib.num_faces);
    for (unsigned int i = 0; i < attrib.num_faces; i++) {
        vector_append(
            tris,
            ((cl_int3){
                {attrib.faces[i].v_idx,
                 attrib.faces[i].vn_idx,
                 attrib.faces[i].vt_idx}}));
    }
    Vector3 *norms = new_list(sizeof(*norms) * attrib.num_normals);
    for (unsigned int i = 0; i < attrib.num_normals; i++) {
        vector_append(
            norms,
            Vector3(
                attrib.normals[3 * i + 0],
                attrib.normals[3 * i + 1],
                attrib.normals[3 * i + 2]));
    }
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
    clock_t end = clock();
    printf(
        "OBJ file parsed in %ld ms. Building kd-tree...\n",
        (end - start) * 1000 / CLOCKS_PER_SEC);
    start = clock();
    *tree = build_kd(tris, verts, norms, path);
    end   = clock();
    printf("kd-tree built in %ld ms.\n", (end - start) * 1000 / CLOCKS_PER_SEC);
    return 0;
}

int
LoadModel(const char *filename, kd *tree) {
    char *path = NULL;
    switch (get_filetype(filename, &path)) {
        case MODEL_OBJ:;
            int ret;
            if (tinyOBJ_parse(filename, path, tree)) {
                ret = 1;
            } else {
                ret = 0;
            }
            free(path);
            return ret;
        case MODEL_KD: return parse_kd(filename, tree);
        default:
            fprintf(stderr, "Unrecognized filetype: \"%s\"\n", filename);
            fprintf(stderr, "Supported filetypes are: ");
            char *sep = "";
            for (size_t i = 0; i < sizeof(filetypes) / sizeof(*filetypes);
                 i++) {
                fprintf(stderr, "%s\"%s\"", sep, filetypes[i].ext);
                sep = ", ";
            }
            fprintf(stderr, "\n");
            free(path);
            return 1;
    }
}
