#include<stdio.h>
#include<GL/glut.h>
#include<math.h>

#define PI 3.1415926535

float centerX = 0;   // circle center X
float centerY = 0;   // circle center Y

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
    glBegin(GL_POINTS);

    float x, y;

    for (float i = 0; i < 2 * PI; i += 0.001)
    {
        x = centerX + 200 * cos(i);
        y = centerY + 200 * sin(i);
        glVertex2f(x, y);
    }

    glEnd();
    glFlush();
}

void update(int value)
{
    // Move circle to the right
    centerX += 2.0;
    centerY += 1.5;

    // Reset position if it goes out of screen
    if (centerX > 800)
        centerX = -800;

    glutPostRedisplay();  // request redraw
    glutTimerFunc(16, update, 0); // ~60 FPS
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowSize(1366, 768);
    glutCreateWindow("Moving Circle");

    myInit();

    glutDisplayFunc(display);

    // Timer for animation
    glutTimerFunc(0, update, 0);

    glutMainLoop();
}
