#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H
#include <QGLWidget>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TriMesh;
class QString;
class QMouseEvent;
class QWheelEvent;
//class ArcBall;

class OpenGLWindow : public QGLWidget
{
    Q_OBJECT

    struct Camera {
        float distance; // distance of camera from goal
        glm::vec3 goal; // position where camera looks at
        glm::vec3 init_position; // This gives the initial position of the camera.
        int current_position[2];

        Camera() : distance(3.f), goal(0.f), init_position{0.0f, 0.0f, 1.0f}, current_position{0, 0} {}
        glm::mat4 LookAt() const {
            return glm::lookAt(distance * init_position,
                               goal,
                               glm::vec3(0.0f, 1.0f, 0.0f));
        }
        void CameraMove(int x, int y, int width, int height) {
            goal[0] -= 2.*GLfloat(x - current_position[0]) / GLfloat(width);
            goal[1] += 2.*GLfloat(y - current_position[1]) / GLfloat(height);
        }
        void SetCurrentPosition(int x, int y) {
            current_position[0] = x;
            current_position[1] = y;
        }
    };

public:
    OpenGLWindow(QWidget *parent);
    ~OpenGLWindow();

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
    void Render();      // Main func doning the dirty job
    void DrawAxes(bool);
    void DrawPoints(bool);
    void DrawEdges(bool);
    void DrawFaces(bool);
    void DrawTexture(bool);

public slots:
    void ReadMesh();


signals:
    void operatorInfo(QString); // a simple signal hoding information.
                                // probably for logging
private:
    std::shared_ptr<TriMesh> m_mesh;
    Camera m_camera;
//    ArcBall m_arcball;
    bool m_draw_axes;
    bool m_draw_points;
    bool m_draw_edges;
    bool m_draw_faces;
    bool m_draw_texture;
};

#endif // OPENGLWINDOW_H
