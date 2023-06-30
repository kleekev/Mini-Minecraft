#pragma once

#include "entity.h"
#include "cube.h"
#include "node.h"
#include "smartpointerhelp.h"
#include "chunkhelper.h"

// the different UV dimensions of the creeper texture blocks
#define UV8 0.0625f
#define UV12 0.09375f
#define UV16 0.125f
#define UV24 0.1875f


enum CreeperCubeType : unsigned char {
    HEAD, BODY, LEG
};

enum LegDirection : unsigned char {
    FORWARD, BACKWARD
};

class CreeperCube: public Cube
{
private:
    CreeperCubeType cubeType;
public:
    CreeperCube(OpenGLContext* context, CreeperCubeType cubeType);
    virtual ~CreeperCube(){}
    void createVBOdata() override;
};

class Creeper : public Entity
{
private:
    glm::vec3 m_velocity;

    glm::vec3 m_walkDirection;

    const Terrain &mcr_terrain;

    CreeperCube& m_head, m_body, m_leg;

    uPtr<Node> m_root;

    // rotation nodes for head and body
    RotateNode *rotHead, *rotBody;

    // rotation nodes for front right, front left, back right, back left legs
    RotateNode *rotFR, *rotFL, *rotBR, *rotBL;

    // directions in which the leg is currently moving
    LegDirection directionFR, directionFL, directionBR, directionBL;

    bool m_isGrounded, m_hitWall;

    // performs collision detection and updates position
    void computePhysics(float dT, const Terrain &terrain);

    // update the creeper's m_isGounded variable
    void updateGrounded(const Terrain &terrain);

    // update the creeper's m_inLiquid variable
    void updateLiquid(const Terrain &terrain);

    // animate legs and update orientation of creeper body and head
    void updateLegOrientation(RotateNode *leg, LegDirection& legDirection, float dT);

    // stop the legs from rotating
    void stopCreeperLegs();

    // make the body face the direction specified
    void updateBodyOrientation(glm::vec2 direction);

    // make the head face the direction specified
    void updateHeadOrientation(glm::vec3 direction);

    uPtr<Node> createSceneGraph();

    void traverseSceneGraph(ShaderProgram& prog, const uPtr<Node> &root, glm::mat4 T);

public:
    // used to delete creeper if it ever touches water
    bool m_inLiquid;

    Creeper(glm::vec3 pos, const Terrain &terrain,
            CreeperCube& head, CreeperCube& body, CreeperCube& leg);
    virtual ~Creeper() override;

    // override just because we need to for purely virtual functions
    // this function does nothing
    void tick(float dT, InputBundle &input) override;

    void tick(float dT, const glm::vec3 playerPosition, int elapsedTicks);

    // draws the creeper
    void draw(ShaderProgram& prog);
};
