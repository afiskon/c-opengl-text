#include "linearalg.h"
#include "constants.h"
#include <string.h>
#include <math.h>

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

void
vectorNormalizeInplace(Vector* v)
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
vectorDot(Vector v1, Vector v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector
vectorCross(Vector v1, Vector v2)
{
    Vector out = {{
        v1.y*v2.z - v1.z*v2.y,
        v1.z*v2.x - v1.x*v2.z,
        v1.x*v2.y - v1.y*v2.x,
        0.0f
    }};
    return out;
}

Matrix
matrixIdentity()
{
    Matrix out = {{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    }};

    return out;
}

Matrix
matrixPerspective(float fovy, float aspect, float zNear, float zFar)
{
    Matrix out = {{ 0 }};

    // range = tan(radians(fovy / 2)) * zNear;
    float range = tan(M_PI * (fovy / (2.0f * 180.f))) * zNear;    
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;

    out.m[0*4 + 0] = (2.0f * zNear) / (right - left);
    out.m[1*4 + 1] = (2.0f * zNear) / (top - bottom);
    out.m[2*4 + 2] = - (zFar + zNear) / (zFar - zNear);
    out.m[2*4 + 3] = - 1.0f;
    out.m[3*4 + 2] = - (2.0f * zFar * zNear) / (zFar - zNear);
    
    return out;
}

Matrix
matrixOrthogonal(float left, float right, float bottom, float top)
{
    Matrix out = matrixIdentity();
    out.m[0*4 + 0] = 2 / (right - left);
    out.m[1*4 + 1] = 2 / (top - bottom);
    out.m[2*4 + 2] = -1;
    out.m[3*4 + 0] = - (right + left) / (right - left);
    out.m[3*4 + 1] = - (top + bottom) / (top - bottom);

    return out;
}

// WARNING! What in GLM is m1 * m2 is multiplymat4(m2, m1)
Matrix
multiplymat4(const Matrix* m1, const Matrix* m2)
{
    Matrix out = matrixIdentity();
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
    // TODO: use .x, .y, .z
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

Matrix
rotate(const Matrix* in, float angle, float axis_x, float axis_y, float axis_z)
{
    float a = M_PI * (angle / 180.0f);
    float c = cos(a);
    float s = sin(a);

    Vector axis = {{ axis_x, axis_y, axis_z, 0.0f }};
    vectorNormalizeInplace(&axis);

    Vector temp = vectorMul(axis, (1.0f - c));
    Matrix rotate = {{ 0 }};

    // TODO: use .x, .y, .z
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
    Matrix scale = matrixIdentity();

    // TODO: use 4*i + j
    scale.m[0] = x;
    scale.m[5] = y;
    scale.m[10] = z;

    memcpy(m->m, multiplymat4(m, &scale).m, sizeof(m->m));
}

void
translate(Matrix* m, float x, float y, float z)
{
    Matrix translation = matrixIdentity();
    
    // TODO: use 4*i + j
    translation.m[12] = x;
    translation.m[13] = y;
    translation.m[14] = z;

    memcpy(m->m, multiplymat4(m, &translation).m, sizeof(m->m));
}

Matrix
lookAt(Vector eye, Vector center, Vector up)
{
    Vector f = vectorAdd(center, vectorMul(eye, -1.0f));
    vectorNormalizeInplace(&f);

    Vector u = up;
    vectorNormalizeInplace(&u);

    Vector s = vectorCross(f, u);
    vectorNormalizeInplace(&s);
    u = vectorCross(s, f);

    Matrix out = matrixIdentity();

    // TODO: use 4*i + j
    out.m[0] = s.x;
    out.m[4] = s.y;
    out.m[8] = s.z;

    out.m[1] = u.x;
    out.m[5] = u.y;
    out.m[9] = u.z;

    out.m[2] = -f.x;
    out.m[6] = -f.y;
    out.m[10] = -f.z;

    out.m[12] = -vectorDot(s, eye);
    out.m[13] = -vectorDot(u, eye);
    out.m[14] =  vectorDot(f, eye);
    return out;
}