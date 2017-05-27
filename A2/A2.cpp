#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <cmath>
#include <functional>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData():
  numVertices(0),
  index(0)
{
  positions.resize(kMaxVertices);
  colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2() :
  m_currentLineColour(vec3(0.0f)),
  M(A2::createM()),
  view(A2::createView()),
  proj(createProj()),
  isModelScaling(false),
  isModelTranslating(false),
  isModelRotating(false),
  isMouseButtonLeftPressed(false),
  isMouseButtonRightPressed(false),
  isMouseButtonMiddlePressed(false)
{
  const float min = -1;
  const float max = 1;

  // Bottom Square
  gridLines.push_back(LineSegment{glm::vec4(min, min, min, 1), glm::vec4(max, min, min, 1)});
  gridLines.push_back(LineSegment{glm::vec4(min, min, min, 1), glm::vec4(min, min, max, 1)});
  gridLines.push_back(LineSegment{glm::vec4(max, min, max, 1), glm::vec4(max, min, min, 1)});
  gridLines.push_back(LineSegment{glm::vec4(max, min, max, 1), glm::vec4(min, min, max, 1)});

  // Top Square
  gridLines.push_back(LineSegment{glm::vec4(min, max, min, 1), glm::vec4(max, max, min, 1)});
  gridLines.push_back(LineSegment{glm::vec4(min, max, min, 1), glm::vec4(min, max, max, 1)});
  gridLines.push_back(LineSegment{glm::vec4(max, max, max, 1), glm::vec4(max, max, min, 1)});
  gridLines.push_back(LineSegment{glm::vec4(max, max, max, 1), glm::vec4(min, max, max, 1)});

  // Pillars
  for (const float x : std::vector<float>{min, max}) {
    for (const float z : std::vector<float>{min, max}) {
      gridLines.push_back(LineSegment{glm::vec4(x, min, z, 1), glm::vec4(x, max, z, 1)});
    }
  }

  /**
   * Model Gnomon
   */

  modelGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1)});
  modelGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 1)});
  modelGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1)});

  /**
   * World Gnomon
   */
  worldGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1)});
  worldGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 1)});
  worldGnomon.push_back(LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1)});
}

glm::mat4 A2::createM() {
  return glm::mat4(
    1, 0, 0, 0, // First column
    0, 1, 0, 0, // Second column
    0, 0, 1, 0, // Third column
    0, 0, 0, 1  // Fourth column
  );
}

glm::mat4 A2::createView() {
  glm::vec3 origin{0, 0, 5};
  glm::vec3 lookAt{0, 0, -1};
  glm::vec3 up{lookAt.x, lookAt.y + 1, lookAt.z};

  glm::vec3 z = lookAt;
  glm::vec3 x = glm::cross(up, z);
  glm::vec3 y = glm::cross(z, x);

  glm::mat4 MT {
    glm::vec4(x, 0),
    glm::vec4(y, 0),
    glm::vec4(z, 0),
    glm::vec4(origin, 1)
  };

  // View will take coordinates in the cartesian frame to coordinates in the camera's frame
  // http://www.uio.no/studier/emner/matnat/ifi/INF3320/h03/undervisningsmateriale/lecture3.pdf

  return glm::inverse(MT);
}

glm::mat4 A2::createProj() {
  float aspect = 1.0;
  float theta = glm::radians(30.0f);
  float far = 1000.0f;
  float near = 1.0f;
  float cot = std::cos(theta / 2) / std::sin(theta / 2);

  // return glm::perspective(
  //   glm::radians( 40.0f ),
  //   16.0f / 9,
  //   1.0f,
  //   1000.0f
  // );

  return glm::mat4(
    cot / aspect,   0,                               0,  0,
    0,            cot,                               0,  0,
    0,              0,    -(far + near) / (far - near),  1,
    0,              0,  2 * far * near  / (far - near),  0
  );
}


//----------------------------------------------------------------------------------------
// Destructor
A2::~A2() {

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init() {
  // Set the background colour.
  glClearColor(0.3, 0.5, 0.7, 1.0);

  createShaderProgram();

  glGenVertexArrays(1, &m_vao);

  enableVertexAttribIndices();

  generateVertexBuffers();

  mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram() {
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
  m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
  m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices() {
  glBindVertexArray(m_vao);

  // Enable the attribute index location for "position" when rendering.
  GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray(positionAttribLocation);

  // Enable the attribute index location for "colour" when rendering.
  GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
  glEnableVertexAttribArray(colourAttribLocation);

  // Restore defaults
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers() {
  // Generate a vertex buffer to store line vertex positions
  {
    glGenBuffers(1, &m_vbo_positions);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

    // Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
        GL_DYNAMIC_DRAW);


    // Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }

  // Generate a vertex buffer to store line colors
  {
    glGenBuffers(1, &m_vbo_colours);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

    // Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
        GL_DYNAMIC_DRAW);


    // Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation() {
  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao);

  // Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
  // "position" vertex attribute index for any bound shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
  GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
  glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
  // "colour" vertex attribute index for any bound shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
  GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
  glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData() {
  m_vertexData.numVertices = 0;
  m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
  const glm::vec3 & colour
) {
  m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
  const glm::vec2 & v0,   // Line Start (NDC coordinate)
  const glm::vec2 & v1    // Line End (NDC coordinate)
) {

  m_vertexData.positions[m_vertexData.index] = v0;
  m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
  ++m_vertexData.index;
  m_vertexData.positions[m_vertexData.index] = v1;
  m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
  ++m_vertexData.index;

  m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic() {
  // Place per frame, application logic here ...

  // Call at the beginning of frame, before drawing lines:
  initLineData();

  // Draw outer square:
  setLineColour(vec3(1.0f, 0.7f, 0.8f));

  const glm::mat4 T = proj * view *  M;

  for (const LineSegment& line : gridLines) {
    LineSegment worldLine {
      view * M * MTransformations * std::get<0>(line),
      view * M * MTransformations * std::get<1>(line)
    };

    LineSegment transformedLine {
      proj * std::get<0>(worldLine),
      proj * std::get<1>(worldLine)
    };

    for (const LineSegment& clippedLine : Clipper::clip(transformedLine)) {
      glm::vec4 start = homogenize(std::get<0>(clippedLine));
      glm::vec4 end = homogenize(std::get<1>(clippedLine));

      drawLine(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
    }
  }

  // Draw model gnomon
  for (unsigned int i = 0; i < modelGnomon.size(); i += 1) {
    glm::vec3 color;
    color[i] = 1;
    setLineColour(color);

    LineSegment& line = modelGnomon.at(i);

    LineSegment worldLine {
      view * M * MGnomonTransformations * std::get<0>(line),
      view * M * MGnomonTransformations * std::get<1>(line)
    };

    LineSegment transformedLine {
      proj * std::get<0>(worldLine),
      proj * std::get<1>(worldLine)
    };

    for (const LineSegment& clippedLine : Clipper::clip(transformedLine)) {
      glm::vec4 start = homogenize(std::get<0>(clippedLine));
      glm::vec4 end = homogenize(std::get<1>(clippedLine));

      drawLine(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
    }
  }
}

glm::vec4 A2::homogenize(const glm::vec4& v) {
  if (std::fabs(v.w) < 0.00001) {
    return v;
  }

  return glm::vec4(
    v.x * 1.0f / v.w,
    v.y * 1.0f  / v.w,
    v.z * 1.0f / v.w,
    1
  );
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic() {
  static bool firstRun(true);
  if (firstRun) {
    ImGui::SetNextWindowPos(ImVec2(50, 50));
    firstRun = false;
  }

  static bool showDebugWindow(true);
  ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
  float opacity(0.5f);

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
      windowFlags);


    // Add more gui elements here here ...


    // Create Button, and check if it was clicked:
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

  //-- Copy vertex position data into VBO, m_vbo_positions:
  {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
        m_vertexData.positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }

  //-- Copy vertex colour data into VBO, m_vbo_colours:
  {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
        m_vertexData.colours.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw() {
  uploadVertexDataToVbos();

  glBindVertexArray(m_vao);

  m_shader.enable();
    glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
  m_shader.disable();

  // Restore defaults
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup() {

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
  int entered
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
  double xPos,
  double yPos
) {
  const double diffX = xPos - prevX;
  const double diffY = yPos - prevY;

  if (isModelRotating) {
    double theta = glm::radians(diffX);

    if (isMouseButtonLeftPressed) {
      const glm::mat4 rotationX (
        1, 0, 0, 0,
        0, std::cos(theta), std::sin(theta), 0,
        0, -std::sin(theta), std::cos(theta), 0,
        0, 0, 0, 1
      );

      MTransformations = rotationX * MTransformations;
      MGnomonTransformations = rotationX * MGnomonTransformations;
    }

    if (isMouseButtonRightPressed) {
      const glm::mat4 rotationY (
        std::cos(theta), 0, -std::sin(theta), 0,
        0, 1, 0, 0,
        std::sin(theta), 0, std::cos(theta), 0,
        0, 0, 0, 1
      );

      MTransformations = rotationY * MTransformations;
      MGnomonTransformations = rotationY * MGnomonTransformations;
    }

    if (isMouseButtonMiddlePressed) {
      const glm::mat4 rotationZ (
        std::cos(theta), std::sin(theta), 0, 0,
        -std::sin(theta), std::cos(theta), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      );

      MTransformations = rotationZ * MTransformations;
      MGnomonTransformations = rotationZ * MGnomonTransformations;
    }
  }

  if (isModelScaling) {
    const double factor = diffX > 0 ? 1.05 : 1/1.05;

    if (isMouseButtonLeftPressed) {
      const glm::mat4 scaleX (
        factor, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      );

      MTransformations = scaleX * MTransformations;
    }

    if (isMouseButtonRightPressed) {
      const glm::mat4 scaleY (
        1, 0, 0, 0,
        0, factor, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      );

      MTransformations = scaleY * MTransformations;
    }

    if (isMouseButtonMiddlePressed) {
      const glm::mat4 scaleZ (
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, factor, 0,
        0, 0, 0, 1
      );

      MTransformations = scaleZ * MTransformations;
    }
  }

  if (isModelTranslating) {
    const double diff = -diffX / 100;

    if (isMouseButtonLeftPressed) {
      const glm::mat4 translateX(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        diff, 0, 0, 1
      );

      MTransformations = translateX * MTransformations;
    }

    if (isMouseButtonRightPressed) {
      const glm::mat4 translateY(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, diff, 0, 1
      );

      MTransformations = translateY * MTransformations;
    }

    if (isMouseButtonMiddlePressed) {
      const glm::mat4 translateZ(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, diff, 1
      );

      MTransformations = translateZ * MTransformations;
    }
  }

  prevX = xPos;
  prevY = yPos;
  return true;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
  int button,
  int action,
  int mods
) {
  if (action == GLFW_PRESS) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        isMouseButtonLeftPressed = true;
        return true;
      case GLFW_MOUSE_BUTTON_RIGHT:
        isMouseButtonRightPressed = true;
        return true;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        isMouseButtonMiddlePressed = true;
        return true;
    }
  }

  if (action == GLFW_RELEASE) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        isMouseButtonLeftPressed = false;
        return true;
      case GLFW_MOUSE_BUTTON_RIGHT:
        isMouseButtonRightPressed = false;
        return true;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        isMouseButtonMiddlePressed = false;
        return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
  double xOffSet,
  double yOffSet
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
  int width,
  int height
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
  int key,
  int action,
  int mods
) {

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_R:
        isModelRotating = true;
        return true;
      case GLFW_KEY_T:
        isModelTranslating = true;
        return true;
      case GLFW_KEY_S:
        isModelScaling = true;
        return true;
    }
  }

  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_R:
        isModelRotating = false;
        return true;
      case GLFW_KEY_T:
        isModelTranslating = false;
        return true;
      case GLFW_KEY_S:
        isModelScaling = false;
        return true;
    }
  }

  return false;
}

/**
 * Clipper
 */

float Clipper::BL(const glm::vec4 point) {
  return point.w + point.x;
}

float Clipper::BR(const glm::vec4 point) {
  return point.w - point.x;
}

float Clipper::BB(const glm::vec4 point) {
  return point.w + point.y;
}

float Clipper::BT(const glm::vec4 point) {
  return point.w - point.y;
}

float Clipper::BN(const glm::vec4 point) {
  return point.w - point.z;
}

float Clipper::BF(const glm::vec4 point) {
  return point.w + point.z;
}

LineSegment Clipper::clipPos(const LineSegment &line) {
  glm::vec4 P1 = std::get<0>(line);
  glm::vec4 P2 = std::get<1>(line);

  std::vector<std::function<float(const glm::vec4)> > clippingBoundaries {
    BL, BR, BB, BT, BN, BF
  };

  int i = 0;

  for (const auto clippingBoundary : clippingBoundaries) {
    i += 1;
    float wecP1 = clippingBoundary(P1);
    float wecP2 = clippingBoundary(P2);

    if (wecP1 < 0 && wecP2 < 0) {
      // std::cerr << "i: " << i << std::endl;
      // if (i == 1) {
      //   std::cerr << "BL(P1: " << P1 << "): " << BL(P1) << std::endl;
      //   std::cerr << "BL(P2: " << P2 << "): " << BL(P2) << std::endl;
      // }
      // std::cerr << "wecP1: " << wecP1 << std::endl;
      // std::cerr << "wecP2: " << wecP2 << std::endl;
      throw LineRejected();
    }

    if (wecP1 >= 0 && wecP2 >= 0) {
      continue;
    }

    float a = wecP1 / (wecP1 - wecP2);

    if (wecP1 < 0) {
      P1 = P1 + a * (P2 - P1);
    } else {
      P2 = P1 + a * (P2 - P1);
    }
  }

  return LineSegment{P1, P2};
}

std::vector<LineSegment> Clipper::clip(const LineSegment &line) {
  glm::vec4 P1 = std::get<0>(line);
  glm::vec4 P2 = std::get<1>(line);

  if (P1.w >= 0 && P2.w >= 0) {
    try {
      return std::vector<LineSegment>{clipPos(line)};
    } catch (LineRejected) {
      return std::vector<LineSegment>{};
    }
  }

  LineSegment mirrored {
    P1 * -1.0f,
    P2 * -1.0f
  };

  if (P1.w < 0 && P2.w < 0) {
    try {
      return std::vector<LineSegment>{clipPos(mirrored)};
    } catch (LineRejected) {
      return std::vector<LineSegment>{};
    }
  }

  std::vector<LineSegment> clippedLines;

  try {
    clippedLines.push_back(clipPos(line));
  } catch (LineRejected) {}

  try {
    clippedLines.push_back(clipPos(mirrored));
  } catch (LineRejected) {}

  return clippedLines;
}
