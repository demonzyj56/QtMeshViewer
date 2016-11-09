#include "common.h"
#include "GL/glew.h"
#include <stdio.h>
#include <math.h>
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

// http://devernay.free.fr/cours/opengl/materials.html
std::unordered_map<std::string, Material> RegisterMaterials() {
    auto mat = std::unordered_map<std::string, Material>{};
    mat["emerald"] = Material(0.0215, 0.1745, 0.0215, 0.07568, 0.61424, 0.07568, 0.633, 0.727811, 0.633, 0.6);
    mat["jade"] = Material(0.135, 0.2225, 0.1575, 0.54, 0.89, 0.63, 0.316228, 0.316228, 0.316228, 0.1);
    mat["obsidian"]  = Material(0.05375, 0.05, 0.06625, 0.18275, 0.17, 0.22525, 0.332741, 0.328634, 0.346435, 0.3);
    mat["pearl"] = Material(0.25, 0.20725, 0.20725, 1, 0.829, 0.829, 0.296648, 0.296648, 0.296648, 0.088);
    mat["ruby"] = Material(0.1745, 0.01175, 0.01175, 0.61424, 0.04136, 0.04136, 0.727811, 0.626959, 0.626959, 0.6);
    mat["turquoise"] = Material(0.1,0.18725,0.1745,0.396,0.74151,0.69102,0.297254,0.30829,0.306678,0.1);
    mat["brass"] = Material(0.329412, 0.223529, 0.027451, 0.780392, 0.568627, 0.113725, 0.992157, 0.941176, 0.807843, 0.21794872);
    mat["bronze"] = Material(0.2125, 0.1275, 0.054, 0.714, 0.4284, 0.18144, 0.393548, 0.271906, 0.166721, 0.2);
    mat["chrome"] = Material(0.25, 0.25, 0.25, 0.4, 0.4, 0.4, 0.774597, 0.774597, 0.774597, 0.6);
    mat["copper"] = Material(0.19125, 0.0735, 0.0225, 0.7038, 0.27048, 0.0828, 0.256777, 0.137622, 0.086014, 0.1);
    mat["gold"] = Material(0.24725, 0.1995, 0.0745, 0.75164, 0.60648, 0.22648, 0.628281, 0.555802, 0.366065, 0.4);
    mat["silver"] = Material(0.19225, 0.19225, 0.19225, 0.50754, 0.50754, 0.50754, 0.508273, 0.508273, 0.508273, 0.4);
    return mat;
}

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QGLWidget(parent), m_mesh(nullptr), m_camera(),
      m_draw_axes(true), m_draw_points(true), m_draw_edges(true),
      m_draw_faces(true), m_draw_texture(true), m_arcball(this->width(), this->height()),
      m_draw_bounding_box(false), m_lighting(true),
      m_bounding_box{0.f, 0.f, 0.f, 0.f, 0.f, 0.f}, m_projection(Persp), m_shade(Smooth),
      m_roll_speed(0.001), m_normalize_size(false), m_materials(RegisterMaterials()),
      m_material_name("emerald"), m_light_intensity(1.0)
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
//    glShadeModel(GL_SMOOTH);
    Shade();

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

    // Projection
//    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), GLfloat(w)/GLfloat(h), 0.01f, 100.0f);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glLoadMatrixf(glm::value_ptr(Projection));
    Project();

    // View
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void OpenGLWindow::paintGL() {
//    glShadeModel(GL_SMOOTH);
    Shade();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_lighting) {
        SetLight();
    } else {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
    // MVP, projection should come first.
    Project();

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
    m_camera.distance += e->delta()*m_roll_speed;
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
    this->PrintMeshInfo(filename);
    updateGL();
}

void OpenGLWindow::Render() {
    DrawAxes(m_draw_axes);
    NormalizeSize(m_normalize_size);
    DrawPoints(m_draw_points);
    DrawEdges(m_draw_edges);
    DrawFaces(m_draw_faces);
    DrawBoundingBox(m_draw_bounding_box);
//    DrawTexture(m_draw_texture);
}

void OpenGLWindow::SetLight() {
    static std::string mat_name;
    static Material material;
    static GLfloat light_position[] = {0.0, 5.0, 0.0, 1.0};
    static GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    if (mat_name != m_material_name) {
        auto it = m_materials.find(m_material_name);
        assert(it != m_materials.end());
        mat_name = it->first;
        material = it->second;
    }
    white_light[0] = white_light[1] = white_light[2] = m_light_intensity;
    glMaterialfv(GL_FRONT, GL_AMBIENT, material.Ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material.Diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.Specular);
    glMaterialf(GL_FRONT, GL_SHININESS, material.Shininess*128.);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, material.ModelAmbient);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);     // This is important!
    glEnable(GL_LIGHTING);
}

void OpenGLWindow::DrawAxes(bool bv) {
    if (bv) {
        if (m_lighting) {
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);
        }
        glLineWidth(3.);
        static Cone cx(0.4, 0.1, 18);
        static Cone cy(0.4, 0.1, 18);
        static Cone cz(0.4, 0.1, 18);
        // x-axis
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(4.7f, 0.0f, 0.0f);
        glEnd();
        glPushMatrix(); // cone-x
        glTranslatef(4.7, 0.0, 0.0);
        glRotatef(90.0, 0.0, 1.0, 0.0);
        cx.Draw();
        glPopMatrix();

        // y-axis
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 4.7f, 0.0f);
        glEnd();
        glPushMatrix(); // cone-y
        glTranslatef(0.0, 4.7, 0.0);
        glRotatef(-90.0, 1.0, 0.0, 0.0);
        cy.Draw();
        glPopMatrix();


        // z-axis
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 4.7f);
        glEnd();
        glPushMatrix(); // cone-z
        glTranslatef(0.0, 0.0, 4.7);
        cz.Draw();
        glPopMatrix();
        glLineWidth(1.);

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
        if (m_lighting) {
            SetLight();
        }
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
        glLineWidth(3.);
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
        glLineWidth(1.);
    }
}

void OpenGLWindow::NormalizeSize(bool bv) {
    if (bv && m_mesh) {
        float ctr_x = (m_bounding_box.xmin + m_bounding_box.xmax)/2;
        float ctr_y = (m_bounding_box.ymin + m_bounding_box.ymax)/2;
        float ctr_z = (m_bounding_box.zmin + m_bounding_box.zmax)/2;
        float x = m_bounding_box.xmax - m_bounding_box.xmin;
        float y = m_bounding_box.ymax - m_bounding_box.ymin;
        float z = m_bounding_box.zmax - m_bounding_box.zmin;
        float scale = 1.f/MIN(x, MIN(y, z));
        glTranslatef(scale*ctr_x, scale*ctr_y, scale*ctr_z);
        glScalef(scale, scale, scale);
        glTranslatef(-ctr_x, -ctr_y, -ctr_z);
    }
}

void OpenGLWindow::Project() {
    static float pi = acos(-1.);
    static float fov = 45.0;
    static float len = tan(fov/2./180.*pi)*sqrt(3.); // Heuristics since init location is (1,1,1).
    float ar = float(this->width()) / float(this->height()); // aspect ratio
    if (m_projection == Ortho) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Adjust viewing window to zoom in/out.
        glOrtho(-m_camera.distance*len*ar, m_camera.distance*len*ar,
                -m_camera.distance*len, m_camera.distance*len,
                0.01, 100.);
    } else {
        glm::mat4 Projection = glm::perspective(glm::radians(fov),
            GLfloat(ar), 0.01f, 100.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(Projection));
    }
}

void OpenGLWindow::Shade() {
    glShadeModel(m_shade==Smooth?GL_SMOOTH:GL_FLAT);
}

// helper func

void OpenGLWindow::ComputeBoundingBox() {
    if (!m_mesh) return;
    m_bounding_box.xmin = m_bounding_box.xmax =
    m_bounding_box.ymin = m_bounding_box.ymax =
    m_bounding_box.zmin = m_bounding_box.zmax = 0.;
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

void OpenGLWindow::PrintMeshInfo(const QString &filename = "") {
    if (!m_mesh) return;
    this->ComputeBoundingBox();
    printf("------ Mesh Info ------\n");
    if (!filename.isEmpty())
        printf("Mesh name: %s\n", filename.toStdString().c_str());
    printf("Num of vertices: %d\n", (int)m_mesh->NumVertices());
    printf("Num of half edges: %d\n", (int)m_mesh->NumEdges());
    printf("Num of faces: %d\n", (int)m_mesh->NumFaces());
    printf("(xmin, xmax): (%.3f, %.3f)\n", m_bounding_box.xmin, m_bounding_box.xmax);
    printf("(ymin, ymax): (%.3f, %.3f)\n", m_bounding_box.ymin, m_bounding_box.ymax);
    printf("(zmin, zmax): (%.3f, %.3f)\n", m_bounding_box.zmin, m_bounding_box.zmax);
    printf("Euler characteristic: %d\n", (int)m_mesh->NumVertices()-(int)m_mesh->NumEdges()/2+(int)m_mesh->NumFaces());
    printf("-----------------------\n");
}
