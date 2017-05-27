#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <tuple>
#include <vector>
#include <glm/glm.hpp>

typedef std::tuple<glm::vec4, glm::vec4> LineSegment;

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;

// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
  VertexData();

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colours;
  GLuint index;
  GLsizei numVertices;
};


class A2 : public CS488Window {
public:
  A2();
  virtual ~A2();

protected:
  virtual void init() override;
  virtual void appLogic() override;
  virtual void guiLogic() override;
  virtual void draw() override;
  virtual void cleanup() override;

  virtual bool cursorEnterWindowEvent(int entered) override;
  virtual bool mouseMoveEvent(double xPos, double yPos) override;
  virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
  virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
  virtual bool windowResizeEvent(int width, int height) override;
  virtual bool keyInputEvent(int key, int action, int mods) override;

  void createShaderProgram();
  void enableVertexAttribIndices();
  void generateVertexBuffers();
  void mapVboDataToVertexAttributeLocation();
  void uploadVertexDataToVbos();

  void initLineData();

  void setLineColour(const glm::vec3 & colour);

  void drawLine (
    const glm::vec2 & v0,
    const glm::vec2 & v1
  );

  ShaderProgram m_shader;

  GLuint m_vao;            // Vertex Array Object
  GLuint m_vbo_positions;  // Vertex Buffer Object
  GLuint m_vbo_colours;    // Vertex Buffer Object

  VertexData m_vertexData;

  glm::vec3 m_currentLineColour;

  std::vector<LineSegment> gridLines;
  std::vector<LineSegment> modelGnomon;
  std::vector<LineSegment> worldGnomon;
  glm::mat4 M;
  glm::mat4 view;
  glm::mat4 proj;

  /**
   * Model transformations
   */
  bool isModelTranslating;
  bool isModelRotating;
  bool isModelScaling;

  glm::mat4 MTransformations;
  glm::mat4 MGnomonTransformations;

  /**
   * Mouse input state
   */
  double prevX;
  double prevY;
  bool isMouseButtonLeftPressed;
  bool isMouseButtonRightPressed;
  bool isMouseButtonMiddlePressed;

  static glm::mat4 createM();
  static glm::mat4 createView();
  glm::mat4 createProj();
  static glm::vec4 homogenize(const glm::vec4& v);
};

#include <exception>

class Clipper {
private:
  static float BL(const glm::vec4 point);
  static float BR(const glm::vec4 point);
  static float BB(const glm::vec4 point);
  static float BT(const glm::vec4 point);
  static float BN(const glm::vec4 point);
  static float BF(const glm::vec4 point);
  static LineSegment clipPos(const LineSegment &line);

  class LineRejected : public std::exception {
    virtual const char* what() const throw() {
      return "Line cannot be viewed.";
    }
  };
public:
  static std::vector<LineSegment> clip(const LineSegment &line);
};
