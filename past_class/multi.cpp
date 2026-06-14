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

// マウス処理．
bool click = false;

GLdouble M[16];

void scale(GLdouble x, GLdouble y, GLdouble z, GLdouble M[])
{
    M[0] = x;   M[1] = 0.0; M[2] = 0.0; M[3] = 0.0;
    M[4] = 0.0; M[5] = y;   M[6] = 0.0; M[7] = 0.0;
    M[8] = 0.0; M[9] = 0.0; M[10] = z;  M[11] = 0.0;
    M[12] = 0.0; M[13] = 0.0; M[14] = 0.0; M[15] = 1.0;
}

void product(GLdouble vertex[][3], GLdouble M[])
{
    for (int i = 0; i < 8; i++) {
        GLdouble x = vertex[i][0];
        GLdouble y = vertex[i][1];
        GLdouble z = vertex[i][2];

        vertex[i][0] = M[0] * x + M[1] * y + M[2] * z + M[3];
        vertex[i][1] = M[4] * x + M[5] * y + M[6] * z + M[7];
        vertex[i][2] = M[8] * x + M[9] * y + M[10] * z + M[11];
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    /* 図形の描画 */
    glColor3d(0.0, 0.0, 1.0);

    glBegin(GL_LINES);

    // forループを使って12個の辺を表す線を引く
    for (int i = 0; i < 12; i++) {
        glVertex3dv(vertex[edge[i][0]]);
        glVertex3dv(vertex[edge[i][1]]);
    }

    glEnd();

    glFlush();
}

// マウスのボタン処理．
void mouse_click(int button, int state, int x, int y)
{
    if ((state == GLUT_DOWN) && (button == GLUT_LEFT_BUTTON))
    {
        click = true;

        product(vertex, M);

        glutPostRedisplay();
    }
    else if (state == GLUT_UP)
    {
        click = false;
    }
}

void init(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glScalef(0.5f, 0.5f, 0.5f);
}

int main(int argc, char *argv[])
{
    GLdouble x, y, z;

    x = 1.1;
    y = 1.1;
    z = 1.1;

    scale(x, y, z, M);

    glutInitWindowSize(500, 500);

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA);

    glutCreateWindow(argv[0]);

    glutDisplayFunc(display);

    glutMouseFunc(mouse_click);

    init();

    glutMainLoop();

    return 0;
}