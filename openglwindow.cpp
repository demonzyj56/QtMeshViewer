#include "common.h"
#include "GL/glew.h"
#include <stdio.h>
#include "openglwindow.h"
#include "TriMesh.h"
#include "MParser.h"
#include "arcball.h"
#include <QFileDialog>
#include <QString>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QGLWidget(parent), m_mesh(nullptr), m_camera(),
      m_draw_axes(true), m_draw_points(true), m_draw_edges(true),
      m_draw_faces(true), m_draw_texture(true), m_arcball(this->width(), this->height()),
      m_draw_bounding_box(false), m_lighting(true),
      m_bounding_box{0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
{
}

OpenGLWindow::~OpenGLWindow() {}

void OpenGLWindow::initializeGL() {
    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        exit(1);
    }
    glClearColor(0.3f, 0.3f, 0.3f, 0.0);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);

    SetLight();
}

void OpenGLWindow::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    m_arcball.SetSize(w, h);

    // View
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Projection
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), GLfloat(w)/GLfloat(h), 0.01f, 100.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(Projection));
}

void OpenGLWindow::paintGL() {
    glShadeModel(GL_SMOOTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_lighting) {
        SetLight();
    } else {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(glm::value_ptr(m_camera.LookAt()));

    glPushMatrix();
    glMultMatrixf(glm::value_ptr(m_arcball.GetMatrix()));
    Render();
    glPopMatrix();

}

// events
void OpenGLWindow::mousePressEvent(QMouseEvent *e) {
    switch (e->button()) {
    case Qt::LeftButton:
//        printf("Left button is pressed.\n");
        m_arcball.MouseDown(e->pos().x(), e->pos().y());
        break;
    case Qt::MidButton:
//        printf("Mouse MidButton is pressed. Current position: %d %d\n", e->pos().x(), e->pos().y());
        m_camera.SetCurrentPosition(e->pos().x(), e->pos().y());
        break;
    case Qt::RightButton:
        break;
    default:
        break;
    }
    updateGL();
}
void OpenGLWindow::mouseMoveEvent(QMouseEvent *e) {
    // Note that the returned value is always Qt::NoButton for mouse move events.
    switch (e->buttons()) { // This should be BUTTONS!
    case Qt::LeftButton:
        m_arcball.MouseMove(e->pos().x(), e->pos().y());
        break;
    case Qt::MidButton:
//        printf("Mouse MidButton is moved. Current position: %d %d\n", e->pos().x(), e->pos().y());
        m_camera.CameraMove(e->pos().x(), e->pos().y(), this->width(), this->height());
        m_camera.SetCurrentPosition(e->pos().x(), e->pos().y());
        break;
    case Qt::RightButton:
        break;
    default:
        printf("No such button!\n");
        break;
    }
    updateGL();
}
void OpenGLWindow::mouseReleaseEvent(QMouseEvent *e) {
    switch (e->button()) {
    case Qt::LeftButton:
        m_arcball.MouseUp(e->pos().x(), e->pos().y());
        break;
    case Qt::MidButton:
        break;
    case Qt::RightButton:
        break;
    default:
        break;
    }
    updateGL();
}

void OpenGLWindow::wheelEvent(QWheelEvent *e) {
    m_camera.distance += e->delta()*0.001;
    m_camera.distance = m_camera.distance > 0 ? m_camera.distance : 0;
    updateGL();
}

//void OpenGLWindow::keyPressEvent(QKeyEvent *e) {}
//void OpenGLWindow::keyReleaseEvent(QKeyEvent *e) {}

// public slots:


void OpenGLWindow::ReadMesh() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Read m-file"), ".", tr("M-File (*.m)"));
    if (filename.isEmpty()) {
        emit(operatorInfo(QString("Cannot open mesh file.")));
        return;
    }
//    MParser parser{};
//    m_mesh = parser.ReadMFile(filename.toStdString());
    m_mesh = ReadMFile(filename.toStdString());
    if (!m_mesh) {
        emit(operatorInfo(QString("Cannot read mesh from file.")));
        return;
    }
    emit(operatorInfo(QString("Read Mesh from")+filename));
    this->ComputeBoundingBox();
    printf("------ Mesh Info ------\n");
    printf("Mesh name: %s\n", filename.toStdString().c_str());
    printf("Num of vertices: %d\n", (int)m_mesh->NumVertices());
    printf("Num of edges: %d\n", (int)m_mesh->NumEdges());
    printf("Num of faces: %d\n", (int)m_mesh->NumFaces());
    printf("-----------------------\n");
    updateGL();
}

void OpenGLWindow::Render() {
    DrawAxes(m_draw_axes);
    DrawPoints(m_draw_points);
    DrawEdges(m_draw_edges);
    DrawFaces(m_draw_faces);
    DrawBoundingBox(m_draw_bounding_box);
//    DrawTexture(m_draw_texture);
}

void OpenGLWindow::SetLight() {
    static GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    static GLfloat mat_shininess[] = {50.0};
    static GLfloat light_position[] = {0.0, 0.0, 0.0, 1.0};
    static GLfloat white_light[] = {0.8, 0.8, 0.8, 1.0};
    static GLfloat lmodel_ambient[] = {1.0, 1.0, 1.0, 1.0};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void OpenGLWindow::DrawAxes(bool bv) {
    if (bv) {
        // x-axis
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(4.7f, 0.0f, 0.0f);
        glEnd();

        // y-axis
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 4.7f, 0.0f);
        glEnd();

        // z-axis
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 4.7f);
        glEnd();

        // ground
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_LINES);
        for (int i = 0; i <= 20; ++i) {
            glVertex3f(-2.f+0.2f*i, 0.f, -2.f);
            glVertex3f(-2.f+0.2*i, 0.f, 2.0f);
            glVertex3f(-2.f, 0.f, -2.f+0.2f*i);
            glVertex3f(2.f, 0.f, -2.f+0.2f*i);
        }
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

void OpenGLWindow::DrawTexture(bool bv) {
    NotImplemented;
}

void OpenGLWindow::DrawPoints(bool bv) {
    if (bv && m_mesh) {
        glBegin(GL_POINTS);
        for (auto vit = m_mesh->GetVerticesBegin(); vit != m_mesh->GetVerticesEnd(); ++vit) {
            glNormal3f((*vit)->nx, (*vit)->ny, (*vit)->nz);
            glVertex3f((*vit)->x, (*vit)->y, (*vit)->z);
        }
        glEnd();
    }
}

void OpenGLWindow::DrawEdges(bool bv) {
    if (bv && m_mesh) {
        // We iterate over all faces and draw edges by GL_LINE_LOOP
        for (auto fit = m_mesh->GetFacesBegin(); fit != m_mesh->GetFacesEnd(); ++fit) {
            HE_edge *e = (*fit)->edge;
            HE_vert *v1 = e->vert;
            HE_vert *v2 = e->next->vert;
            HE_vert *v3 = e->prev->vert;
            glBegin(GL_LINE_LOOP);
            glNormal3f(v1->nx, v1->ny, v1->nz);
            glVertex3f(v1->x, v1->y, v1->z);
            glNormal3f(v2->nx, v2->ny, v2->nz);
            glVertex3f(v2->x, v2->y, v2->z);
            glNormal3f(v3->nx, v3->ny, v3->nz);
            glVertex3f(v3->x, v3->y, v3->z);
            glEnd();
        }
    }
}

void OpenGLWindow::DrawFaces(bool bv) {
    if (bv && m_mesh) {
        glBegin(GL_TRIANGLES);
        for (auto fit = m_mesh->GetFacesBegin(); fit != m_mesh->GetFacesEnd(); ++fit) {
            HE_edge *e = (*fit)->edge;
            HE_vert *v1 = e->vert;
            HE_vert *v2 = e->next->vert;
            HE_vert *v3 = e->prev->vert;
            glNormal3f(v1->nx, v1->ny, v1->nz);
            glVertex3f(v1->x, v1->y, v1->z);
            glNormal3f(v2->nx, v2->ny, v2->nz);
            glVertex3f(v2->x, v2->y, v2->z);
            glNormal3f(v3->nx, v3->ny, v3->nz);
            glVertex3f(v3->x, v3->y, v3->z);
        }
        glEnd();
    }
}

void OpenGLWindow::DrawBoundingBox(bool bv) {
    if (bv && m_mesh) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        // zmin->zmax
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmax);

        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmax);

        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmax);

        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmax);

        // ymin->ymax
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmin);

        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmin);

        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmax);
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmax);

        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmax);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmax);

        // xmin->xmax
        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmin);

        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmin);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmin);

        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymin, m_bounding_box.zmax);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymin, m_bounding_box.zmax);

        glVertex3f(m_bounding_box.xmin, m_bounding_box.ymax, m_bounding_box.zmax);
        glVertex3f(m_bounding_box.xmax, m_bounding_box.ymax, m_bounding_box.zmax);

        glEnd();
    }
}

void OpenGLWindow::ComputeBoundingBox() {
    if (!m_mesh) return;
    for (auto vit = m_mesh->GetVerticesBegin(); vit != m_mesh->GetVerticesEnd(); ++vit) {
        if ((*vit)->x > m_bounding_box.xmax)
            m_bounding_box.xmax = (*vit)->x;
        if ((*vit)->x < m_bounding_box.xmin)
            m_bounding_box.xmin = (*vit)->x;
        if ((*vit)->y > m_bounding_box.ymax)
            m_bounding_box.ymax = (*vit)->y;
        if ((*vit)->y < m_bounding_box.ymin)
            m_bounding_box.ymin = (*vit)->y;
        if ((*vit)->z > m_bounding_box.zmax)
            m_bounding_box.zmax = (*vit)->z;
        if ((*vit)->z < m_bounding_box.zmin)
            m_bounding_box.zmin = (*vit)->z;
    }
}
