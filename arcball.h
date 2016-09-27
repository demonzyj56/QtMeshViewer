#ifndef LEOYOLO_ARCBALL_H
#define LEOYOLO_ARCBALL_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stdio.h>
#include <cmath>

class ArcBall {
public:
    ArcBall(int width, int height)
        : m_mouse_down(false) {
        this->SetSize(width, height);
    }

    void SetSize(int width, int height) {
        m_width = static_cast<float>(width);
        m_height = static_cast<float>(height);
    }

    void MouseDown(int px, int py) {
        glm::vec2 screen_coord = this->ConvertToScreen(px, py);
        printf("ArcBall.MouseDown: Screen Coordinate: %.3f, %.3f\n", screen_coord.x, screen_coord.y);
        m_mouse_down = true;
        m_last_pos = this->MouseOnSphere(this->ConvertToScreen(px, py), glm::vec2(0.0f), 1.0f);
        m_cur_pos = m_last_pos;
    }

    void MouseMove(int px, int py) {
        glm::vec2 screen_coord = this->ConvertToScreen(px, py);
        printf("ArcBall.MouseMove: Screen Coordinate: %.3f, %.3f\n", screen_coord.x, screen_coord.y);
        if (!m_mouse_down)
            return;
        m_cur_pos = this->MouseOnSphere(this->ConvertToScreen(px, py), glm::vec2(0.0f), 1.0f);
    }

    void MouseUp(int px, int py) {
        glm::vec2 screen_coord = this->ConvertToScreen(px, py);
        printf("ArcBall.MouseUp: Screen Coordinate: %.3f, %.3f\n", screen_coord.x, screen_coord.y);
        m_mouse_down = false;
        m_cur_pos = this->MouseOnSphere(this->ConvertToScreen(px, py), glm::vec2(0.0f), 1.0f);
    }

    glm::mat4 GetMatrix() {
        return glm::mat4_cast(this->GetQuat(m_last_pos, m_cur_pos));
    }

private:
    // Project the position on screen to position on sphere coordiante.
    // Screen coordinate x, y is given by normalized coordinates w.r.t. screen size.
    glm::vec3 MouseOnSphere(const glm::vec2 &screen_coord, const glm::vec2 &ctr, float radius) {
        glm::vec3 sphere_coord;
        sphere_coord.x = (screen_coord.x - ctr.x) / radius;
        sphere_coord.y = (screen_coord.y - ctr.y) / radius;
        double mag = static_cast<double>(sphere_coord.x *sphere_coord.x + sphere_coord.y * sphere_coord.y);
        if (mag > 1.0) {
            float scale = static_cast<float>(1.0/std::sqrt(mag));
            sphere_coord.x *= scale;
            sphere_coord.y *= scale;
            sphere_coord.z = 0.f;
        } else {
            sphere_coord.z = static_cast<float>(std::sqrt(1.-mag));
        }
        return sphere_coord;
    }

    // Convert two points on unit sphere to quaternion
    glm::quat GetQuat(const glm::vec3 &from, const glm::vec3 &to) {
        return glm::rotation(from, to);
    }

    glm::vec2 ConvertToScreen(int x, int y) {
        glm::vec2 screen;
        if (m_width >= m_height) {
            screen.x = (2.f*float(x)/m_width-1.f) * (m_width/m_height);
            screen.y = -(2.f*float(y)/m_height-1.f);
        } else {
            screen.x = (2.f*float(x)/m_width-1.f);
            screen.y = -(2.f*float(y)/m_height-1.f) * (m_height/m_width);
        }
        return screen;
    }


private:
    bool m_mouse_down;
    float m_width;
    float m_height;
    glm::vec3 m_last_pos;
    glm::vec3 m_cur_pos;
//    int m_last_pos[2];
//    int m_cur_pos[2];

};

#endif // LEOYOLO_ARCBALL_H
