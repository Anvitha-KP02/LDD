#include<stdio.h>
#include<GL/glut.h>
#include<math.h>

#define PI 3.1415926535

float angle = 0.0;   // rotation angle

void myInit(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(0.0, 1.0, 0.0);
    glPointSize(2.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-780, 780, -420, 420);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();

    glRotatef(angle, 0.0, 0.0, 1.0); // rotate around Z-axis

    glBegin(GL_TRIANGLES);
        glVertex2f(0, 200);
        glVertex2f(-200, -200);
        glVertex2f(200, -200);
    glEnd();

    glPopMatrix();

    glFlush();
}


void update(int value)
{
    angle += 1.0;   // degrees

    if (angle > 360)
        angle = 0;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowSize(1366, 768);
    glutCreateWindow("Rotating Circle");

    myInit();
    glutDisplayFunc(display);

    glutTimerFunc(0, update, 0);

    glutMainLoop();
}
