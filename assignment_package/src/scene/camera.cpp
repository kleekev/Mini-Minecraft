#include "camera.h"
#include "glm_includes.h"
#include <algorithm>
#include <iostream>

Camera::Camera(glm::vec3 pos)
    : Camera(400, 400, pos)
{}

Camera::Camera(unsigned int w, unsigned int h, glm::vec3 pos)
    : Entity(pos), m_fovy(45), m_width(w), m_height(h),
      m_near_clip(0.1f), m_far_clip(1000.f), m_aspect(w / static_cast<float>(h)),
      phi(0.f), theta(0.f), sensitivity(40.f)
{}

Camera::Camera(const Camera &c)
    : Entity(c),
      m_fovy(c.m_fovy),
      m_width(c.m_width),
      m_height(c.m_height),
      m_near_clip(c.m_near_clip),
      m_far_clip(c.m_far_clip),
      m_aspect(c.m_aspect),
      phi(c.phi),
      theta(c.theta),
      sensitivity(c.sensitivity)
{}


void Camera::setWidthHeight(unsigned int w, unsigned int h) {
    m_width = w;
    m_height = h;
    m_aspect = w / static_cast<float>(h);
}


void Camera::tick(float dT, InputBundle &input) {
    theta += (input.mouseX * m_aspect * sensitivity);
    theta = fmod(theta, 360.f);
    phi += (input.mouseY * sensitivity);
    // set max phi just under 90 degrees so that we can still derive forward vector from phi
    phi = std::clamp(phi, -89.9f, 89.9f);
    glm::mat4 rotatePhi = glm::rotate(glm::mat4(1.f), glm::radians(phi), glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 rotateTheta = glm::rotate(glm::mat4(1.f), glm::radians(-theta), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rotation = rotateTheta * rotatePhi;
    m_forward = glm::vec3(rotation * glm::vec4(0.f, 0.f, 1.f, 0.f));
    m_up = glm::vec3(rotation * glm::vec4(0.f, 1.f, 0.f, 0.f));
    m_right = glm::vec3(rotation * glm::vec4(-1.f, 0.f, 0.f, 0.f));

    input.mouseY = 0.f;
    input.mouseX = 0.f;
}

glm::mat4 Camera::getViewProj() const {
    return glm::perspective(glm::radians(m_fovy), m_aspect, m_near_clip, m_far_clip) * glm::lookAt(m_position, m_position + m_forward, m_up);
}
