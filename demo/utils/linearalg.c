#include "linearalg.h"
#include "constants.h"
#include <string.h>
#include <math.h>

static const Matrix IDENTITY_MATRIX =
{{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
}};

Vector
vectorNull()
{
    Vector res = {{ 0 }};
    return res;
}

Vector
vectorAdd(Vector v1, Vector v2)
{
    Vector res;

    res.x = v1.x + v2.x;
    res.y = v1.y + v2.y;
    res.z = v1.z + v2.z;
    res.w = v1.w + v2.w;

    return res;
}

Vector
vectorMul(Vector v, float n)
{
    Vector res = v;

    res.x *= n;
    res.y *= n;
    res.z *= n;
    res.w *= n;

    return res;
}

Matrix
identitymat()
{
    Matrix out = IDENTITY_MATRIX;
    return out;
}

// WARNING! What in GLM is m1 * m2 is multiplymat4(m2, m1)
Matrix
multiplymat4(const Matrix* m1, const Matrix* m2)
{
    Matrix out = IDENTITY_MATRIX;
    unsigned int row, column, row_offset;

    for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
        for (column = 0; column < 4; ++column)
            out.m[row_offset + column] =
                (m1->m[row_offset + 0] * m2->m[column + 0]) +
                (m1->m[row_offset + 1] * m2->m[column + 4]) +
                (m1->m[row_offset + 2] * m2->m[column + 8]) +
                (m1->m[row_offset + 3] * m2->m[column + 12]);

    return out;
}

Vector
mulmatvec4(const Matrix* m, const Vector* v)
{
    Vector out;
    for(int i = 0; i < 4; ++i) {
        out.m[i] =
            (v->m[0] * m->m[i + 0]) +
            (v->m[1] * m->m[i + 4]) +
            (v->m[2] * m->m[i + 8]) +
            (v->m[3] * m->m[i + 12]);
    }

    return out;
}

void
normalizevec4(Vector* v)
{
    float sqr = v->m[0] * v->m[0] + v->m[1] * v->m[1] + v->m[2] * v->m[2];
    if(sqr == 1 || sqr == 0)
        return;

    float invrt = 1.f/sqrt(sqr);
    v->m[0] *= invrt;
    v->m[1] *= invrt;
    v->m[2] *= invrt;
}

float
dotvec4(Vector v1, Vector v2)
{
    return v1.m[0] * v2.m[0] + v1.m[1] * v2.m[1] + v1.m[2] * v2.m[2] +
        v1.m[3] * v2.m[3];
}

Vector
crossvec4(Vector v1, Vector v2)
{
    Vector out = {{ 0 }};
    out.m[0] = v1.m[1]*v2.m[2] - v1.m[2]*v2.m[1];
    out.m[1] = v1.m[2]*v2.m[0] - v1.m[0]*v2.m[2];
    out.m[2] = v1.m[0]*v2.m[1] - v1.m[1]*v2.m[0];
    return out;
}

Matrix
rotate(const Matrix* in, float angle, float axis_x, float axis_y, float axis_z)
{
    float a = M_PI * (angle / 180.0f);
    float c = cos(a);
    float s = sin(a);

    Vector axis = {{ axis_x, axis_y, axis_z, 0.0f }};
    normalizevec4(&axis);

    Vector temp = vectorMul(axis, (1.0f - c));
    Matrix rotate = {{ 0 }};

    rotate.m[0*4 + 0] = c + temp.m[0] * axis.m[0];
    rotate.m[0*4 + 1] = 0 + temp.m[0] * axis.m[1] + s * axis.m[2];
    rotate.m[0*4 + 2] = 0 + temp.m[0] * axis.m[2] - s * axis.m[1];

    rotate.m[1*4 + 0] = 0 + temp.m[1] * axis.m[0] - s * axis.m[2];
    rotate.m[1*4 + 1] = c + temp.m[1] * axis.m[1];
    rotate.m[1*4 + 2] = 0 + temp.m[1] * axis.m[2] + s * axis.m[0];

    rotate.m[2*4 + 0] = 0 + temp.m[2] * axis.m[0] + s * axis.m[1];
    rotate.m[2*4 + 1] = 0 + temp.m[2] * axis.m[1] - s * axis.m[0];
    rotate.m[2*4 + 2] = c + temp.m[2] * axis.m[2];

    Vector r[4], m[4];
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            m[i].m[j] = in->m[i*4 + j];

    for(int i = 0; i <= 2; i++)
        r[i] = vectorAdd( vectorAdd(
                    vectorMul(m[0], rotate.m[i*4 + 0]),
                    vectorMul(m[1], rotate.m[i*4 + 1])
                 ), vectorMul(m[2], rotate.m[i*4 + 2]));
    r[3] = m[3];

    Matrix result = {{ 0 }};
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            result.m[i*4 + j] = r[i].m[j];

    return result;
}

void
scale(Matrix* m, float x, float y, float z)
{
    Matrix scale = IDENTITY_MATRIX;

    scale.m[0] = x;
    scale.m[5] = y;
    scale.m[10] = z;

    memcpy(m->m, multiplymat4(m, &scale).m, sizeof(m->m));
}

void
translate(Matrix* m, float x, float y, float z)
{
    Matrix translation = IDENTITY_MATRIX;
    
    translation.m[12] = x;
    translation.m[13] = y;
    translation.m[14] = z;

    memcpy(m->m, multiplymat4(m, &translation).m, sizeof(m->m));
}

Matrix
perspective(float fovy, float aspect, float zNear, float zFar)
{
    Matrix out = {{ 0 }};

    // range = tan(radians(fovy / 2)) * zNear;
    float range = tan(M_PI * (fovy / (2.0f * 180.f))) * zNear;    
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;

    out.m[0] = (2.0f * zNear) / (right - left);
    out.m[5] = (2.0f * zNear) / (top - bottom);
    out.m[10] = - (zFar + zNear) / (zFar - zNear);
    out.m[11] = - 1.0f;
    out.m[14] = - (2.0f * zFar * zNear) / (zFar - zNear);
    
    return out;
}

Matrix
orthogonal(float left, float right, float bottom, float top)
{
    Matrix out = IDENTITY_MATRIX;
    out.m[0] = 2 / (right - left);
    out.m[5] = 2 / (top - bottom);
    out.m[10] = -1;
    out.m[12] = - (right + left) / (right - left);
    out.m[13] = - (top + bottom) / (top - bottom);

    return out;
}

Matrix
lookAt(Vector eye, Vector center, Vector up)
{
    Vector f = vectorAdd(center, vectorMul(eye, -1.0f));
    normalizevec4(&f);

    Vector u = up;
    normalizevec4(&u);

    Vector s = crossvec4(f, u);
    normalizevec4(&s);
    u = crossvec4(s, f);

    Matrix out = IDENTITY_MATRIX;
    out.m[0] = s.x;
    out.m[4] = s.y;
    out.m[8] = s.z;

    out.m[1] = u.x;
    out.m[5] = u.y;
    out.m[9] = u.z;

    out.m[2] = -f.x;
    out.m[6] = -f.y;
    out.m[10] = -f.z;

    out.m[12] = -dotvec4(s, eye);
    out.m[13] = -dotvec4(u, eye);
    out.m[14] =  dotvec4(f, eye);
    return out;
}