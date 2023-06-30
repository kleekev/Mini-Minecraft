#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include "utils.h"
#include <QImage>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this), m_progSky(this),
      m_progLambert(this), m_progFlat(this)/*, m_progInstanced(this)*/, m_geomQuad(this),
      m_waterPostProcessShader(this), m_lavaPostProcessShader(this), m_noOpPostProcessShader(this),
      m_frameBuffer(this, 0, 0, 0), m_terrain(this), m_player(glm::vec3(48.f, 200.f, 48.f), m_terrain),
      m_head(this, HEAD), m_body(this, BODY), m_leg(this, LEG), creepers(),
      m_time(0), m_seconds(0)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    // set the initial epoch time
    m_prevFrameTime = QDateTime::currentMSecsSinceEpoch();

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Enable alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // create instances of creeper body parts
    m_head.createVBOdata();
    m_body.createVBOdata();
    m_leg.createVBOdata();

    // Create sky shader
    m_progSky.create(":glsl/passthrough.vert.glsl", ":glsl/sky.frag.glsl");
    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // create all the post processing shaders
    m_noOpPostProcessShader.create(":glsl/passthrough.vert.glsl", ":glsl/noOp.frag.glsl");
    m_waterPostProcessShader.create(":glsl/passthrough.vert.glsl", ":glsl/water.frag.glsl");
    m_lavaPostProcessShader.create(":glsl/passthrough.vert.glsl", ":glsl/lava.frag.glsl");

    m_geomQuad.createVBOdata();
    m_frameBuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    m_frameBuffer.create();

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // Create Creeper texture
    glGenTextures(1, &creeperTextureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, creeperTextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    QImage creeperImg(":/textures/creeper.png");
    creeperImg = creeperImg.convertToFormat(QImage::Format_ARGB32);
    creeperImg = creeperImg.mirrored();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, creeperImg.width(), creeperImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, creeperImg.bits());

    // Create Block Texture
    glGenTextures(1, &textureHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    QImage img(":/textures/minecraft_textures_all.png");
    img = img.convertToFormat(QImage::Format_ARGB32);
    img = img.mirrored();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    m_frameBuffer.resize(w, h, this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    m_progSky.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));

    // Upload the view-projection matrix and its inverse to our shaders (i.e. onto the graphics card)
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    glm::mat4 inverse_viewproj = glm::inverse(viewproj);

    m_progLambert.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progLambert.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(inverse_viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    // Calculate change in time since last tick
    long long currTime = QDateTime::currentMSecsSinceEpoch();

    // Rescale from msecs to seconds
    float dT = (currTime - m_prevFrameTime) * 0.001f;
    m_prevFrameTime = currTime;

    // call tick on all our entities
    m_player.tick(dT, m_inputs);
    for(const auto& creeper : creepers) {
        creeper->tick(dT, m_player.mcr_position, m_time);
    }

    // remove creepers that are in liquid
    creepers.erase(std::remove_if(creepers.begin(), creepers.end(),
                                  [](auto& creeper) { return creeper->m_inLiquid; }),
                   creepers.end());

    // generate new terrain based on player position
    m_terrain.GenerateNew(m_player.mcr_position);

    // Set time in the shaders
    m_progLambert.setTime(m_time);
    m_progSky.setTime(m_time);
    m_waterPostProcessShader.setTime(m_time);
    m_lavaPostProcessShader.setTime(m_time);
    m_time++;

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
    emit sig_sendPlayerHumid(QString::fromStdString("( " + std::to_string(m_terrain.getChunkAt(chunk.x, chunk.y)->humidity) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    m_frameBuffer.bindFrameBuffer();

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // send view proj to shaders
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progFlat.setViewProjMatrix(viewproj);
    m_progFlat.setCamPos(m_player.mcr_camera.mcr_position);
    m_progLambert.setViewProjMatrix(viewproj);
    m_progLambert.setCamPos(m_player.mcr_camera.mcr_position);

    // send inverse view proj to sky shader
    glm::mat4 inverse_viewproj = glm::inverse(viewproj);
    m_progSky.setViewProjMatrix(inverse_viewproj);
    m_progSky.setCamPos(m_player.mcr_camera.mcr_position);

    // draw the sky
    m_progSky.draw(m_geomQuad);

    // bind minecraft texture pack to texture slot 0 and draw the terrain
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    renderTerrain();

    // bind creeper texture and draw the creepers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, creeperTextureHandle);
    for(const auto& creeper : creepers) {
        creeper->draw(m_progLambert);
    }

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progLambert.setModelMatrix(glm::mat4());

    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
    performPostprocessRenderPass();
}

void MyGL::renderTerrain() {
    m_terrain.draw(&m_progLambert);
}


void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind rendered texture to Texture Slot 1
    m_frameBuffer.bindToTextureSlot(1);


    // find the type of the current block that our camera is in
    glm::vec3 cameraPos = m_player.mcr_camera.mcr_position;
    BlockType cameraBlock = EMPTY;
    try {
        cameraBlock = m_terrain.getBlockAt(glm::floor(cameraPos[0]), glm::floor(cameraPos[1]), glm::floor(cameraPos[2]));
    }
    catch (std::out_of_range) {
        cameraBlock = EMPTY;
    }

    // conditionally perform preprocessing based on camera block
    if (cameraBlock == WATER) {
        m_waterPostProcessShader.draw(m_geomQuad, 1);
    }
    else if (cameraBlock == LAVA) {
        m_lavaPostProcessShader.draw(m_geomQuad, 1);
    }
    else {
        m_noOpPostProcessShader.draw(m_geomQuad, 1);
    }
}



void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    }
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_player.toggleFlightMode();
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = true;
    } else if (e->key() == Qt::Key_C) {
        spawnCreeper();
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    float scaledXPos = 2.f * ((float) e->pos().x() / (float) width()) - 1.f;
    m_inputs.mouseX = scaledXPos;
    float scaledYPos = 2.f * ((float) e->pos().y() / (float) height()) - 1.f;
    m_inputs.mouseY = scaledYPos;
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        removeBlock();
    }
    if (e->button() == Qt::RightButton) {
        placeBlock();
    }
}


void MyGL::placeBlock() {
    const Camera& cam = m_player.mcr_camera;
    float dist = 0.f;
    glm::ivec3 blockHit = glm::ivec3(0);

    // check if our camera forward vector collided with any blocks in the terrain 3 units away
    if (Utils::gridMarch(cam.mcr_position, 3.f * cam.mcr_forward, m_terrain, &dist, &blockHit)) {

        // find the block face in which we hit the block
        glm::vec3 intersectionPoint = cam.mcr_position + dist * cam.mcr_forward;

        // find the neighboring block that is closest to our intersection point
        glm::ivec3 newBlockLocation;
        glm::vec3 blockHitCenter = glm::vec3(blockHit) + glm::vec3(0.5f, 0.5f, 0.5f);
        float minNeighborBlockDist = 2.f;
        for (int i = 0; i < 3; i++) {
            glm::vec3 negBlockCenter = blockHitCenter;
            negBlockCenter[i] -= 1.f;
            glm::vec3 posBlockCenter = blockHitCenter;
            posBlockCenter[i] += 1.f;
            float negDist = glm::distance(negBlockCenter, intersectionPoint);
            float posDist = glm::distance(posBlockCenter, intersectionPoint);
            if (negDist < minNeighborBlockDist) {
                newBlockLocation = glm::floor(negBlockCenter);
                minNeighborBlockDist = negDist;
            }
            if (posDist < minNeighborBlockDist) {
                newBlockLocation = glm::floor(posBlockCenter);
                minNeighborBlockDist = posDist;
            }
        }

        // don't allow the block to be placed if player is obscuring the new block
        // we assume the player blocks are all the blocks in which the player has a vertex in
        const glm::vec3 &playerPos = m_player.mcr_position;
        for (int i = 0; i < 3; i++) {
            if (glm::ivec3(playerPos[0] + 0.5f, playerPos[1] + i, playerPos[2] + 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] - 0.5f, playerPos[1] + i, playerPos[2] + 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] + 0.5f, playerPos[1] + i, playerPos[2] - 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] - 0.5f, playerPos[1] + i, playerPos[2] - 0.5f) == newBlockLocation) {
                return;
            }
        }
        // don't allow block to be placed in the location of a nonempty block
        if (m_terrain.getBlockAt(newBlockLocation[0], newBlockLocation[1], newBlockLocation[2]) != EMPTY) {
            return;
        }
        m_terrain.setBlockAt(newBlockLocation[0], newBlockLocation[1], newBlockLocation[2], STONE);
    }
}


void MyGL::removeBlock() {
    const Camera& cam = m_player.mcr_camera;
    float dist = 0.f;
    glm::ivec3 blockToRemove = glm::ivec3(0);
    if (Utils::gridMarch(cam.mcr_position, 3.f * cam.mcr_forward, m_terrain, &dist, &blockToRemove)) {

        // prohibit bedrock from being removed
        BlockType toRemove = m_terrain.getBlockAt(blockToRemove[0], blockToRemove[1], blockToRemove[2]);
        if (toRemove != BEDROCK) {
            m_terrain.setBlockAt(blockToRemove[0], blockToRemove[1], blockToRemove[2], EMPTY);
        }
    }
}

void MyGL::spawnCreeper() {
    const Camera& cam = m_player.mcr_camera;
    float dist = 0.f;
    glm::ivec3 blockHit = glm::ivec3(0);

    // check if our camera forward vector collided with any blocks in the terrain 3 units away
    if (Utils::gridMarch(cam.mcr_position, 3.f * cam.mcr_forward, m_terrain, &dist, &blockHit)) {

        // find the block face in which we hit the block
        glm::vec3 intersectionPoint = cam.mcr_position + dist * cam.mcr_forward;

        // find the neighboring block that is closest to our intersection point
        glm::ivec3 newBlockLocation;
        glm::vec3 blockHitCenter = glm::vec3(blockHit) + glm::vec3(0.5f, 0.5f, 0.5f);
        float minNeighborBlockDist = 2.f;
        for (int i = 0; i < 3; i++) {
            glm::vec3 negBlockCenter = blockHitCenter;
            negBlockCenter[i] -= 1.f;
            glm::vec3 posBlockCenter = blockHitCenter;
            posBlockCenter[i] += 1.f;
            float negDist = glm::distance(negBlockCenter, intersectionPoint);
            float posDist = glm::distance(posBlockCenter, intersectionPoint);
            if (negDist < minNeighborBlockDist) {
                newBlockLocation = glm::floor(negBlockCenter);
                minNeighborBlockDist = negDist;
            }
            if (posDist < minNeighborBlockDist) {
                newBlockLocation = glm::floor(posBlockCenter);
                minNeighborBlockDist = posDist;
            }
        }

        // don't allow the block to be placed if player is obscuring the new block
        // we assume the player blocks are all the blocks in which the player has a vertex in
        const glm::vec3 &playerPos = m_player.mcr_position;
        for (int i = 0; i < 3; i++) {
            if (glm::ivec3(playerPos[0] + 0.5f, playerPos[1] + i, playerPos[2] + 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] - 0.5f, playerPos[1] + i, playerPos[2] + 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] + 0.5f, playerPos[1] + i, playerPos[2] - 0.5f) == newBlockLocation
             || glm::ivec3(playerPos[0] - 0.5f, playerPos[1] + i, playerPos[2] - 0.5f) == newBlockLocation) {
                return;
            }
        }
        // don't allow block to be placed in the location of a nonempty block
        if (m_terrain.getBlockAt(newBlockLocation[0], newBlockLocation[1], newBlockLocation[2]) != EMPTY) {
            return;
        }
        uPtr<Creeper> newCreep = mkU<Creeper>(glm::vec3(newBlockLocation[0] + 0.5f, newBlockLocation[1], newBlockLocation[2] + 0.5f), m_terrain, m_head, m_body, m_leg);
        creepers.push_back(std::move(newCreep));
    }
}
