#ifndef AFISKON_LINEARALG_H
#define AFISKON_LINEARALG_H

/*
More algorithms could be found here:
- https://github.com/Groovounet/glm
- https://github.com/shua/jams/tree/master/ld26
*/

typedef struct Matrix
{
    float m[16];
} Matrix;

typedef union Vector
{
    float m[4];
    struct {
        float x, y, z, w;
    };
} Vector;

Vector vectorNull();

// Other possible constructors: vectorXAxis, vectorYAxis, vectorZAxis

Vector addvec4(Vector v1, Vector v2);
Vector mulvec4(Vector v, float n);

// TODO: rename to ...Inplace
void normalizevec4(Vector* v);
float dotvec4(Vector v1, Vector v2);
Vector crossvec4(Vector v1, Vector v2);

Matrix identitymat();
Matrix perspective(float fovy, float aspect, float zNear, float zFar);
Matrix orthogonal(float left, float right, float bottom, float top);
Matrix lookAt(Vector eye, Vector center, Vector up);

Vector mulmatvec4(const Matrix* m, const Vector* v);
Matrix multiplymat4(const Matrix* m1, const Matrix* m2);

Matrix rotate(const Matrix* m, float angle,
	float axis_x, float axis_y, float axis_z);

// TODO: rename to ...Inplace
void scale(Matrix* m, float x, float y, float z);

// TODO: rename to ...Inplace
void translate(Matrix* m, float x, float y, float z);

#endif // AFISKON_LINEARALG_H