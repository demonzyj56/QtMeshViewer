#ifndef LEOYOLO_ARCBALL_H
#define LEOYOLO_ARCBALL_H

#include <glm/glm.hpp>
#include <stdio.h>

class ArcBall {
private:
    void MouseDown(int px, int py, int width, int height) {
        m_mouse_down = true;
    }

    void MouseMove(int px, int py, int width, int height);
    void MouseUp(int px, int py, int width, int height);

private:
    bool m_mouse_down;

};

#endif // LEOYOLO_ARCBALL_H
