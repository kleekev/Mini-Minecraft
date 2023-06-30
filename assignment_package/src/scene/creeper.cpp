#include "creeper.h"
#include "utils.h"
#include "cube.h"
#include "noise.h"
#include <random>
#include <iostream>
#include <cmath>

#define ROA 5.f
#define GRAVITY 90.f
#define JUMPVELOCITY 7.5f
#define EPSILON 0.001f
#define CREEPERWIDTH 0.375f

static const int CUB_IDX_COUNT = 36;
static const int CUB_VERT_COUNT = 24;

// Vertex Order: upper right, lower right, lower left, upper left
void addFaceUVs(glm::vec4 (&cub_uv)[CUB_VERT_COUNT], int startIndex, glm::vec2 offset, float width, float height) {
    cub_uv[startIndex] = glm::vec4(glm::vec2(width, height) + offset, 0.f, 0.f); // UR
    cub_uv[startIndex+1] = glm::vec4(glm::vec2(width, 0.f) + offset, 0.f, 0.f); // LR
    cub_uv[startIndex+2] = glm::vec4(offset, 0.f, 0.f); // LL
    cub_uv[startIndex+3] = glm::vec4(glm::vec2(0.f, height) + offset, 0.f, 0.f); // UL
}

// Face order: front, right, left, back, top, bottom
void createHeadUVs(glm::vec4 (&cub_uv)[CUB_VERT_COUNT])
{
    // lower left corner uv coordinates in face order
    glm::vec2 uvOffsets[6] = {glm::vec2(1.f * UV16, 2.f * UV16 + 0.5),  // front
                              glm::vec2(0.f * UV16, 2.f * UV16 + 0.5),  // right
                              glm::vec2(2.f * UV16, 2.f * UV16 + 0.5),  // left
                              glm::vec2(3.f * UV16, 2.f * UV16 + 0.5),  // back
                              glm::vec2(1.f * UV16, 3.f * UV16 + 0.5),  // top
                              glm::vec2(2.f * UV16, 3.f * UV16 + 0.5)}; // bottom

    for (int i = 0; i < CUB_VERT_COUNT / 4; i++) {
        addFaceUVs(cub_uv, 4 * i, uvOffsets[i], UV16, UV16);
    }
}

void createBodyUVs(glm::vec4 (&cub_uv)[CUB_VERT_COUNT])
{
    // lower left corner uv coordinates in face order
    glm::vec2 uvOffsets[6] = {glm::vec2(5.f * UV8, 0.f * UV24 + 0.5),  // front
                              glm::vec2(4.f * UV8, 0.f * UV24 + 0.5),  // right
                              glm::vec2(7.f * UV8, 0.f * UV24 + 0.5),  // left
                              glm::vec2(8.f * UV8, 0.f * UV24 + 0.5),  // back
                              glm::vec2(5.f * UV8, 1.f * UV24 + 0.5),  // top
                              glm::vec2(7.f * UV8, 1.f * UV24 + 0.5)}; // bottom

    addFaceUVs(cub_uv, 0, uvOffsets[0], UV16, UV24); // front
    addFaceUVs(cub_uv, 4, uvOffsets[1], UV8, UV24); // right
    addFaceUVs(cub_uv, 8, uvOffsets[2], UV8, UV24); // left
    addFaceUVs(cub_uv, 12, uvOffsets[3], UV16, UV24); // back
    addFaceUVs(cub_uv, 16, uvOffsets[4], UV16, UV8); // top
    addFaceUVs(cub_uv, 20, uvOffsets[5], UV16, UV8); // bottom
}

void createLegUVs(glm::vec4 (&cub_uv)[CUB_VERT_COUNT])
{
    // lower left corner uv coordinates in face order
    glm::vec2 uvOffsets[6] = {glm::vec2(1.f * UV8, 1.f * UV12 + 0.5),  // front
                              glm::vec2(0.f * UV8, 1.f * UV12 + 0.5),  // right
                              glm::vec2(2.f * UV8, 1.f * UV12 + 0.5),  // left
                              glm::vec2(3.f * UV8, 1.f * UV12 + 0.5),  // back
                              glm::vec2(1.f * UV8, 2.f * UV12 + 0.5),  // top
                              glm::vec2(2.f * UV8, 2.f * UV12 + 0.5)}; // bottom

    addFaceUVs(cub_uv, 0, uvOffsets[0], UV8, UV12); // front
    addFaceUVs(cub_uv, 4, uvOffsets[1], UV8, UV12); // right
    addFaceUVs(cub_uv, 8, uvOffsets[2], UV8, UV12); // left
    addFaceUVs(cub_uv, 12, uvOffsets[3], UV8, UV12); // back
    addFaceUVs(cub_uv, 16, uvOffsets[4], UV8, UV8); // top
    addFaceUVs(cub_uv, 20, uvOffsets[5], UV8, UV8); // bottom
}

CreeperCube::CreeperCube(OpenGLContext* context, CreeperCubeType cubeType)
    :Cube(context), cubeType(cubeType)
{}


void CreeperCube::createVBOdata()
{
    Cube::createVBOdata();
    const int cubeVertexCount = 24;
    glm::vec4 cubeUVs[cubeVertexCount];

    if (cubeType == HEAD) {
        createHeadUVs(cubeUVs);
    }
    else if (cubeType == BODY) {
        createBodyUVs(cubeUVs);
    } else if (cubeType == LEG) {
        createLegUVs(cubeUVs);
    }

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, CUB_VERT_COUNT * sizeof(glm::vec4), cubeUVs, GL_STATIC_DRAW);

}


Creeper::Creeper(glm::vec3 pos, const Terrain &terrain,
                 CreeperCube &head, CreeperCube &body, CreeperCube &leg)
    :Entity(pos), m_velocity(0, 0, 0), m_walkDirection(0, 0, 0), mcr_terrain(terrain),
      m_head(head), m_body(body), m_leg(leg), m_root(nullptr), rotHead(nullptr), rotBody(nullptr),
      rotFR(nullptr), rotFL(nullptr), rotBR(nullptr), rotBL(nullptr),
      m_isGrounded(false), m_inLiquid(false), m_hitWall(false)
{
    m_root = createSceneGraph();
}

Creeper::~Creeper()
{}


void Creeper::tick(float dT, InputBundle &input)
{}

void Creeper::tick(float dT, const glm::vec3 playerPosition, int elapsedTicks) {
    glm::vec3 playerDirection = playerPosition - m_position;

    float distance = glm::length(playerDirection);

    if (distance != 0.f) {
        playerDirection = glm::normalize(playerDirection);
    }

    if (m_hitWall && m_isGrounded) {
        m_velocity[1] += JUMPVELOCITY + GRAVITY * dT;
    }

    // if player within distance 15, always walk towards the player
    if (distance < 15.f) {
        m_walkDirection = playerDirection;
        m_walkDirection[1] = 0.f;
        if (glm::length(m_walkDirection) != 0.f) {
            m_walkDirection = glm::normalize(m_walkDirection);
        }
        // make creeper look at the player
        updateHeadOrientation(playerDirection);
    }
    // pick a new direction for the creeper to walk in every 1000 ticks
    else if (elapsedTicks % 1000 == 0) {
        float x_direction = Noise::perlinNoise2D(glm::vec2(m_position[0], m_position[1]) + (123.25f * elapsedTicks));
        float z_direction = Noise::perlinNoise2D(glm::vec2(m_position[1], m_position[2]) + (342.39f * elapsedTicks));
        m_walkDirection = glm::vec3(x_direction, 0.f, z_direction);
        if (glm::length(m_walkDirection) != 0.f) {
            m_walkDirection = glm::normalize(m_walkDirection);
        }
        // make creeper look in the direction it is walking
        rotHead->phi = 0.f;
    }

    // stop creeper x, z movement when it is within 1.5 distance of player
    if (distance < 1.5f) {
        m_velocity[0] = 0.f;
        m_velocity[2] = 0.f;
        stopCreeperLegs();
    }
    else {
        m_velocity += m_walkDirection * ROA * dT;

        // update leg orientations to simulate animation
        updateLegOrientation(rotFR, directionFR, dT);
        updateLegOrientation(rotFL, directionFL, dT);
        updateLegOrientation(rotBR, directionBR, dT);
        updateLegOrientation(rotBL, directionBL, dT);
    }

    // diminish the velocity and also enforce a terminal velocity
    m_velocity *= 0.95f;

    // perform collision detection and update creeper position
    computePhysics(dT, mcr_terrain);

    // update the body to face the direction it is walking
    updateBodyOrientation(glm::vec2(m_walkDirection[0], m_walkDirection[2]));
}


void Creeper::computePhysics(float dT, const Terrain &terrain) {

    // determine the creeper's displacement vector
    glm::vec3 rayDirection = m_velocity * dT;

    // apply gravity to the creeper if they are not not grounded
    updateGrounded(terrain);
    if (!m_isGrounded) m_velocity[1] -= GRAVITY * dT;

    // set the creeper's m_inLiquid variable so that it can be deleted
    updateLiquid(terrain);

    // store all the vertices to cast rays from
    std::vector<glm::vec3> rayOrigins;
    for (int i = 0; i < 3; i++) {
        rayOrigins.push_back(m_position + glm::vec3(CREEPERWIDTH, i, CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-CREEPERWIDTH, i, CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(CREEPERWIDTH, i, -CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-CREEPERWIDTH, i, -CREEPERWIDTH));
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

    m_hitWall = false;
    // collision detection along all three cardinal axes
    for (int axis = 0; axis < 3; axis++) {
        for (unsigned long long i = 0; i < rayOrigins.size(); i++) {
            if (rayDirection[axis] == 0.f) {
                continue;
            }

            if(Utils::gridMarchIgnoreNonSolidBlocks(rayOrigins[i], rayComponentAxesVectors[axis], terrain, &currDistTraveled, &blockHit)) {

                if (axis != 1) {
                    m_hitWall = true;
                }
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

    // move along all the axes
    moveRightGlobal(rayDirection[0]);
    moveUpGlobal(rayDirection[1]);
    moveForwardGlobal(rayDirection[2]);
    // update the translation matrix at the root of creeper scene graph
    TranslateNode* t = dynamic_cast<TranslateNode*>(m_root.get());
    t->x_translation = m_position[0];
    t->y_translation = m_position[1] + 0.75f;
    t->z_translation = m_position[2];
}

// update the creeper's m_isGrounded variable
void Creeper::updateGrounded(const Terrain &terrain) {
    // check if the creeper is grounded
    // add small epsilon to avoid rounding errors
    std::vector<glm::vec3> rayOrigins = { m_position + glm::vec3(CREEPERWIDTH, EPSILON, CREEPERWIDTH),
                                          m_position + glm::vec3(CREEPERWIDTH, EPSILON, -CREEPERWIDTH),
                                          m_position + glm::vec3(-CREEPERWIDTH, EPSILON, CREEPERWIDTH),
                                          m_position + glm::vec3(-CREEPERWIDTH, EPSILON, CREEPERWIDTH),
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


// update the creeper's m_inLiquid variable
void Creeper::updateLiquid(const Terrain &terrain) {
    // store all the vertices of our creeper model
    std::vector<glm::vec3> rayOrigins;
    for (int i = 0; i < 3; i++) {
        rayOrigins.push_back(m_position + glm::vec3(CREEPERWIDTH, i, CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-CREEPERWIDTH, i, CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(CREEPERWIDTH, i, -CREEPERWIDTH));
        rayOrigins.push_back(m_position + glm::vec3(-CREEPERWIDTH, i, -CREEPERWIDTH));
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

void Creeper::draw(ShaderProgram& prog) {
    traverseSceneGraph(prog, m_root, glm::mat4(1));
}

void Creeper::traverseSceneGraph(ShaderProgram& prog, const uPtr<Node> &root, glm::mat4 T) {
    // traverse the scene graph and draw the creeper components
    T = T * root->getTransformation();
    for (const uPtr<Node> &child : root->children) {
        traverseSceneGraph(prog, child, T);
    }
    if (root->geometry != nullptr) {
        prog.setModelMatrix(T);
        prog.draw(*root->geometry);
    }
}

void Creeper::updateLegOrientation(RotateNode *leg, LegDirection& legDirection, float dT) {
    float angleUpdate = 25 * dT;
    if (legDirection == FORWARD) {
        leg->phi += angleUpdate;
    }
    else {
        leg->phi -= angleUpdate;
    }
    leg->phi = glm::clamp(leg->phi, -10.f, 10.f);

    if (leg->phi >= 9.99f) {
        legDirection = BACKWARD;
    }
    else if (leg->phi <= -9.99f){
        legDirection = FORWARD;
    }
}

void Creeper::stopCreeperLegs() {
    rotFR->phi = 0.f;
    directionFR = FORWARD;
    rotFL->phi = 0.f;
    directionFL = BACKWARD;
    rotBR->phi = 0.f;
    directionBR = FORWARD;
    rotBL->phi = 0.f;
    directionBL = BACKWARD;
}

void Creeper::updateBodyOrientation(glm::vec2 direction) {
    float thetaUpdate = std::atan2(direction[0], direction[1]) * 57.2958f;
    rotBody->theta = thetaUpdate;
}

// call this after calling update body
void Creeper::updateHeadOrientation(glm::vec3 direction) {
    // calculate phi
    float phiUpdate = -std::atan2(direction[1], 1.f) * 57.2958f;
    rotHead->phi = phiUpdate;
}

uPtr<Node> Creeper::createSceneGraph()
{
    // construct the puppet torso
    uPtr<TranslateNode> translateCreeper = mkU<TranslateNode>(m_position[0], m_position[1] + 0.75f, m_position[2]);
    uPtr<RotateNode> rotateCreeper       = mkU<RotateNode>(0.f, 0.f);
    uPtr<TranslateNode> centerCreeper    = mkU<TranslateNode>(-0.25, -0.375, -0.125);
    uPtr<ScaleNode> scaleCreeper         = mkU<ScaleNode>(0.50f, 0.75f, 0.25f);
    scaleCreeper->geometry = &m_body;

    // construct the creeper head
    uPtr<TranslateNode> creeperHead = mkU<TranslateNode>(0.f, 0.625f, 0.f);
    uPtr<RotateNode> rotateHead     = mkU<RotateNode>(0.f, 0.f);
    uPtr<TranslateNode> centerHead  = mkU<TranslateNode>(-0.25, -0.25, -0.25);
    uPtr<ScaleNode> scaleHead       = mkU<ScaleNode>(0.50f, 0.50f, 0.50f);
    scaleHead->geometry = &m_head;


    // construct the four creeper legs
    // Front Right Leg
    uPtr<TranslateNode> legFRTrans  = mkU<TranslateNode>(0.125f, -0.5625f, 0.10f);
    uPtr<RotateNode> legFRRot       = mkU<RotateNode>(0.f, 7.5f);
    uPtr<TranslateNode> legFRCenter = mkU<TranslateNode>(-0.125, -0.1875f, 0.0);
    uPtr<ScaleNode> legFRScale      = mkU<ScaleNode>(0.25f, 0.375f, 0.25f);

    // Front Left Leg
    uPtr<TranslateNode> legFLTrans  = mkU<TranslateNode>(-0.125f, -0.5625f, 0.10f);
    uPtr<RotateNode> legFLRot       = mkU<RotateNode>(0.f, -7.5f);
    uPtr<TranslateNode> legFLCenter = mkU<TranslateNode>(-0.125, -0.1875f, 0.0);
    uPtr<ScaleNode> legFLScale      = mkU<ScaleNode>(0.25f, 0.375f, 0.25f);

    // Back Right Leg
    uPtr<TranslateNode> legBRTrans  = mkU<TranslateNode>(0.125f, -0.5625f, -0.10f);
    uPtr<RotateNode> legBRRot       = mkU<RotateNode>(0.f, -7.5f);
    uPtr<TranslateNode> legBRCenter = mkU<TranslateNode>(-0.125, -0.1875f, -0.25);
    uPtr<ScaleNode> legBRScale      = mkU<ScaleNode>(0.25f, 0.375f, 0.25f);

    // Back Left Leg
    uPtr<TranslateNode> legBLTrans  = mkU<TranslateNode>(-0.125f, -0.5625f, -0.10f);
    uPtr<RotateNode> legBLRot       = mkU<RotateNode>(0.f, 7.5f);
    uPtr<TranslateNode> legBLCenter = mkU<TranslateNode>(-0.125, -0.1875f, -0.25);
    uPtr<ScaleNode> legBLScale      = mkU<ScaleNode>(0.25f, 0.375f, 0.25f);

    legFRScale->geometry   = &m_leg;
    legFLScale->geometry   = &m_leg;
    legBRScale->geometry = &m_leg;
    legBLScale->geometry  = &m_leg;

    // keep track of rotation nodes for animation
    rotHead = rotateHead.get();
    rotBody = rotateCreeper.get();
    rotFR = legFRRot.get();
    rotFL = legFLRot.get();
    rotBR = legBRRot.get();
    rotBL = legBLRot.get();

    // update initial leg travel directions
    directionFR = BACKWARD;
    directionFL = FORWARD;
    directionBR = FORWARD;
    directionFL = BACKWARD;

    // assemble the leg nodes
    legFRCenter->addChild(std::move(legFRScale));
    legFRRot->addChild(std::move(legFRCenter));
    legFRTrans->addChild(std::move(legFRRot));

    legFLCenter->addChild(std::move(legFLScale));
    legFLRot->addChild(std::move(legFLCenter));
    legFLTrans->addChild(std::move(legFLRot));

    legBRCenter->addChild(std::move(legBRScale));
    legBRRot->addChild(std::move(legBRCenter));
    legBRTrans->addChild(std::move(legBRRot));

    legBLCenter->addChild(std::move(legBLScale));
    legBLRot->addChild(std::move(legBLCenter));
    legBLTrans->addChild(std::move(legBLRot));

    // assemble the head nodes
    centerHead->addChild(std::move(scaleHead));
    rotateHead->addChild(std::move(centerHead));
    creeperHead->addChild(std::move(rotateHead));

    // attach head and legs to creeper body
    rotateCreeper->addChild(std::move(creeperHead));
    rotateCreeper->addChild(std::move(legFRTrans));
    rotateCreeper->addChild(std::move(legFLTrans));
    rotateCreeper->addChild(std::move(legBRTrans));
    rotateCreeper->addChild(std::move(legBLTrans));

    // assemble creeper body
    centerCreeper->addChild(std::move(scaleCreeper));
    rotateCreeper->addChild(std::move(centerCreeper));
    translateCreeper->addChild(std::move(rotateCreeper));

    return translateCreeper;
}
