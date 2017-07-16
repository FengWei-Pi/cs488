#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "Animation.hpp"
#include "Keyframe.hpp"
#include "Player.hpp"
#include "Visitor.hpp"
#include "SceneNodeFunctor.hpp"
#include "Collidable.hpp"
#include "Platform.hpp"
#include "StateManager.hpp"
#include "Camera.hpp"
#include "Level.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <chrono>
#include <set>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <AL/alut.h>

struct LightSource {
  glm::vec3 position;
  glm::vec3 rgbIntensity;
};

class A5 : public CS488Window {
public:
  A5();
  virtual ~A5();

protected:
  virtual void init() override;
  virtual void appLogic() override;
  virtual void guiLogic() override;
  virtual void draw() override;
  virtual void cleanup() override;

  //-- Virtual callback methods
  virtual bool cursorEnterWindowEvent(int entered) override;
  virtual bool mouseMoveEvent(double xPos, double yPos) override;
  virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
  virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
  virtual bool windowResizeEvent(int width, int height) override;
  virtual bool keyInputEvent(int key, int action, int mods) override;

  //-- One time initialization methods:
  std::shared_ptr<SceneNode> readLuaSceneFile(const std::string& filename);
  void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
  void initViewMatrix();
  void initLightSources();

  void initPerspectiveMatrix();

  glm::mat4 m_perpsective;
  glm::mat4 m_view;

  LightSource m_light;

  //-- GL resources for mesh geometry data:
  GLuint m_vao_meshData;
  GLuint m_vbo_vertexPositions;
  GLuint m_vbo_vertexNormals;
  GLuint m_vbo_uvCoords;
  ShaderProgram m_shader;

  // BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
  // object. Each BatchInfo object contains an index offset and the number of indices
  // required to render the mesh with identifier MeshId.
  BatchInfoMap m_batchInfoMap;

  std::string m_luaSceneFile;

  std::shared_ptr<SceneNode> blockSceneNode;
  std::shared_ptr<SceneNode> puppetSceneNode;
private:
  struct Mouse {
    double x;
    double y;
    double prevX;
    double prevY;
    bool isControllingMinimap = false;
    bool isRightButtonPressed = false;
    bool isLeftButtonPressed = false;
    bool isMiddleButtonPressed = false;
  } mouse;

  Camera gameCamera;
  Camera minimapCamera;

  double animationStartTime;
  Animation playerWalkingAnimation;
  Animation playerStandingAnimation;
  Animation playerPreparingToJumpAnimation;
  Animation playerJumpingAnimation;
  Animation* currentAnimation;

  std::set<int> keysPressed;

  Player player;

  bool isKeyPressed(int key);
  glm::vec3 calculatePlayerInputVelocity();

  /**
   * Shadow Map
   */

  GLuint depthFramebuffer;
  GLuint renderedTexture;
  GLuint depthTexture;
  unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
  ShaderProgram m_shader_depth;
  ShaderProgram m_shader_quad;
  ShaderProgram m_shader_skybox;

  GLuint quad_vertexArray;
  GLuint quad_vertexbuffer;
  GLuint depthRenderBuffer;

  struct World {
    glm::vec3 F_g; // Gravitational force
    glm::vec3 F_wind; // Wind force
    float ufs; // co-efficient of static friction
    float ufk; // co-efficient of kinetic friction
    World(glm::vec3, glm::vec3, float, float);
  } world;

  static void initShaderProgram(ShaderProgram& program, const std::string& name);

  enum PlayerState {
    INIT,
    WALKING,
    PREPARING_TO_JUMP,
    STANDING,
    AIRBORN,
  };

  StateManager<PlayerState> playerStateManager;

  static GLuint loadCubemap(std::vector<std::string> faces);
  GLuint skyboxTexture;
  GLuint skyboxVAO;
  GLuint skyboxVBO;

  void fillDepthTexture(const glm::mat4& LightProjection, const glm::mat4& LightView);
  void renderTextureToQuad(GLuint textureId, const glm::vec2& position, const glm::vec2& size);
  void renderSceneNormally(
    const glm::mat4& Projection,
    const glm::mat4& View,
    const glm::mat4& LightProjection,
    const glm::mat4& LightView
  );
  void renderPuppet(SceneNodeFunctor<void, glm::mat4>& renderer);
  void renderPlatform(Platform& block, SceneNodeFunctor<void, glm::mat4>& renderer);

  void renderSkybox(const glm::mat4& Projection, const glm::mat4& View);
  glm::mat4 createMinimapPerspectiveMatrix();
  glm::mat4 createMinimapViewMatrix();

  Level level;

  Platform* ground = nullptr;
  float playerJumpVelocity = 0;

  struct GameState {
    bool isPlaying = true;
    int lives = 1;
    int score = 0;
  } gameState;

  bool isMouseOnMinimap() const;
  void refreshMinimapViewportDimensions();

  struct Viewport {
    glm::vec2 position{20, 20};
    glm::vec2 size;
  } minimapViewport;

  /** OpenAL stuff */
  ALCdevice *device;
  ALCcontext *context;
  ALuint playerSource;
  ALuint windSource;
  void repositionAndReorientListener();
  void checkOpenALErrors();
  void repositionPlayerSource();
  void repositionWindSource();
  void updateWindSourceGain();

  int oldKeyframeId = -1;

  std::map<std::string, ALuint> soundBuffers;
  void playSoundWithSource(ALuint source, std::string filename);

  // Texture mapping
  static GLuint createTexture2D(std::string filename);

  GLuint tileTexture;
  GLuint darkTileTexture;

  void resetPlayer();
};
