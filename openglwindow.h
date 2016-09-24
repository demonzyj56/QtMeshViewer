#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H
#include <QGLWidget>

class OpenGLWindow : public QGLWidget
{
public:
    OpenGLWindow(QWidget *parent);
    ~OpenGLWindow();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};

#endif // OPENGLWINDOW_H
