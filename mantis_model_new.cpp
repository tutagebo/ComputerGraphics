// mantis_model_new.cpp
// mantis_model_old.cpp を参考に、添付画像の特徴を強めたカマキリモデル。
// - 細長い前胸、三角気味の頭、大きい複眼
// - 葉のような半透明の翅と葉脈
// - 折りたたんだ鎌状前脚と内側のトゲ
// - 長い中脚・後脚、細い触角、枝に止まる構図

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Vec3 {
    double x;
    double y;
    double z;
};

Vec3 V(double x, double y, double z)
{
    Vec3 v = {x, y, z};
    return v;
}

Vec3 add(Vec3 a, Vec3 b) { return V(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec3 sub(Vec3 a, Vec3 b) { return V(a.x - b.x, a.y - b.y, a.z - b.z); }
Vec3 mul(Vec3 a, double s) { return V(a.x * s, a.y * s, a.z * s); }

Vec3 lerp(Vec3 a, Vec3 b, double t)
{
    return add(mul(a, 1.0 - t), mul(b, t));
}

double dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(Vec3 a, Vec3 b)
{
    return V(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

double len(Vec3 a)
{
    return sqrt(dot(a, a));
}

Vec3 normalize(Vec3 a)
{
    double l = len(a);
    if (l < 1.0e-8) return V(0.0, 0.0, 0.0);
    return mul(a, 1.0 / l);
}

static double rotX = 58.0;
static double rotY = 0.0;
static double rotZ = -28.0;
static double zoom = 1.0;
static int lastX = 0;
static int lastY = 0;
static int dragging = 0;

void setMaterial(double r, double g, double b, double a, double shininess)
{
    GLfloat diffuse[] = {
        (GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a
    };
    GLfloat ambient[] = {
        (GLfloat)(r * 0.34), (GLfloat)(g * 0.34), (GLfloat)(b * 0.34), (GLfloat)a
    };
    GLfloat specular[] = {
        0.22f, 0.24f, 0.16f, 1.0f
    };

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (GLfloat)shininess);
    glColor4d(r, g, b, a);
}

void rotateZAxisTo(Vec3 dir)
{
    Vec3 z = V(0.0, 0.0, 1.0);
    Vec3 d = normalize(dir);
    double c = dot(z, d);

    if (c > 0.9999) return;

    if (c < -0.9999) {
        glRotated(180.0, 1.0, 0.0, 0.0);
        return;
    }

    Vec3 axis = normalize(cross(z, d));
    double angle = acos(c) * 180.0 / M_PI;
    glRotated(angle, axis.x, axis.y, axis.z);
}

void drawCylinderBetween(Vec3 a, Vec3 b,
                         double r0, double r1,
                         double cr, double cg, double cb)
{
    Vec3 d = sub(b, a);
    double h = len(d);
    if (h < 1.0e-7) return;

    setMaterial(cr, cg, cb, 1.0, 28.0);

    GLUquadric *q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);

    glPushMatrix();
    glTranslated(a.x, a.y, a.z);
    rotateZAxisTo(d);
    gluCylinder(q, r0, r1, h, 20, 4);
    glPopMatrix();

    gluDeleteQuadric(q);
}

void drawConeBetween(Vec3 a, Vec3 b, double r,
                     double cr, double cg, double cb)
{
    drawCylinderBetween(a, b, r, 0.0, cr, cg, cb);
}

void drawEllipsoid(Vec3 p,
                   double sx, double sy, double sz,
                   double r, double g, double b, double a)
{
    setMaterial(r, g, b, a, 34.0);

    glPushMatrix();
    glTranslated(p.x, p.y, p.z);
    glScaled(sx, sy, sz);
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();
}

void drawBezierLine(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
                    double r, double g, double b, double width)
{
    glDisable(GL_LIGHTING);
    glColor3d(r, g, b);
    glLineWidth((GLfloat)width);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 48; i++) {
        double t = (double)i / 48.0;
        double u = 1.0 - t;
        Vec3 p = add(
            add(mul(p0, u * u * u), mul(p1, 3.0 * u * u * t)),
            add(mul(p2, 3.0 * u * t * t), mul(p3, t * t * t))
        );
        glVertex3d(p.x, p.y, p.z);
    }
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void drawLine(Vec3 a, Vec3 b, double r, double g, double bl, double width)
{
    glDisable(GL_LIGHTING);
    glColor3d(r, g, bl);
    glLineWidth((GLfloat)width);
    glBegin(GL_LINES);
    glVertex3d(a.x, a.y, a.z);
    glVertex3d(b.x, b.y, b.z);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void drawJoint(Vec3 p, double r)
{
    drawEllipsoid(p, r, r, r, 0.48, 0.68, 0.22, 1.0);
}

void drawBranch()
{
    Vec3 a = V(-4.9, -0.45, -1.18);
    Vec3 b = V(4.7, 0.32, -1.02);

    drawCylinderBetween(a, b, 0.20, 0.18, 0.42, 0.28, 0.12);
    drawCylinderBetween(V(-1.9, -0.22, -1.10), V(-2.8, 0.65, -0.75),
                        0.08, 0.02, 0.32, 0.21, 0.10);
    drawCylinderBetween(V(1.9, 0.07, -1.05), V(2.9, -0.62, -0.67),
                        0.07, 0.02, 0.32, 0.21, 0.10);

    for (int i = -4; i <= 4; i++) {
        double x = (double)i;
        drawLine(V(x, -0.30 + 0.06 * i, -0.96),
                 V(x - 0.25, -0.43 + 0.06 * i, -1.22),
                 0.22, 0.13, 0.06, 1.0);
    }
}

double abdomenBackZ(double t)
{
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    double body = sin(M_PI * t);
    double centerZ = 0.09 - 0.12 * t + 0.035 * sin(M_PI * t);
    double topZ = (0.15 + 0.33 * body) * (1.0 - 0.12 * t);
    return centerZ + topZ;
}

void wingPoint(int side, double t, Vec3 *inner, Vec3 *outer)
{
    double s = (double)side;
    double x = 0.02 + 2.82 * t;
    double z = abdomenBackZ(t) + 0.025;
    double halfWidth = 0.05 + 0.50 * sin(M_PI * t) * (1.0 - 0.08 * t);

    *inner = V(x, s * 0.035, z + 0.025);
    *outer = V(x - 0.06 * t, s * halfWidth, z - 0.015 + 0.015 * sin(M_PI * t));
}

Vec3 wingSurfacePoint(int side, double t, double u)
{
    Vec3 inner, outer;
    wingPoint(side, t, &inner, &outer);

    Vec3 p = lerp(inner, outer, u);

    // 幅方向に丸みをつけ、内側は背中のラインに沿わせる。
    double crossCurve = sin(M_PI * u);
    double lengthCurve = 0.35 + 0.65 * sin(M_PI * t);
    p.z += 0.045 * crossCurve * lengthCurve;

    // 頭からおしりへの軸まわりに外側を倒し、腹部の丸みに沿わせる。
    double axisZ = inner.z;
    double angle = -side * (M_PI / 180.0) * 18.0 * u;
    double dy = p.y;
    double dz = p.z - axisZ;
    p.y = dy * cos(angle) - dz * sin(angle);
    p.z = axisZ + dy * sin(angle) + dz * cos(angle);

    return p;
}

void drawWing(int side)
{
    const int N = 56;
    const int W = 7;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    setMaterial(0.66, 0.90, 0.36, 0.92, 18.0);

    for (int w = 0; w < W; w++) {
        double u0 = (double)w / (double)W;
        double u1 = (double)(w + 1) / (double)W;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= N; i++) {
            double t = (double)i / (double)N;
            Vec3 p0 = wingSurfacePoint(side, t, u0);
            Vec3 p1 = wingSurfacePoint(side, t, u1);

            glNormal3d(0.0, 0.0, 1.0);
            glVertex3d(p0.x, p0.y, p0.z);
            glVertex3d(p1.x, p1.y, p1.z);
        }
        glEnd();
    }

    glDisable(GL_LIGHTING);
    glColor4d(0.23, 0.43, 0.12, 0.82);
    glLineWidth(1.4f);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= N; i++) {
        Vec3 inner = wingSurfacePoint(side, (double)i / (double)N, 0.0);
        glVertex3d(inner.x, inner.y, inner.z + 0.006);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= N; i++) {
        Vec3 outer = wingSurfacePoint(side, (double)i / (double)N, 1.0);
        glVertex3d(outer.x, outer.y, outer.z + 0.006);
    }
    glEnd();

    for (int i = 6; i < N - 2; i += 5) {
        double t = (double)i / (double)N;
        Vec3 veinRoot = wingSurfacePoint(side, t, 0.18);
        Vec3 veinTip = wingSurfacePoint(side, t, 1.0);

        glBegin(GL_LINES);
        glVertex3d(veinRoot.x, veinRoot.y, veinRoot.z + 0.008);
        glVertex3d(veinTip.x, veinTip.y, veinTip.z + 0.008);
        glEnd();
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void drawAbdomen()
{
    const int ribs = 18;

    for (int i = 0; i < ribs; i++) {
        double t = (double)i / (double)(ribs - 1);
        double x = 0.02 + 2.80 * t;
        double body = sin(M_PI * t);
        double sx = 0.13;
        double sy = (0.12 + 0.30 * body) * (1.0 - 0.18 * t);
        double sz = (0.15 + 0.33 * body) * (1.0 - 0.12 * t);
        double z = 0.09 - 0.12 * t + 0.035 * sin(M_PI * t);

        drawEllipsoid(V(x, 0.0, z), sx, sy, sz, 0.52, 0.74, 0.26, 1.0);
    }

    glDisable(GL_LIGHTING);
    glColor3d(0.18, 0.36, 0.09);
    glLineWidth(1.1f);

    for (int k = 1; k < ribs - 1; k++) {
        double t = (double)k / (double)(ribs - 1);
        double x = 0.02 + 2.80 * t;
        double body = sin(M_PI * t);
        double sy = (0.12 + 0.30 * body) * (1.0 - 0.18 * t);
        double sz = (0.15 + 0.33 * body) * (1.0 - 0.12 * t);
        double z = 0.09 - 0.12 * t + 0.035 * sin(M_PI * t);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 40; i++) {
            double a = 2.0 * M_PI * i / 40.0;
            glVertex3d(x, sy * cos(a), z + sz * sin(a));
        }
        glEnd();
    }

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 42; i++) {
        double t = (double)i / 42.0;
        glVertex3d(-0.10 + 3.0 * t, 0.0, 0.45 - 0.16 * t + 0.04 * sin(M_PI * t));
    }
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void drawPentagonHead()
{
    setMaterial(0.52, 0.76, 0.31, 1.0, 32.0);

    Vec3 front[5] = {
        V(-2.02,  0.28, 0.75),
        V(-2.02, -0.28, 0.75),
        V(-2.19, -0.20, 0.54),
        V(-2.30,  0.00, 0.43),
        V(-2.19,  0.20, 0.54)
    };
    Vec3 back[5] = {
        V(-1.82,  0.22, 0.69),
        V(-1.82, -0.22, 0.69),
        V(-1.98, -0.16, 0.52),
        V(-2.06,  0.00, 0.44),
        V(-1.98,  0.16, 0.52)
    };

    glBegin(GL_POLYGON);
    glNormal3d(-1.0, 0.0, 0.25);
    for (int i = 0; i < 5; i++) {
        glVertex3d(front[i].x, front[i].y, front[i].z);
    }
    glEnd();

    glBegin(GL_POLYGON);
    glNormal3d(1.0, 0.0, 0.25);
    for (int i = 4; i >= 0; i--) {
        glVertex3d(back[i].x, back[i].y, back[i].z);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (int i = 0; i < 5; i++) {
        int j = (i + 1) % 5;
        glNormal3d(0.0, 0.0, 1.0);
        glVertex3d(front[i].x, front[i].y, front[i].z);
        glVertex3d(front[j].x, front[j].y, front[j].z);
        glVertex3d(back[j].x, back[j].y, back[j].z);
        glVertex3d(back[i].x, back[i].y, back[i].z);
    }
    glEnd();

    drawEllipsoid(V(-2.22, 0.0, 0.455), 0.09, 0.08, 0.055, 0.94, 0.74, 0.36, 1.0);

    drawLine(V(-2.17, 0.19, 0.55), V(-2.31, 0.04, 0.41), 0.04, 0.03, 0.02, 2.0);
    drawLine(V(-2.17, -0.19, 0.55), V(-2.31, -0.04, 0.41), 0.04, 0.03, 0.02, 2.0);
}

void drawThoraxAndHead()
{
    drawCylinderBetween(V(-1.55, 0.0, 0.48), V(0.22, 0.0, 0.16),
                        0.13, 0.19, 0.46, 0.70, 0.24);
    drawEllipsoid(V(-0.72, 0.0, 0.34), 0.52, 0.18, 0.24, 0.54, 0.76, 0.30, 1.0);
    drawEllipsoid(V(-1.62, 0.0, 0.50), 0.22, 0.16, 0.18, 0.42, 0.64, 0.20, 1.0);

    drawCylinderBetween(V(-1.72, 0.0, 0.54), V(-1.93, 0.0, 0.58),
                        0.105, 0.13, 0.46, 0.70, 0.24);
    drawEllipsoid(V(-1.86, 0.0, 0.57), 0.15, 0.12, 0.12, 0.48, 0.70, 0.24, 1.0);

    drawPentagonHead();

    drawEllipsoid(V(-2.03, 0.30, 0.69), 0.17, 0.11, 0.15, 0.72, 0.90, 0.38, 1.0);
    drawEllipsoid(V(-2.03, -0.30, 0.69), 0.17, 0.11, 0.15, 0.72, 0.90, 0.38, 1.0);
    drawEllipsoid(V(-2.14, 0.31, 0.68), 0.035, 0.024, 0.034, 0.08, 0.10, 0.04, 1.0);
    drawEllipsoid(V(-2.14, -0.31, 0.68), 0.035, 0.024, 0.034, 0.08, 0.10, 0.04, 1.0);

    drawBezierLine(V(-2.14, 0.08, 0.80), V(-2.55, 0.42, 1.28),
                   V(-3.05, 0.80, 1.58), V(-3.58, 1.10, 1.44),
                   0.30, 0.18, 0.07, 1.7);
    drawBezierLine(V(-2.14, -0.08, 0.80), V(-2.55, -0.42, 1.28),
                   V(-3.05, -0.80, 1.58), V(-3.58, -1.10, 1.44),
                   0.30, 0.18, 0.07, 1.7);

    drawLine(V(-2.25, 0.05, 0.50), V(-2.38, 0.11, 0.36), 0.14, 0.09, 0.04, 1.2);
    drawLine(V(-2.25, -0.05, 0.50), V(-2.38, -0.11, 0.36), 0.14, 0.09, 0.04, 1.2);
}

void drawSpines(Vec3 a, Vec3 b, int side, int count, double size)
{
    double s = (double)side;

    for (int i = 1; i <= count; i++) {
        double t = (double)i / (double)(count + 1);
        Vec3 base = lerp(a, b, t);
        Vec3 tip = add(base, V(-0.05, -0.10 * s, -size));
        drawConeBetween(base, tip, size * 0.16, 0.78, 0.82, 0.43);
    }
}

void drawForeLeg(int side)
{
    double s = (double)side;
    Vec3 shoulder = V(-1.27, 0.16 * s, 0.32);
    Vec3 upper = V(-1.72, 0.48 * s, -0.08);
    Vec3 femurEnd = V(-2.48, 0.68 * s, -0.74);
    Vec3 tibiaEnd = V(-2.34, 0.28 * s, -1.24);
    Vec3 claw = V(-2.54, 0.12 * s, -1.55);

    drawCylinderBetween(shoulder, upper, 0.055, 0.075, 0.48, 0.70, 0.22);
    drawCylinderBetween(upper, femurEnd, 0.15, 0.09, 0.57, 0.78, 0.30);
    drawCylinderBetween(femurEnd, tibiaEnd, 0.070, 0.032, 0.52, 0.74, 0.25);
    drawConeBetween(tibiaEnd, claw, 0.024, 0.33, 0.16, 0.05);

    drawJoint(shoulder, 0.065);
    drawJoint(upper, 0.075);
    drawJoint(femurEnd, 0.068);
    drawJoint(tibiaEnd, 0.045);

    drawSpines(upper, femurEnd, side, 8, 0.12);
    drawSpines(femurEnd, tibiaEnd, side, 5, 0.09);
}

void drawWalkingLeg(int side, int rear)
{
    double s = (double)side;
    Vec3 a, b, c, d, foot;

    if (rear) {
        a = V(1.28, 0.22 * s, -0.12);
        b = V(1.95, 0.72 * s, -0.44);
        c = V(2.62, 1.34 * s, -1.10);
        d = V(3.00, 1.10 * s, -1.50);
        foot = V(3.34, 0.92 * s, -1.54);
    } else {
        a = V(0.16, 0.20 * s, 0.05);
        b = V(0.72, 0.58 * s, -0.34);
        c = V(1.15, 0.94 * s, -1.04);
        d = V(1.40, 0.72 * s, -1.47);
        foot = V(1.74, 0.58 * s, -1.51);
    }

    drawCylinderBetween(a, b, 0.050, 0.038, 0.43, 0.65, 0.20);
    drawCylinderBetween(b, c, 0.038, 0.024, 0.55, 0.73, 0.26);
    drawCylinderBetween(c, d, 0.024, 0.014, 0.50, 0.68, 0.22);
    drawCylinderBetween(d, foot, 0.014, 0.006, 0.34, 0.44, 0.13);

    drawJoint(a, 0.048);
    drawJoint(b, 0.040);
    drawJoint(c, 0.030);

    drawConeBetween(foot, add(foot, V(0.14, 0.05 * s, -0.04)),
                    0.010, 0.30, 0.16, 0.05);
}

void drawMantisModel()
{
    drawBranch();

    glPushMatrix();
    glTranslated(0.0, 0.0, 0.34);
    glRotated(-5.0, 0.0, 1.0, 0.0);

    drawForeLeg(-1);
    drawForeLeg(1);
    drawWalkingLeg(-1, 0);
    drawWalkingLeg(1, 0);
    drawWalkingLeg(-1, 1);
    drawWalkingLeg(1, 1);

    drawAbdomen();
    drawThoraxAndHead();

    drawWing(-1);
    drawWing(1);

    glPopMatrix();
}

void drawFloor()
{
    glDisable(GL_LIGHTING);
    glColor3d(0.91, 0.92, 0.90);

    glBegin(GL_QUADS);
    glVertex3d(-6.0, -4.0, -1.62);
    glVertex3d(6.0, -4.0, -1.62);
    glVertex3d(6.0, 4.0, -1.62);
    glVertex3d(-6.0, 4.0, -1.62);
    glEnd();

    glEnable(GL_LIGHTING);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslated(0.0, 0.0, -7.2);
    glScaled(zoom, zoom, zoom);
    glRotated(rotX, 1.0, 0.0, 0.0);
    glRotated(rotY, 0.0, 1.0, 0.0);
    glRotated(rotZ, 0.0, 0.0, 1.0);

    drawFloor();
    drawMantisModel();

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(34.0, (double)w / (double)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int)
{
    switch (key) {
    case 27:
    case 'q':
        exit(0);
        break;
    case 'r':
        rotX = 58.0;
        rotY = 0.0;
        rotZ = -28.0;
        zoom = 1.0;
        break;
    case '+':
        zoom *= 1.08;
        break;
    case '-':
        zoom /= 1.08;
        break;
    case 'w':
        rotX += 5.0;
        break;
    case 's':
        rotX -= 5.0;
        break;
    case 'a':
        rotZ -= 5.0;
        break;
    case 'd':
        rotZ += 5.0;
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void specialKey(int key, int, int)
{
    switch (key) {
    case GLUT_KEY_LEFT:
        rotZ -= 5.0;
        break;
    case GLUT_KEY_RIGHT:
        rotZ += 5.0;
        break;
    case GLUT_KEY_UP:
        rotX += 5.0;
        break;
    case GLUT_KEY_DOWN:
        rotX -= 5.0;
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastX = x;
        lastY = y;
    }
}

void mouseMotion(int x, int y)
{
    if (!dragging) return;

    rotZ += (double)(x - lastX) * 0.35;
    rotX += (double)(y - lastY) * 0.35;
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

void init()
{
    glClearColor(0.96f, 0.97f, 0.95f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = {2.8f, -3.5f, 5.8f, 1.0f};
    GLfloat lightDiff[] = {0.98f, 0.98f, 0.92f, 1.0f};
    GLfloat lightAmb[] = {0.28f, 0.29f, 0.24f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);

    glShadeModel(GL_SMOOTH);
    glDisable(GL_CULL_FACE);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1000, 900);
    glutCreateWindow("mantis_model_new");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
