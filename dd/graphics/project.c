#include <GL/glut.h>

float angle = 0.0;

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    // Move object slightly backward
    glTranslatef(0.0f, 0.0f, -5.0f);

    // Rotate cube
    glRotatef(angle, 1.0f, 1.0f, 0.0f);

    // Draw Cube
    glutWireCube(2.0);

    glutSwapBuffers();
}

// Update rotation
void update(int value) {
    angle += 1.0f;
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Initialize settings
void init() {
    glEnable(GL_DEPTH_TEST);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("2D/3D Renderer");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
