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
  near(1.0f),
  far(1000.0f),
  fov(30.0f),
  M(A2::createM()),
  view(A2::createView()),
  isModelScaling(false),
  isModelTranslating(false),
  isModelRotating(false),
  isMouseButtonLeftPressed(false),
  isMouseButtonRightPressed(false),
  isMouseButtonMiddlePressed(false),
  captureViewportPosition(false),
  selectedMode(RotateModel)
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

  modelGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
    LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1)},
    glm::vec3(1, 0, 0)
  });

  modelGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
    LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 1)},
    glm::vec3(0, 1, 0)
  });

  modelGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
    LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1)},
    glm::vec3(0, 0, 1)
  });

  /**
   * World Gnomon
   */

   worldGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
     LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1)},
     glm::vec3(11.0/255, 57.0/255, 84.0/255)
   });

   worldGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
     LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 1)},
     glm::vec3(1, 102.0/255, 99.0/255)
   });

   worldGnomon.push_back(std::tuple<LineSegment, glm::vec3>{
     LineSegment{glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1)},
     glm::vec3(1, 1, 1)
   });

  /**
   * Modes
   */

  modeNames[RotateView] = "Rotate View";
  modeNames[TranslateView] = "Translate View";
  modeNames[Perspective] = "Perspective";
  modeNames[RotateModel] = "Rotate Model";
  modeNames[TranslateModel] = "Translate Model";
  modeNames[ScaleModel] = "Scale Model";
  modeNames[Viewport] = "Viewport";
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
  glm::vec3 origin{0, 0, 10};
  glm::vec3 lookAt{0, 0, -1};
  glm::vec3 up{lookAt.x, lookAt.y + 1, lookAt.z};

  glm::vec3 z = lookAt;
  glm::vec3 x = glm::cross(z, up);
  glm::vec3 y = glm::cross(x, z);

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
  float aspect = float(m_framebufferWidth) / float(m_framebufferHeight);
  float theta = glm::radians(fov);
  float cot = std::cos(theta / 2) / std::sin(theta / 2);

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
  proj = createProj();

  viewportX = 0.05f * m_framebufferWidth;
  viewportY = 0.05f * m_framebufferHeight;
  viewportWidth = 0.9f * m_framebufferWidth;
  viewportHeight = 0.9f * m_framebufferHeight;

  initViewportX = viewportX;
  initViewportY = viewportY;
  initViewportWidth = viewportWidth;
  initViewportHeight = viewportHeight;

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

  for (const LineSegment& line : gridLines) {
    LineSegment worldLine {
      VTransformations * view * M * MTransformations * std::get<0>(line),
      VTransformations * view * M * MTransformations * std::get<1>(line)
    };

    LineSegment transformedLine {
      proj * std::get<0>(worldLine),
      proj * std::get<1>(worldLine)
    };

    for (const LineSegment& clippedLine : Clipper::clip(transformedLine)) {
      glm::vec4 start = homogenize(std::get<0>(clippedLine));
      glm::vec4 end = homogenize(std::get<1>(clippedLine));

      drawLineInViewport(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
    }
  }

  // Draw model gnomon

  for (const auto gnomonAxis : modelGnomon) {
    const LineSegment gnomonLineSegment = std::get<0>(gnomonAxis);
    const glm::vec3 color = std::get<1>(gnomonAxis);

    setLineColour(color);

    LineSegment worldLine {
      VTransformations * view * M * MGnomonTransformations * std::get<0>(gnomonLineSegment),
      VTransformations * view * M * MGnomonTransformations * std::get<1>(gnomonLineSegment)
    };

    LineSegment transformedLine {
      proj * std::get<0>(worldLine),
      proj * std::get<1>(worldLine)
    };

    for (const LineSegment& clippedLine : Clipper::clip(transformedLine)) {
      glm::vec4 start = homogenize(std::get<0>(clippedLine));
      glm::vec4 end = homogenize(std::get<1>(clippedLine));

      drawLineInViewport(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
    }
  }

  // Draw world gnomon
  for (const auto gnomonAxis : worldGnomon) {
    const LineSegment gnomonLineSegment = std::get<0>(gnomonAxis);
    const glm::vec3 color = std::get<1>(gnomonAxis);

    setLineColour(color);

    LineSegment worldLine {
      VGnomonTransformations * view * M * std::get<0>(gnomonLineSegment),
      VGnomonTransformations * view * M * std::get<1>(gnomonLineSegment)
    };

    LineSegment transformedLine {
      proj * std::get<0>(worldLine),
      proj * std::get<1>(worldLine)
    };

    for (const LineSegment& clippedLine : Clipper::clip(transformedLine)) {
      glm::vec4 start = homogenize(std::get<0>(clippedLine));
      glm::vec4 end = homogenize(std::get<1>(clippedLine));

      drawLineInViewport(glm::vec2(start.x, start.y), glm::vec2(end.x, end.y));
    }
  }

  // Draw Viewport
  setLineColour(glm::vec3(1, 1, 1));
  drawLineInViewport(glm::vec2(-1, -1), glm::vec2(glm::vec2(-1, 1)));
  drawLineInViewport(glm::vec2(-1, -1), glm::vec2(glm::vec2(1, -1)));
  drawLineInViewport(glm::vec2(1, 1), glm::vec2(glm::vec2(-1, 1)));
  drawLineInViewport(glm::vec2(1, 1), glm::vec2(glm::vec2(1, -1)));

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

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    // Create Button, and check if it was clicked:
    if( ImGui::Button( "Reset" ) ) {
      reset();
    }

    ImGui::SameLine();

    if( ImGui::Button( "Quit" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

    // Eventually you'll create multiple colour widgets with
    // radio buttons.  If you use PushID/PopID to give them all
    // unique IDs, then ImGui will be able to keep them separate.
    // This is unnecessary with a single colour selector and
    // radio button, but I'm leaving it in as an example.

    // Prefixing a widget name with "##" keeps it from being
    // displayed.

    for (int m = RotateView; m != LastMode; m++) {
      ImGui::PushID( m );
      if( ImGui::RadioButton( modeNames[m].c_str(), (int*) &selectedMode, m ) ) {

      }
      ImGui::PopID();
    }

    ImGui::Text("FOV: %.2f deg", fov);
    ImGui::Text("Near: %.2f", near);
    ImGui::Text("Far: %.2f", far);

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

  switch (selectedMode) {
    case RotateModel:
      rotateModel(xPos, yPos);
      break;
    case ScaleModel:
      scaleModel(xPos, yPos);
      break;
    case TranslateModel:
      translateModel(xPos, yPos);
      break;
    case RotateView:
      rotateView(xPos, yPos);
      break;
    case TranslateView:
      translateView(xPos, yPos);
      break;
    case Perspective:
      perspective(xPos, yPos);
      break;
    case Viewport:
      viewport(xPos, yPos);
      break;
    default:;
  }

  prevX = xPos;
  prevY = yPos;
  return true;
}

void A2::rotateModel(double xPos, double yPos) {
  double theta = glm::radians(xPos - prevX);

  if (isMouseButtonLeftPressed) {
    const glm::mat4 rotationX (
      1, 0, 0, 0,
      0, std::cos(theta), std::sin(theta), 0,
      0, -std::sin(theta), std::cos(theta), 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * rotationX;
    MGnomonTransformations = MGnomonTransformations * rotationX;
  }

  if (isMouseButtonMiddlePressed) {
    const glm::mat4 rotationY (
      std::cos(theta), 0, -std::sin(theta), 0,
      0, 1, 0, 0,
      std::sin(theta), 0, std::cos(theta), 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * rotationY;
    MGnomonTransformations = MGnomonTransformations * rotationY;
  }

  if (isMouseButtonRightPressed) {
    const glm::mat4 rotationZ (
      std::cos(theta), std::sin(theta), 0, 0,
      -std::sin(theta), std::cos(theta), 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * rotationZ;
    MGnomonTransformations = MGnomonTransformations * rotationZ;
  }
}

void A2::scaleModel(double xPos, double yPos) {
  const double factor = xPos > prevX ? 1.05 : 1/1.05;

  if (isMouseButtonLeftPressed) {
    const glm::mat4 scaleX (
      factor, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * scaleX;
  }

  if (isMouseButtonMiddlePressed) {
    const glm::mat4 scaleY (
      1, 0, 0, 0,
      0, factor, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * scaleY;
  }

  if (isMouseButtonRightPressed) {
    const glm::mat4 scaleZ (
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, factor, 0,
      0, 0, 0, 1
    );

    MTransformations = MTransformations * scaleZ;
  }
}

void A2::translateModel(double xPos, double yPos) {
  const double diff = (xPos - prevX) / 100;

  if (isMouseButtonLeftPressed) {
    const glm::mat4 translateX(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      diff, 0, 0, 1
    );

    MTransformations = MTransformations * translateX;
    MGnomonTransformations = MGnomonTransformations * translateX;
  }

  if (isMouseButtonMiddlePressed) {
    const glm::mat4 translateY(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, diff, 0, 1
    );

    MTransformations = MTransformations * translateY;
    MGnomonTransformations = MGnomonTransformations * translateY;
  }

  if (isMouseButtonRightPressed) {
    const glm::mat4 translateZ(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, diff, 1
    );

    MTransformations = MTransformations * translateZ;
    MGnomonTransformations = MGnomonTransformations * translateZ;
  }
}

void A2::rotateView(double xPos, double yPos) {
  double theta =-glm::radians(xPos - prevX);

  if (isMouseButtonLeftPressed) {
    const glm::mat4 rotationX (
      1, 0, 0, 0,
      0, std::cos(theta), std::sin(theta), 0,
      0, -std::sin(theta), std::cos(theta), 0,
      0, 0, 0, 1
    );

    VTransformations = rotationX * VTransformations;
    VGnomonTransformations = rotationX * VGnomonTransformations;
  }

  if (isMouseButtonMiddlePressed) {
    const glm::mat4 rotationY (
      std::cos(theta), 0, -std::sin(theta), 0,
      0, 1, 0, 0,
      std::sin(theta), 0, std::cos(theta), 0,
      0, 0, 0, 1
    );

    VTransformations = rotationY * VTransformations;
    VGnomonTransformations = rotationY * VGnomonTransformations;
  }

  if (isMouseButtonRightPressed) {
    const glm::mat4 rotationZ (
      std::cos(theta), std::sin(theta), 0, 0,
      -std::sin(theta), std::cos(theta), 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    );

    VTransformations = rotationZ * VTransformations;
    VGnomonTransformations = rotationZ * VGnomonTransformations;
  }
}

void A2::translateView(double xPos, double yPos) {
  const double diff = - (xPos - prevX) / 100;

  if (isMouseButtonLeftPressed) {
    const glm::mat4 translateX(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      diff, 0, 0, 1
    );

    VTransformations = translateX * VTransformations;
    VGnomonTransformations = translateX * VGnomonTransformations;
  }

  if (isMouseButtonMiddlePressed) {
    const glm::mat4 translateY(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, diff, 0, 1
    );

    VTransformations = translateY * VTransformations;
    VGnomonTransformations = translateY * VGnomonTransformations;
  }

  if (isMouseButtonRightPressed) {
    const glm::mat4 translateZ(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, diff, 1
    );

    VTransformations = translateZ * VTransformations;
    VGnomonTransformations = translateZ * VGnomonTransformations;
  }
}

void A2::perspective(double xPos, double yPos) {
  const float diff = (xPos - prevX) / 50;

  if (isMouseButtonLeftPressed) {
    fov = glm::clamp(fov + diff, 5.0f, 160.0f);
    proj = createProj();
  }

  if (isMouseButtonMiddlePressed) {
    near = std::min(near + diff, far);
    proj = createProj();
  }

  if (isMouseButtonRightPressed) {
    far = std::max(far + diff, near);
    proj = createProj();
  }
}

void A2::viewport(double x, double y) {
  float xPos = 2 * x;
  float yPos = m_framebufferHeight - 2 * y;
  if (isMouseButtonLeftPressed) {
    if (captureViewportPosition) {
      viewportX = glm::clamp(xPos, 0.0f, (float)m_framebufferWidth - 1);
      viewportY = glm::clamp(yPos, 0.0f, (float)m_framebufferHeight - 1);
      viewportHeight = 1;
      viewportWidth = 1;
      captureViewportPosition = false;
    } else {
      viewportWidth = glm::clamp(xPos, 0.0f, (float)m_framebufferWidth) - viewportX;
      viewportHeight = glm::clamp(yPos, 0.0f, (float)m_framebufferHeight) - viewportY;
    }
  }

  // std::cerr << "viewportX: " << viewportX << std::endl;
  // std::cerr << "viewportY: " << viewportY << std::endl;
  // std::cerr << "viewportWidth: " << viewportWidth << std::endl;
  // std::cerr << "viewportHeight: " << viewportHeight << std::endl;
}

void A2::reset() {
  selectedMode = RotateModel;
  near = 1.0f;
  far = 1000.0f;
  fov = 30.0f;

  proj = createProj();
  MTransformations = glm::mat4();
  MGnomonTransformations = glm::mat4();
  VTransformations = glm::mat4();
  VGnomonTransformations = glm::mat4();

  viewportX = initViewportX;
  viewportY = initViewportY;
  viewportWidth = initViewportWidth;
  viewportHeight = initViewportHeight;
}

// glm::mat4 getViewportTransform() {
//   float vEndX = 2 * std::max(viewportX + viewportWidth, viewportX) / m_framebufferWidth - 1;
//   float vEndY = 2 * std::max(viewportY + viewportHeight, viewportY) / m_framebufferHeight - 1;
//   float vx = 2 * std::min(viewportX + viewportWidth, viewportX) / m_framebufferWidth - 1;
//   float vy = 2 * std::min(viewportY + viewportHeight, viewportY) / m_framebufferHeight - 1;
//
//   float vcx = (vEndx + vx) / 2;
//   float vcy = (vEndY + vy) / 2;
//
//   float sx
//   float tx = vcx / m_framebufferWidth;
//   float ty = vcy / m_framebufferHeight;
//
//   return glm::mat4(
//     vWidth /
//   )
// }

void A2::drawLineInViewport(const glm::vec2 & v0, const glm::vec2 & v1) {
  drawLine(scalePointToViewport(v0), scalePointToViewport(v1));
}

glm::vec2 A2::scalePointToViewport(const glm::vec2 & v) {
  // std::cerr << "viewportX: " << viewportX << std::endl;
  // std::cerr << "viewportY: " << viewportY << std::endl;
  // std::cerr << "viewportWidth: " << viewportWidth << std::endl;
  // std::cerr << "viewportHeight: " << viewportHeight << std::endl;
  // std::cerr << "m_framebufferWidth: " << m_framebufferWidth << std::endl;
  // std::cerr << "m_framebufferHeight: " << m_framebufferHeight << std::endl;

  float vEndX = 2 * std::max(viewportX + viewportWidth, viewportX) / m_framebufferWidth - 1;
  float vEndY = 2 * std::max(viewportY + viewportHeight, viewportY) / m_framebufferHeight - 1;
  float vx = 2 * std::min(viewportX + viewportWidth, viewportX) / m_framebufferWidth - 1;
  float vy = 2 * std::min(viewportY + viewportHeight, viewportY) / m_framebufferHeight - 1;

  // std::cerr << "vx: " << vx << std::endl;
  // std::cerr << "vy: " << vy << std::endl;
  // std::cerr << "vEndX: " << vEndX << std::endl;
  // std::cerr << "vEndY: " << vEndY << std::endl;

  glm::vec2 result(
    (vEndX - vx) / (1 - (-1)) * (v.x - (-1)) + vx,
    ((vEndY - vy) / (1 - (-1)) * (v.y - (-1)) + vy)
  );

  return result;
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

        if (selectedMode == Viewport) {
          captureViewportPosition = true;
        }
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

  /**
   * @todo What should happen here?
   * Should the viewport also resize with the window?
   */
  proj = createProj();

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
        selectedMode = RotateModel;
        return true;
      case GLFW_KEY_T:
        selectedMode = TranslateModel;
        return true;
      case GLFW_KEY_S:
        selectedMode = ScaleModel;
        return true;
      case GLFW_KEY_O:
        selectedMode = RotateView;
        return true;
      case GLFW_KEY_N:
        selectedMode = TranslateView;
        return true;
      case GLFW_KEY_P:
        selectedMode = Perspective;
        return true;
      case GLFW_KEY_V:
        selectedMode = Viewport;
        return true;
      case GLFW_KEY_A:
        reset();
        return true;
      case GLFW_KEY_Q:
        glfwSetWindowShouldClose(m_window, GL_TRUE);
        return true;
    }
  }

  return false;
}

/**
 * The Clipper
 * Citation:
 *
 * James F. Blinn and Martin E. Newell. 1978. Clipping using homogeneous coordinates.
 * In Proceedings of the 5th annual conference on Computer graphics and interactive techniques
 * (SIGGRAPH '78). ACM, New York, NY, USA, 245-251.
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
