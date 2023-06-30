#include "player.h"
#include <QString>
#include <iostream>
#include <cmath>
#include "utils.h"

#define ROA 35.f
#define EPSILON 0.001f
#define GRAVITY 90.f
#define JUMPVELOCITY 10.f
#define SWIMVELOCITY 2.f
#define PLAYERWIDTH 0.4f

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0, 0, 0), m_acceleration(0.f, 0.f, 0.f),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      m_flyMode(true), m_isGrounded(false), m_inLiquid(false),
      mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {

    // updates the camera orientation based on mouse inputs
    m_camera.tick(dT, input);

    // sync player local axes to camera local axes
    if (m_flyMode) {
        m_forward = glm::vec3(m_camera.mcr_forward);
        m_right = glm::vec3(m_camera.mcr_right);
        m_up = glm::vec3(m_camera.mcr_up);
    }
    else {
        m_forward = glm::normalize(glm::vec3(m_camera.mcr_forward[0], 0.f, m_camera.mcr_forward[2]));
        m_up = glm::vec3(0.f, 1.f, 0.f);
        m_right = glm::cross(m_forward, m_up);
    }

    processInputs(dT, input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(float dT, InputBundle &inputs) {

    m_acceleration = glm::vec3(0.f);
    // find the direction that the player wants to move in
    glm::vec3 direction = glm::vec3(0.f);
    if (inputs.wPressed) {
        direction += m_forward;
    }
    if (inputs.aPressed) {
        direction += -m_right;
    }
    if (inputs.sPressed) {
        direction += -m_forward;
    }
    if (inputs.dPressed) {
        direction += m_right;
    }
    if (m_flyMode && inputs.ePressed) {
        direction += glm::vec3(0.f, 1.f, 0.f);
    }
    if (m_flyMode && inputs.qPressed) {
        direction += glm::vec3(0.f, -1.f, 0.f);
    }
    if (!m_flyMode && inputs.spacePressed) {

        if (m_inLiquid) {
            m_velocity += (SWIMVELOCITY + GRAVITY * dT) * glm::vec3(0.f, 1.f, 0.f);
        }
        else if (m_isGrounded) {
            m_velocity += (JUMPVELOCITY + GRAVITY * dT) * glm::vec3(0.f, 1.f, 0.f);
        }
    }

    // update acceleration and velocity
    if (glm::length(direction) != 0.f) {
        m_acceleration = glm::normalize(direction) * ROA;
    }
    m_velocity += m_acceleration * dT;
 }

void Player::computePhysics(float dT, const Terrain &terrain) {

    // diminish the velocity and also enforce a terminal velocity
    m_velocity *= 0.95f;

    // determine the player's displacement vector
    glm::vec3 rayDirection = m_velocity * dT;

    // collision detection and gravity enabled when player is not in fly mode
    if (!m_flyMode) {
        // apply gravity to the player if they are not in fly mode and not grounded
        updateGrounded(terrain);
        if (!m_isGrounded) m_velocity[1] -= GRAVITY * dT;

        // 2/3 the player's velocity if they are in liquid
        updateLiquid(terrain);
        if (m_inLiquid) m_velocity *= 0.67f;

        // store all the vertices to cast rays from
        std::vector<glm::vec3> rayOrigins;
        for (int i = 0; i < 3; i++) {
            rayOrigins.push_back(m_position + glm::vec3(PLAYERWIDTH, i, PLAYERWIDTH));
            rayOrigins.push_back(m_position + glm::vec3(-PLAYERWIDTH, i, PLAYERWIDTH));
            rayOrigins.push_back(m_position + glm::vec3(PLAYERWIDTH, i, -PLAYERWIDTH));
            rayOrigins.push_back(m_position + glm::vec3(-PLAYERWIDTH, i, -PLAYERWIDTH));
        }

        // axes in which we grid march from each vertex
        std::vector<glm::vec3> rayComponentAxesVectors = {
            glm::vec3(rayDirection[0], 0.f, 0.f),
            glm::vec3(0.f, rayDirection[1], 0.f),
            glm::vec3(0.f, 0.f, rayDirection[2]),
        };

        // values used for collision response
        glm::vec3 min_t = glm::vec3(1.f);
        glm::ivec3 blockHit = glm::ivec3(0);
        float currDistTraveled = 0.f;

        // collision detection along all three cardinal axes
        for (int axis = 0; axis < 3; axis++) {
            for (unsigned long long i = 0; i < rayOrigins.size(); i++) {
                if (rayDirection[axis] == 0.f) {
                    continue;
                }

                if(Utils::gridMarchIgnoreNonSolidBlocks(rayOrigins[i], rayComponentAxesVectors[axis], terrain, &currDistTraveled, &blockHit)) {

                    // slightly downscale min dist traveled along the current axis to avoid rounding errors
                    // normal value causes us to phase into the terrain
                    min_t[axis] = glm::min(min_t[axis], currDistTraveled * 0.99f);

                    // collision response against solid blocks
                    if (min_t[axis] < EPSILON) {
                        min_t[axis] = 0.f;
                        m_velocity[axis] = 0.f;
                        rayDirection[axis] = 0.f;
                    }
                    else {
                        rayDirection[axis] = glm::sign(rayDirection[axis]) * min_t[axis];
                    }
                }
            }

            // move all bound checking vertices along the current axis
            for (unsigned long long i = 0; i < rayOrigins.size(); i++) {
                rayOrigins[i][axis] += rayDirection[axis];
            }
        }
    }
    // move along all the axes
    moveRightGlobal(rayDirection[0]);
    moveUpGlobal(rayDirection[1]);
    moveForwardGlobal(rayDirection[2]);
}


void Player::updateGrounded(const Terrain &terrain) {
    // check if the player is grounded
    // add small epsilon to avoid rounding errors
    std::vector<glm::vec3> rayOrigins = { m_position + glm::vec3(PLAYERWIDTH, EPSILON, PLAYERWIDTH),
                                          m_position + glm::vec3(PLAYERWIDTH, EPSILON, -PLAYERWIDTH),
                                          m_position + glm::vec3(-PLAYERWIDTH, EPSILON, PLAYERWIDTH),
                                          m_position + glm::vec3(-PLAYERWIDTH, EPSILON, -PLAYERWIDTH),
                                        };

    // grid march downwards from the bottom corners of our two block character
    float distFromGround = 1.f;
    glm::vec3 downVector = glm::vec3(0.f, -1.f, 0.f);
    for (unsigned long long i = 0; i < rayOrigins.size(); i++) {
        float currDistTraveled = 0.f;
        glm::ivec3 blockHit = glm::ivec3(0);
        if(Utils::gridMarchIgnoreNonSolidBlocks(rayOrigins[i], downVector, terrain, &currDistTraveled, &blockHit)) {
            distFromGround = glm::min(distFromGround, currDistTraveled);
        }
    }

    // dist from be < 11 * epsilon because we already added epsilon to original y coordinate
    m_isGrounded = distFromGround < 11.f * EPSILON;
}

void Player::updateLiquid(const Terrain &terrain) {
    // store all the vertices of our player model
    std::vector<glm::vec3> rayOrigins;
    for (int i = 0; i < 3; i++) {
        rayOrigins.push_back(m_position + glm::vec3(PLAYERWIDTH, i, PLAYERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-PLAYERWIDTH, i, PLAYERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(PLAYERWIDTH, i, -PLAYERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-PLAYERWIDTH, i, -PLAYERWIDTH));
    }

    // check if any of the character vertices are in a liquid block
    for (glm::vec3 &pos : rayOrigins) {
        try {
            BlockType type = terrain.getBlockAt(glm::floor(pos[0]), glm::floor(pos[1]), glm::floor(pos[2]));
            if (type == LAVA || type == WATER) {
                m_inLiquid = true;
                return;
            }
        } catch (std::out_of_range) {}
    }
    m_inLiquid = false;
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}
void Player::toggleFlightMode() {
    m_flyMode = !m_flyMode;
}


QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}


