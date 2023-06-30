#pragma once
#include "glm_includes.h"
#include "scene/entity.h"



//A perspective projection camera
//Receives its eye position and reference point from the scene XML file
class Camera : public Entity {
private:
    float m_fovy;
    unsigned int m_width, m_height;  // Screen dimensions
    float m_near_clip;  // Near clip plane distance
    float m_far_clip;  // Far clip plane distance
    float m_aspect;    // Aspect ratio
    float phi; // angle that we rotate about the x-axis
    float theta; // angle that we rotate about the y-axis
    float sensitivity; // degrees in which we update theta and phi after mouse movement between -1 to 1

public:

    Camera(glm::vec3 pos);
    Camera(unsigned int w, unsigned int h, glm::vec3 pos);
    Camera(const Camera &c);
    void setWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    glm::mat4 getViewProj() const;
};
