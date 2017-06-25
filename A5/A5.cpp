#include "A5.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "Visitor.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"
#include "Keyframe.hpp"
#include "Animation.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <stack>
#include <cassert>
#include <chrono>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A5::A5()
  : m_positionAttribLocation(0),
    m_normalAttribLocation(0),
    m_vao_meshData(0),
    m_vbo_vertexPositions(0),
    m_vbo_vertexNormals(0),
    m_vao_arcCircle(0),
    m_vbo_arcCircle(0),
    t_start(std::chrono::high_resolution_clock::now()),
    playerWalkingAnimation(0.1)
{
  double bodyXRotation = 25;
  float yDispScale = 0.5;
  Keyframe one;
  one.rotations["body"] = bodyXRotation;
  one.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  one.rotations["head-joint"] = 0;
  one.rotations["neck-joint"] = 0;

  one.rotations["left-upper-arm-joint"] = -20;
  one.rotations["left-lower-arm-joint"] = -100;
  one.rotations["left-hand-joint"] = 0;

  one.rotations["right-upper-arm-joint"] = 60;
  one.rotations["right-lower-arm-joint"] = -45;
  one.rotations["right-hand-joint"] = 0;

  one.rotations["left-upper-leg-joint"] = 0;
  one.rotations["left-lower-leg-joint"] = 90;
  one.rotations["left-foot-joint"] = 0;

  one.rotations["right-upper-leg-joint"] = -45;
  one.rotations["right-lower-leg-joint"] = 0;
  one.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(one);

  Keyframe two;
  two.rotations["body"] = bodyXRotation;
  two.positions["body"] = glm::vec3(0, -0.5, 0) * yDispScale;
  two.rotations["head-joint"] = 0;
  two.rotations["neck-joint"] = 0;

  two.rotations["left-upper-arm-joint"] = -20;
  two.rotations["left-lower-arm-joint"] = -90;
  two.rotations["left-hand-joint"] = 0;

  two.rotations["right-upper-arm-joint"] = 40;
  two.rotations["right-lower-arm-joint"] = -15;
  two.rotations["right-hand-joint"] = 0;

  two.rotations["left-upper-leg-joint"] = -10;
  two.rotations["left-lower-leg-joint"] = 90;
  two.rotations["left-foot-joint"] = 20;

  two.rotations["right-upper-leg-joint"] = -45;
  two.rotations["right-lower-leg-joint"] = 45;
  two.rotations["right-foot-joint"] = -15;

  playerWalkingAnimation.push(two);

  Keyframe three;
  three.rotations["body"] = bodyXRotation;
  three.positions["body"] = glm::vec3(0, -0.25, 0) * yDispScale;
  three.rotations["head-joint"] = 0;
  three.rotations["neck-joint"] = 0;

  three.rotations["left-upper-arm-joint"] = 0;
  three.rotations["left-lower-arm-joint"] = -30;
  three.rotations["left-hand-joint"] = 0;

  three.rotations["right-upper-arm-joint"] = 0;
  three.rotations["right-lower-arm-joint"] = -5;
  three.rotations["right-hand-joint"] = 0;

  three.rotations["left-upper-leg-joint"] = -30;
  three.rotations["left-lower-leg-joint"] = 60;
  three.rotations["left-foot-joint"] = 20;

  three.rotations["right-upper-leg-joint"] = -20;
  three.rotations["right-lower-leg-joint"] = 30;
  three.rotations["right-foot-joint"] = -15;

  playerWalkingAnimation.push(three);

  Keyframe four;
  four.rotations["body"] = bodyXRotation;
  four.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  four.rotations["head-joint"] = 0;
  four.rotations["neck-joint"] = 0;

  four.rotations["left-upper-arm-joint"] = 15;
  four.rotations["left-lower-arm-joint"] = -20;
  four.rotations["left-hand-joint"] = 0;

  four.rotations["right-upper-arm-joint"] = 0;
  four.rotations["right-lower-arm-joint"] = -80;
  four.rotations["right-hand-joint"] = 0;

  four.rotations["left-upper-leg-joint"] = -90;
  four.rotations["left-lower-leg-joint"] = 90;
  four.rotations["left-foot-joint"] = 20;

  four.rotations["right-upper-leg-joint"] = -10;
  four.rotations["right-lower-leg-joint"] = 10;
  four.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(four);


  Keyframe five;
  five.rotations["body"] = bodyXRotation;
  five.positions["body"] = glm::vec3(0, 0.4, 0) * yDispScale;
  five.rotations["head-joint"] = 0;
  five.rotations["neck-joint"] = 0;

  five.rotations["left-upper-arm-joint"] = 30;
  five.rotations["left-lower-arm-joint"] = -30;
  five.rotations["left-hand-joint"] = 0;

  five.rotations["right-upper-arm-joint"] = 0;
  five.rotations["right-lower-arm-joint"] = -90;
  five.rotations["right-hand-joint"] = 0;

  five.rotations["left-upper-leg-joint"] = -90;
  five.rotations["left-lower-leg-joint"] = 70;
  five.rotations["left-foot-joint"] = 10;

  five.rotations["right-upper-leg-joint"] = 15;
  five.rotations["right-lower-leg-joint"] = 0;
  five.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(five);

  Keyframe six;
  six.rotations["body"] = bodyXRotation;
  six.positions["body"] = glm::vec3(0, 0.25, 0) * yDispScale;
  six.rotations["head-joint"] = 0;
  six.rotations["neck-joint"] = 0;

  six.rotations["left-upper-arm-joint"] = 45;
  six.rotations["left-lower-arm-joint"] = -30;
  six.rotations["left-hand-joint"] = 0;

  six.rotations["right-upper-arm-joint"] = -30;
  six.rotations["right-lower-arm-joint"] = -95;
  six.rotations["right-hand-joint"] = 0;

  six.rotations["left-upper-leg-joint"] = -90;
  six.rotations["left-lower-leg-joint"] = 60;
  six.rotations["left-foot-joint"] = -10;

  six.rotations["right-upper-leg-joint"] = 0;
  six.rotations["right-lower-leg-joint"] = 60;
  six.rotations["right-foot-joint"] = 15;

  playerWalkingAnimation.push(six);

  Keyframe seven;
  seven.rotations["body"] = bodyXRotation;
  seven.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  seven.rotations["head-joint"] = 0;
  seven.rotations["neck-joint"] = 0;

  seven.rotations["left-upper-arm-joint"] = 60;
  seven.rotations["left-lower-arm-joint"] = -45;
  seven.rotations["left-hand-joint"] = 0;

  seven.rotations["right-upper-arm-joint"] = -20;
  seven.rotations["right-lower-arm-joint"] = -100;
  seven.rotations["right-hand-joint"] = 0;

  seven.rotations["left-upper-leg-joint"] = -45;
  seven.rotations["left-lower-leg-joint"] = 0;
  seven.rotations["left-foot-joint"] = 0;

  seven.rotations["right-upper-leg-joint"] = 0;
  seven.rotations["right-lower-leg-joint"] = 90;
  seven.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(seven);

  Keyframe eight;
  eight.rotations["body"] = bodyXRotation;
  eight.positions["body"] = glm::vec3(0, -0.5, 0) * yDispScale;
  eight.rotations["head-joint"] = 0;
  eight.rotations["neck-joint"] = 0;

  eight.rotations["left-upper-arm-joint"] = 40;
  eight.rotations["left-lower-arm-joint"] = -15;
  eight.rotations["left-hand-joint"] = 0;

  eight.rotations["right-upper-arm-joint"] = -20;
  eight.rotations["right-lower-arm-joint"] = -90;
  eight.rotations["right-hand-joint"] = 0;

  eight.rotations["left-upper-leg-joint"] = -45;
  eight.rotations["left-lower-leg-joint"] = 45;
  eight.rotations["left-foot-joint"] = -15;

  eight.rotations["right-upper-leg-joint"] = -10;
  eight.rotations["right-lower-leg-joint"] = 90;
  eight.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(eight);

  Keyframe nine;
  nine.rotations["body"] = bodyXRotation;
  nine.positions["body"] = glm::vec3(0, -0.25, 0) * yDispScale;
  nine.rotations["head-joint"] = 0;
  nine.rotations["neck-joint"] = 0;

  nine.rotations["left-upper-arm-joint"] = 0;
  nine.rotations["left-lower-arm-joint"] = -5;
  nine.rotations["left-hand-joint"] = 0;

  nine.rotations["right-upper-arm-joint"] = 0;
  nine.rotations["right-lower-arm-joint"] = -30;
  nine.rotations["right-hand-joint"] = 0;

  nine.rotations["left-upper-leg-joint"] = -20;
  nine.rotations["left-lower-leg-joint"] = 30;
  nine.rotations["left-foot-joint"] = -15;

  nine.rotations["right-upper-leg-joint"] = -30;
  nine.rotations["right-lower-leg-joint"] = 60;
  nine.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(nine);

  Keyframe ten;
  ten.rotations["body"] = bodyXRotation;
  ten.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  ten.rotations["head-joint"] = 0;
  ten.rotations["neck-joint"] = 0;

  ten.rotations["right-upper-arm-joint"] = 15;
  ten.rotations["right-lower-arm-joint"] = -20;
  ten.rotations["right-hand-joint"] = 0;

  ten.rotations["left-upper-arm-joint"] = 0;
  ten.rotations["left-lower-arm-joint"] = -80;
  ten.rotations["left-hand-joint"] = 0;

  ten.rotations["right-upper-leg-joint"] = -90;
  ten.rotations["right-lower-leg-joint"] = 90;
  ten.rotations["right-foot-joint"] = 20;

  ten.rotations["left-upper-leg-joint"] = -10;
  ten.rotations["left-lower-leg-joint"] = 10;
  ten.rotations["left-foot-joint"] = 0;

  playerWalkingAnimation.push(ten);

  Keyframe eleven;
  eleven.rotations["body"] = bodyXRotation;
  eleven.positions["body"] = glm::vec3(0, 0.4, 0) * yDispScale;
  eleven.rotations["head-joint"] = 0;
  eleven.rotations["neck-joint"] = 0;

  eleven.rotations["left-upper-arm-joint"] = 0;
  eleven.rotations["left-lower-arm-joint"] = -90;
  eleven.rotations["left-hand-joint"] = 0;

  eleven.rotations["right-upper-arm-joint"] = 30;
  eleven.rotations["right-lower-arm-joint"] = -30;
  eleven.rotations["right-hand-joint"] = 0;

  eleven.rotations["left-upper-leg-joint"] = 15;
  eleven.rotations["left-lower-leg-joint"] = 0;
  eleven.rotations["left-foot-joint"] = 20;

  eleven.rotations["right-upper-leg-joint"] = -90;
  eleven.rotations["right-lower-leg-joint"] = 70;
  eleven.rotations["right-foot-joint"] = 10;

  playerWalkingAnimation.push(eleven);

  Keyframe twelve;
  twelve.rotations["body"] = bodyXRotation;
  twelve.positions["body"] = glm::vec3(0, 0.25, 0) * yDispScale;
  twelve.rotations["head-joint"] = 0;
  twelve.rotations["neck-joint"] = 0;

  twelve.rotations["left-upper-arm-joint"] = -30;
  twelve.rotations["left-lower-arm-joint"] = -95;
  twelve.rotations["left-hand-joint"] = 0;

  twelve.rotations["right-upper-arm-joint"] = 45;
  twelve.rotations["right-lower-arm-joint"] = -30;
  twelve.rotations["right-hand-joint"] = 0;

  twelve.rotations["left-upper-leg-joint"] = 0;
  twelve.rotations["left-lower-leg-joint"] = 60;
  twelve.rotations["left-foot-joint"] = 15;

  twelve.rotations["right-upper-leg-joint"] = -90;
  twelve.rotations["right-lower-leg-joint"] = 60;
  twelve.rotations["right-foot-joint"] = -10;

  playerWalkingAnimation.push(twelve);
}

double A5::getTime() {
  auto t_now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<double> >(t_now - t_start).count();
}

//----------------------------------------------------------------------------------------
// Destructor
A5::~A5() {

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A5::init()
{
  // Set the background colour.
  glClearColor(0.35, 0.35, 0.35, 1.0);

  createShaderProgram();

  glGenVertexArrays(1, &m_vao_arcCircle);
  glGenVertexArrays(1, &m_vao_meshData);

  enableVertexShaderInputSlots();

  puppet = readLuaSceneFile(getAssetFilePath("puppet.lua"));
  level1 = readLuaSceneFile(getAssetFilePath("level1.lua"));

  // Load and decode all .obj files at once here.  You may add additional .obj files to
  // this list in order to support rendering additional mesh types.  All vertex
  // positions, and normals will be extracted and stored within the MeshConsolidator
  // class.
  unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
      getAssetFilePath("cube.obj"),
        getAssetFilePath("sphere.obj"),
        getAssetFilePath("suzanne.obj")
        });


  // Acquire the BatchInfoMap from the MeshConsolidator.
  meshConsolidator->getBatchInfoMap(m_batchInfoMap);

  // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
  uploadVertexDataToVbos(*meshConsolidator);

  mapVboDataToVertexShaderInputLocations();

  initPerspectiveMatrix();

  initViewMatrix();

  initLightSources();


  // Exiting the current scope calls delete automatically on meshConsolidator freeing
  // all vertex data resources.  This is fine since we already copied this data to
  // VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
  // this point.
}

std::shared_ptr<SceneNode> A5::readLuaSceneFile(const std::string& filename) {
  SceneNode* root = import_lua(filename);
  assert(root != NULL);
  return std::shared_ptr<SceneNode>(root);
}

//----------------------------------------------------------------------------------------
void A5::createShaderProgram()
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
  m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
  m_shader.link();

  m_shader_arcCircle.generateProgramObject();
  m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
  m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
  m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A5::enableVertexShaderInputSlots()
{
  //-- Enable input slots for m_vao_meshData:
  {
    glBindVertexArray(m_vao_meshData);

    // Enable the vertex shader attribute location for "position" when rendering.
    m_positionAttribLocation = m_shader.getAttribLocation("position");
    glEnableVertexAttribArray(m_positionAttribLocation);

    // Enable the vertex shader attribute location for "normal" when rendering.
    m_normalAttribLocation = m_shader.getAttribLocation("normal");
    glEnableVertexAttribArray(m_normalAttribLocation);

    CHECK_GL_ERRORS;
  }


  //-- Enable input slots for m_vao_arcCircle:
  {
    glBindVertexArray(m_vao_arcCircle);

    // Enable the vertex shader attribute location for "position" when rendering.
    m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
    glEnableVertexAttribArray(m_arc_positionAttribLocation);

    CHECK_GL_ERRORS;
  }

  // Restore defaults
  glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A5::uploadVertexDataToVbos (
  const MeshConsolidator & meshConsolidator
) {
  // Generate VBO to store all vertex position data
  {
    glGenBuffers(1, &m_vbo_vertexPositions);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
                 meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }

  // Generate VBO to store all vertex normal data
  {
    glGenBuffers(1, &m_vbo_vertexNormals);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
                 meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }

  // Generate VBO to store the trackball circle.
  {
    glGenBuffers( 1, &m_vbo_arcCircle );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

    float *pts = new float[ 2 * CIRCLE_PTS ];
    for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
      float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
      pts[2*idx] = cos( ang );
      pts[2*idx+1] = sin( ang );
    }

    glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
void A5::mapVboDataToVertexShaderInputLocations()
{
  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao_meshData);

  // Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
  // "position" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
  glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
  // "normal" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
  glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;

  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao_arcCircle);

  // Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
  // "position" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
  glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A5::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A5::initViewMatrix() {
  m_view = glm::lookAt(
    vec3(0.0f, 0.0f, 10.0f),
    vec3(0.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, 0.0f)
  );
}

//----------------------------------------------------------------------------------------
void A5::initLightSources() {
  // World-space position
  m_light.position = vec3(-2.0f, 5.0f, 0.5f);
  m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A5::uploadCommonSceneUniforms() {
  m_shader.enable();
  {
    //-- Set Perpsective matrix uniform for the scene:
    GLint location = m_shader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
    CHECK_GL_ERRORS;


    //-- Set LightSource uniform for the scene:
    {
      location = m_shader.getUniformLocation("light.position");
      glUniform3fv(location, 1, value_ptr(m_light.position));
      location = m_shader.getUniformLocation("light.rgbIntensity");
      glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
      CHECK_GL_ERRORS;
    }

    //-- Set background light ambient intensity
    {
      location = m_shader.getUniformLocation("ambientIntensity");
      vec3 ambientIntensity(0.05f);
      glUniform3fv(location, 1, value_ptr(ambientIntensity));
      CHECK_GL_ERRORS;
    }
  }
  m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A5::appLogic()
{
  // Place per frame, application logic here ...

  if (mouse.isLeftButtonPressed) {
    float disp = mouse.x - mouse.prevX;

    m_view = m_view * glm::rotate(glm::mat4(), disp/100, glm::vec3(0, 1, 0));
  }

  uploadCommonSceneUniforms();
  mouse.prevX = mouse.x;
  mouse.prevY = mouse.y;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A5::guiLogic()
{
  if( !show_gui ) {
    return;
  }

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

  ImGui::Text("Cursor: (%.1f, %.1f)", mouse.x, mouse.y);
  ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
  const ShaderProgram & shader,
  const GeometryNode & node,
  const glm::mat4 & viewMatrix
) {

  shader.enable();
  {
    //-- Set ModelView matrix:
    GLint location = shader.getUniformLocation("ModelView");
    mat4 modelView = viewMatrix;
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
    CHECK_GL_ERRORS;

    //-- Set NormMatrix:
    location = shader.getUniformLocation("NormalMatrix");
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
    glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
    CHECK_GL_ERRORS;


    //-- Set Material values:
    location = shader.getUniformLocation("material.kd");
    vec3 kd = node.material.kd;
    glUniform3fv(location, 1, value_ptr(kd));
    CHECK_GL_ERRORS;
    location = shader.getUniformLocation("material.ks");
    vec3 ks = node.material.ks;
    glUniform3fv(location, 1, value_ptr(ks));
    CHECK_GL_ERRORS;
    location = shader.getUniformLocation("material.shininess");
    glUniform1f(location, node.material.shininess);
    CHECK_GL_ERRORS;

  }
  shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A5::draw() {

  glEnable( GL_DEPTH_TEST );
  renderSceneGraph(*puppet);
  renderSceneGraph(*level1);

  glDisable( GL_DEPTH_TEST );
  // renderArcCircle();
}



//----------------------------------------------------------------------------------------
void A5::renderSceneGraph(SceneNode & root) {

  // Bind the VAO once here, and reuse for all GeometryNode rendering below.
  glBindVertexArray(m_vao_meshData);

  class AnimationRenderer : public Visitor {
    std::stack<glm::mat4> transforms;
    void visitChildren(std::list<SceneNode*>& children) {
      for (SceneNode * child : children) {
        child->accept(*this);
      }
    }

    glm::mat4 calculateM(const glm::mat4& trans) const {
      assert(transforms.size() > 0);
      return transforms.top() * trans;
    }

    A5& self;
    Keyframe& frame;

  public:
    AnimationRenderer(A5& self, Keyframe& frame) : self(self), frame(frame) {
      transforms.push(self.m_view);
    }

    void visit(SceneNode& node) {
      const glm::mat4 M = calculateM(node.trans);
      transforms.push(M);
      visitChildren(node.children);
      transforms.pop();
    }

    void visit(GeometryNode& node) {
      const glm::mat4 M = calculateM(node.trans);
      updateShaderUniforms(self.m_shader, node, M);

      // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
      BatchInfo batchInfo = self.m_batchInfoMap[node.meshId];

      //-- Now render the mesh:
      self.m_shader.enable();
      glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
      self.m_shader.disable();

      transforms.push(M);
      visitChildren(node.children);
      transforms.pop();
    }

    void visit(JointNode& node) {
      glm::mat4 M = calculateM(node.trans);

      float xAnimationRotation = 0;
      if (frame.rotations.find(node.m_name) != frame.rotations.end()) {
        xAnimationRotation = frame.rotations.at(node.m_name);
      }

      float xRotation = glm::radians(
        glm::clamp(
          node.m_joint_x.init + xAnimationRotation,
          node.m_joint_x.min,
          node.m_joint_x.max
        )
      );

      float yRotation = glm::radians(
        glm::clamp(
          node.m_joint_y.init,
          node.m_joint_y.min,
          node.m_joint_y.max
        )
      );

      M = M * glm::rotate(glm::mat4(), yRotation, glm::vec3(0, 1, 0));
      M = M * glm::rotate(glm::mat4(), xRotation, glm::vec3(1, 0, 0));

      if (frame.positions.find(node.m_name) != frame.positions.end()) {
        M = glm::translate(glm::mat4(), frame.positions.at(node.m_name)) * M;
      }

      transforms.push(M);
      visitChildren(node.children);
      transforms.pop();
    }
  };

  Keyframe frame = playerWalkingAnimation.get(getTime());
  AnimationRenderer renderer{*this, frame};

  root.accept(renderer);

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A5::renderArcCircle() {
  glBindVertexArray(m_vao_arcCircle);

  m_shader_arcCircle.enable();
  GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
  float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
  glm::mat4 M;
  if( aspect > 1.0 ) {
    M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
  } else {
    M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
  }
  glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
  glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
  m_shader_arcCircle.disable();

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A5::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A5::cursorEnterWindowEvent (
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
bool A5::mouseMoveEvent (
  double xPos,
  double yPos
) {
  mouse.prevX = mouse.x;
  mouse.prevY = mouse.y;
  mouse.x = xPos * double(m_framebufferWidth) / double(m_windowWidth);
  mouse.y = (m_windowHeight - yPos) * double(m_framebufferHeight) / double(m_windowHeight);
  return true;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A5::mouseButtonInputEvent (
  int button,
  int action,
  int mods
) {

  if (action == GLFW_PRESS) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT: {
        mouse.isLeftButtonPressed = true;
        return true;
      }
      case GLFW_MOUSE_BUTTON_RIGHT: {
        mouse.isRightButtonPressed = true;
        return true;
      }
      case GLFW_MOUSE_BUTTON_MIDDLE: {
        mouse.isMiddleButtonPressed = true;
        return true;
      }
    }
  }


  if (action == GLFW_RELEASE) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT: {
        mouse.isLeftButtonPressed = false;
        return true;
      }
      case GLFW_MOUSE_BUTTON_RIGHT: {
        mouse.isRightButtonPressed = false;
        return true;
      }
      case GLFW_MOUSE_BUTTON_MIDDLE: {
        mouse.isMiddleButtonPressed = false;
        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A5::mouseScrollEvent (
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
bool A5::windowResizeEvent (
  int width,
  int height
) {
  bool eventHandled(false);
  initPerspectiveMatrix();
  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A5::keyInputEvent (
  int key,
  int action,
  int mods
) {
  bool eventHandled(false);

  if( action == GLFW_PRESS ) {
    if( key == GLFW_KEY_M ) {
      show_gui = !show_gui;
      eventHandled = true;
    }
  }
  // Fill in with event handling code...

  return eventHandled;
}
