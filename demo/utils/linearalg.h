// based on https://github.com/shua/jams/tree/master/ld26

typedef struct Matrix
{
    float m[16];
} Matrix;

/*
0 1 2 3
4 5 6 7
8 9 A B
C D E F
*/

typedef union Vector4
{
    float m[4];
    struct {
        float x, y, z, w;
    };
} Vector4;

/*
static const Vector4 X_AXIS = {{1, 0, 0, 0}};
static const Vector4 Y_AXIS = {{0, 1, 0, 0}};
static const Vector4 Z_AXIS = {{0, 0, 1, 0}};
static const Vector4 INV_X_AXIS = {{-1, 0, 0, 0}};
static const Vector4 INV_Y_AXIS = {{0, -1, 0, 0}};
static const Vector4 INV_Z_AXIS = {{0, 0, -1, 0}};
*/

Matrix multiplymat4(const Matrix* m1, const Matrix* m2);
Vector4 mulmatvec4(const Matrix* m, const Vector4* v);
void normalizevec4(Vector4* v);
float dotvec4(Vector4 v1, Vector4 v2);
Vector4 crossvec4(Vector4 v1, Vector4 v2);
void rotateX(Matrix* m, float angle);
void rotateY(Matrix* m, float angle);
void rotateZ(Matrix* m, float angle);
void scale(Matrix* m, float x, float y, float z);
void translate(Matrix* m, float x, float y, float z);

Matrix perspective(float fovy, float aspect_ratio, float near_plane,
    float far_plane);
Matrix orthogonal(float left, float right, float bottom, float top);

Matrix lookAt(Vector4 pos, Vector4 dir);
