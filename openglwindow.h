#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H
#include <GL/glew.h>
#include <QGLWidget>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "arcball.h"
#include <vector>
#include <math.h>
#include <unordered_map>
#include <string>

class TriMesh;
class QString;
class QMouseEvent;
class QWheelEvent;

// Referring to
// http://devernay.free.fr/cours/opengl/materials.html
struct Material {
    GLfloat AmbientIntensity;
    GLfloat Ambient[4];
    GLfloat Diffuse[4];
    GLfloat Specular[4];
    GLfloat Shininess;
    GLfloat ModelAmbient[4];
    GLfloat SetAmbientIntensity() {
        AmbientIntensity = (0.212671*Ambient[0]+0.715160*Ambient[1]+0.072169*Ambient[2]) /
                (0.212671*Diffuse[0]+0.715160*Diffuse[1]+0.072169*Diffuse[2]);
        ModelAmbient[0] = ModelAmbient[1] = ModelAmbient[2] = AmbientIntensity;
        ModelAmbient[3] = 1.;
        return AmbientIntensity;
    }
    Material() {}
    explicit Material(float ar, float ag, float ab,
                      float dr, float dg, float db,
                      float sr, float sg, float sb,
                      float sh) {
        Ambient[0] = ar; Ambient[1] = ag; Ambient[2] = ab; Ambient[3] = 1.;
        Diffuse[0] = dr; Diffuse[1] = dg; Diffuse[2] = db; Diffuse[3] = 1.;
        Specular[0] = sr; Specular[1] = sg; Specular[2] = sb; Specular[3] = 1.;
        Shininess = sh;
        SetAmbientIntensity();
    }
};

std::unordered_map<std::string, Material> RegisterMaterials();

class OpenGLWindow : public QGLWidget
{
    Q_OBJECT

    struct Camera {
        float distance; // distance of camera from goal
        glm::vec3 goal; // position where camera looks at
        glm::vec3 init_position; // This gives the initial position of the camera.
        int current_position[2];

        Camera() : distance(3.f), goal(0.f), init_position{1.0f, 1.0f, 1.0f}, current_position{0, 0} {}
        glm::mat4 LookAt() const {
            return glm::lookAt(distance * init_position,
                               goal,
                               glm::vec3(0.0f, 1.0f, 0.0f));
        }
        void CameraMove(int x, int y, int width, int height) {
            goal[0] -= 4.*GLfloat(x - current_position[0]) / GLfloat(width);
            goal[1] += 4.*GLfloat(y - current_position[1]) / GLfloat(height);
        }
        void SetCurrentPosition(int x, int y) {
            current_position[0] = x;
            current_position[1] = y;
        }
        glm::vec3 GetCurrentPosition() const {
            return distance * init_position;
        }
    };


    // Draw a cone with base on the x-y plane and apex at the z-coord.
    struct Cone {
        float h;     // height
        float r;     // radius
        int n;       // Number of radial "slices"
        float pi;
        std::vector<glm::vec3> e; // All points at the base.
        explicit Cone(float height, float radius, int num)
            : h(height), r(radius), n(num), pi(acos(-1.)) { // specify the base points
            assert(360 % n == 0 && "The number of points cannot be divided by 360.");
            for (int i = 0; i != n; ++i) {
                float ang = float(i)*2.*pi/float(n);
                e.emplace_back(radius*cos(ang), radius*sin(ang), 0.);
            }
            e.push_back(e.front()); // Push the last triangle.
        }

        void Draw() { // All we need to do is to calculate the coords of the base points.
            assert(e.size() == size_t(n+1));
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0., 0., h);
            for (int i = 0; i != n+1; ++i) {
                glVertex3f(e[i].x, e[i].y, e[i].z);
            }
            glEnd();
            glBegin(GL_POLYGON);
            for (int i = 0; i != n; ++i) {
                glVertex3f(e[i].x, e[i].y, e[i].z);
            }
            glEnd();
        }
    };

public:
    OpenGLWindow(QWidget *parent);
    ~OpenGLWindow();
    enum ProjMode {Ortho, Persp};
    enum ShadeMode {Flat, Smooth};

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;
//    void keyPressEvent(QKeyEvent *e) override;
//    void keyReleaseEvent(QKeyEvent *e) override;

private:
    void Render();      // Main func doing the dirty job
    void SetLight();
    void DrawAxes(bool);
    void DrawPoints(bool);
    void DrawEdges(bool);
    void DrawFaces(bool);
    void DrawTexture(bool);
    void DrawBoundingBox(bool);
    void NormalizeSize(bool);
    void Project();
    void Shade();

public slots:
    void ReadMesh();
    void SetDrawPoints(bool b) {m_draw_points = b; updateGL();}
    void SetDrawEdges(bool b) {m_draw_edges = b; updateGL();}
    void SetDrawFaces(bool b) {m_draw_faces = b; updateGL(); }
    void SetDrawAxes(bool b) {m_draw_axes = b; updateGL();}
    void SetDrawBoundingBox(bool b) {m_draw_bounding_box = b; updateGL();}
    void SetDrawLighting(bool b) {m_lighting = b; updateGL();}
    void SetNormalized(bool b) {m_normalize_size = b; updateGL();}
    void SetProjectionMode(int p) {m_projection = (p==0?Persp:Ortho); updateGL();}
    void SetShadeMode(int s) {m_shade = (s==0?Smooth:Flat); updateGL();}
    void SetRollSpeed(double s) {m_roll_speed = s; updateGL();}
    void SetMaterial(const QString &s) {m_material_name  = s.toStdString(); updateGL();}
    void SetLightIntensity(double l) {m_light_intensity = float(l); updateGL();}


signals:
    void operatorInfo(QString); // a simple signal hoding information.
                                // probably for logging

private: // helper func
    void ComputeBoundingBox();
    void PrintMeshInfo(const QString &filename);

private:
    std::shared_ptr<TriMesh> m_mesh;
    Camera m_camera;
    ArcBall m_arcball;
    bool m_draw_axes;
    bool m_draw_points;
    bool m_draw_edges;
    bool m_draw_faces;
    bool m_draw_texture;
    bool m_draw_bounding_box;
    bool m_lighting;
    bool m_normalize_size;
    ProjMode m_projection;
    ShadeMode m_shade;
    double m_roll_speed;
    struct {float xmin, xmax, ymin, ymax, zmin, zmax;} m_bounding_box;
    std::string m_material_name;
    std::unordered_map<std::string, Material> m_materials;
    float m_light_intensity;
};

#endif // OPENGLWINDOW_H
