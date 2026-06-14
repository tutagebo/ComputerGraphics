// 本プログラムはmacOS上で作成および動作確認を行っている。
// 授業ではWindows環境の使用が推奨されているが、個人の開発環境の都合によりMacを利用している。
// そのため、ライブラリ（GLUT）のインクルードパスやコンパイル方法が
// Windows環境と異なる点に注意されたい。

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "stdio.h"
#include "math.h"

GLdouble vertex[][3] = {
    {-1.0, -1.0, -1.0},
    { 1.0, -1.0, -1.0},
    { 1.0,  1.0, -1.0},
    {-1.0,  1.0, -1.0},
    {-1.0, -1.0,  1.0},
    { 1.0, -1.0,  1.0},
    { 1.0,  1.0,  1.0},
    {-1.0,  1.0,  1.0}
};
int edge[][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};
static double pi = 3.14159265358979323846;
static double ini_q[4] = {1.0, 0.0, 0.0, 0.0};
static double tmp_q[4];
GLdouble M[16];
static int ini_x, ini_y;
static double r_x, r_y;

// クォータニオンの積 r = qp
void q_product(double r[], double q[], double p[])
{
    r[0] = q[0] * p[0] - q[1] * p[1] - q[2] * p[2] - q[3] * p[3];
    r[1] = q[0] * p[1] + q[1] * p[0] + q[2] * p[3] - q[3] * p[2];
    r[2] = q[0] * p[2] - q[1] * p[3] + q[2] * p[0] + q[3] * p[1];
    r[3] = q[0] * p[3] + q[1] * p[2] - q[2] * p[1] + q[3] * p[0];
}

// クォータニオンから回転行列を作る関数
void q_to_m(double M[], double q[])
{
    double w = q[0], x = q[1], y = q[2], z = q[3];

    M[0] = 1.0 - 2.0 * y * y - 2.0 * z * z;
    M[1] = 2.0 * x * y + 2.0 * w * z;
    M[2] = 2.0 * x * z - 2.0 * w * y;
    M[3] = 0.0;

    M[4] = 2.0 * x * y - 2.0 * w * z;
    M[5] = 1.0 - 2.0 * x * x - 2.0 * z * z;
    M[6] = 2.0 * y * z + 2.0 * w * x;
    M[7] = 0.0;

    M[8] = 2.0 * x * z + 2.0 * w * y;
    M[9] = 2.0 * y * z - 2.0 * w * x;
    M[10] = 1.0 - 2.0 * x * x - 2.0 * y * y;
    M[11] = 0.0;

    M[12] = 0.0;
    M[13] = 0.0;
    M[14] = 0.0;
    M[15] = 1.0;
}

// ゲーミング背景用
void gaming_background(void)
{
    static double t = 0.0;

    double r = (sin(t) + 1.0) / 2.0;
    double g = (sin(t + 2.0 * pi / 3.0) + 1.0) / 2.0;
    double b = (sin(t + 4.0 * pi / 3.0) + 1.0) / 2.0;

    glClearColor(r, g, b, 1.0);

    t += 0.02;
}

void display(void)
{
    gaming_background();

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3d(1.0, 1.0, 1.0);

    glLoadMatrixd(M);
    glScalef(0.5f, 0.5f, 0.5f);

    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++) {
        glVertex3dv(vertex[edge[i][0]]);
        glVertex3dv(vertex[edge[i][1]]);
    }
    glEnd();

    glutSwapBuffers();
}

void resize(int width, int height)
{
    r_x = 1.0 / (double)width;
    r_y = 1.0 / (double)height;
}

void idle(void)
{
    glutPostRedisplay();
}

void mouse_click(int click, int state, int x, int y)
{
    switch (click)
    {
    case GLUT_LEFT_BUTTON:
        switch (state)
        {
        case GLUT_DOWN:
            // ドラッグ開始位置
            ini_x = x;
            ini_y = y;
            // 回転開始
            glutIdleFunc(idle);
            break;
        case GLUT_UP:
            // 回転終了
            glutIdleFunc(idle);
            ini_q[0] = tmp_q[0];
            ini_q[1] = tmp_q[1];
            ini_q[2] = tmp_q[2];
            ini_q[3] = tmp_q[3];
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void mouse_drag(int x, int y)
{
    double dx, dy, distance;
    // マウスのドラッグ開始位置からの変位
    dx = (x - ini_x) * r_x;
    dy = (y - ini_y) * r_y;
    // マウスのドラッグ開始位置からの移動距離
    distance = sqrt(dx * dx + dy * dy);
    if (distance != 0.0)
    {
        // 回転角を1/2倍した値(θ)
        double theta = distance * pi;
        // 回転軸ベクトル（単位ベクトル）
        double qx = sin(theta) * dy / distance;
        double qy = sin(theta) * dx / distance;
        double qz = 0;
        // マウスのドラッグに伴う回転のクォータニオン drag_q
        double drag_q[4] = {cos(theta), qx, qy, qz};
        // 回転の初期値 ini_q に drag_q を掛けて回転を合成
        //  すなわち drag_q x ini_q を計算する。
        q_product(tmp_q, drag_q, ini_q);
        // クォータニオンを回転行列に変換
        q_to_m(M, tmp_q);
    }
}

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    q_to_m(M, ini_q);
}

int main(int argc, char *argv[])
{
    glutInitWindowSize(500, 500);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutIdleFunc(idle);
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_drag);
    init();
    glutMainLoop();
    return 0;
}
