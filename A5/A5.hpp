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

#include <glm/glm.hpp>
#include <memory>
#include <chrono>
#include <set>

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
  void renderSceneGraph(SceneNode &node, Visitor& renderer);

  glm::mat4 m_perpsective;
  glm::mat4 m_view;

  LightSource m_light;

  //-- GL resources for mesh geometry data:
  GLuint m_vao_meshData;
  GLuint m_vbo_vertexPositions;
  GLuint m_vbo_vertexNormals;
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
    bool isRightButtonPressed = false;
    bool isLeftButtonPressed = false;
    bool isMiddleButtonPressed = false;
  } mouse;

  double cameraYAngle;
  double cameraZoom;
  double animationStartTime;
  Animation playerWalkingAnimation;
  Animation playerStandingAnimation;
  Animation* currentAnimation;

  std::set<int> keysPressed;

  Player player;

  Platform* ground;
  std::vector<Platform> blocks;

  bool isKeyPressed(int key);
  glm::vec3 calculatePlayerInputVelocity();
  void refreshPlayerInputVelocity();

  void renderScene(SceneNodeFunctor<void, glm::mat4>& renderer);
  /**
   * Shadow Map
   */

  GLuint FramebufferName;
  GLuint renderedTexture;
  GLuint depthTexture;
  unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
  ShaderProgram m_shader_depth;
  ShaderProgram m_shader_quad;

  GLuint VertexArrayID;
  GLuint quad_vertexbuffer;
  GLuint depthrenderbuffer;

  struct World {
    glm::vec3 F_g = glm::vec3(0, -12, 0); // Gravitational force
    glm::vec3 F_wind = glm::vec3(1, 0, 1); // Wind force
    float ufs = 0.1; // co-efficient of static friction
    float ufk = 0.05; // co-efficient of kinetic friction
  } world;

  static void initShaderProgram(ShaderProgram& program, const std::string& name);
};
