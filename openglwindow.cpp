#include "GL/glew.h"
#include <stdio.h>
#include "openglwindow.h"

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QGLWidget(parent)
{
//    initializeGL();
//    paintGL();
}

OpenGLWindow::~OpenGLWindow() {}

void OpenGLWindow::initializeGL() {
    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        exit(1);
    }
    glClearColor(0.7f, 0.7f, 0.7f, 0.0);
}

void OpenGLWindow::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void OpenGLWindow::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -0.0f);
    glVertex3f(0.0f, 1.0f, -0.0f);
    glVertex3f(1.0f, -1.0f, -0.0f);
    glEnd();
    glFlush();
}
