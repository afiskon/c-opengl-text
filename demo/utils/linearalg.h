#ifndef AFISKON_LINEARALG_H
#define AFISKON_LINEARALG_H

// based on https://github.com/shua/jams/tree/master/ld26 and GLM

typedef struct Matrix
{
    float m[16];
} Matrix;

// TODO: refactor interface: always return Vector/Matrix or never return

// TODO: procedures refactorings
typedef union Vector
{
    float m[4];
    struct {
        float x, y, z, w;
    };
} Vector;

Vector addvec4(Vector v1, Vector v2);
Vector mulvec4(Vector v, float n);
Matrix identitymat();
Matrix multiplymat4(const Matrix* m1, const Matrix* m2);
Vector mulmatvec4(const Matrix* m, const Vector* v);
void normalizevec4(Vector* v);
float dotvec4(Vector v1, Vector v2);
Vector crossvec4(Vector v1, Vector v2);
//void rotateX(Matrix* m, float angle);
//void rotateY(Matrix* m, float angle);
//void rotateZ(Matrix* m, float angle);
Matrix rotate(const Matrix* m, float angle,
	float axis_x, float axis_y, float axis_z);
void scale(Matrix* m, float x, float y, float z);
void translate(Matrix* m, float x, float y, float z);

Matrix perspective(float fovy, float aspect, float zNear, float zFar);
Matrix orthogonal(float left, float right, float bottom, float top);

Matrix lookAt(Vector eye, Vector center, Vector up);

#endif // AFISKON_LINEARALG_H