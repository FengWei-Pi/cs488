#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <queue>
#include <map>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
  : m_luaSceneFile(luaSceneFile),
    m_positionAttribLocation(0),
    m_normalAttribLocation(0),
    m_vao_meshData(0),
    m_vbo_vertexPositions(0),
    m_vbo_vertexNormals(0),
    m_vao_arcCircle(0),
    m_vbo_arcCircle(0),
    interactionMode(PositionOrientation),
    showCircle(false),
    useZBuffer(true),
    useBackfaceCulling(false),
    useFrontfaceCulling(false),
    isPicking(false)
{
  interactionModeNames[PositionOrientation] = "Position/Orientation (P)";
  interactionModeNames[Joints] = "Joints (J)";
}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
  // Set the background colour.
  glClearColor(0.35, 0.35, 0.35, 1.0);

  createShaderProgram();

  glGenVertexArrays(1, &m_vao_arcCircle);
  glGenVertexArrays(1, &m_vao_meshData);
  enableVertexShaderInputSlots();

  processLuaSceneFile(m_luaSceneFile);

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

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
  // This version of the code treats the Lua file as an Asset,
  // so that you'd launch the program with just the filename
  // of a puppet in the Assets/ directory.
  // std::string assetFilePath = getAssetFilePath(filename.c_str());
  // m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

  // This version of the code treats the main program argument
  // as a straightforward pathname.
  m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
  if (!m_rootNode) {
    std::cerr << "Could not open " << filename << std::endl;
  }
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
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
void A3::enableVertexShaderInputSlots()
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
void A3::uploadVertexDataToVbos (
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
void A3::mapVboDataToVertexShaderInputLocations()
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
void A3::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
  m_view = glm::lookAt(
    vec3(0.0f, 0.0f, 10.0f),
    vec3(0.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, 0.0f)
  );
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
  // World-space position
  m_light.position = vec3(-2.0f, 5.0f, 0.5f);
  m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
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
void A3::appLogic() {
  switch (interactionMode) {
    case PositionOrientation: {
      processPositionOrOrientationChanges();
      break;
    }
    case Joints: {
      processJointChanges();
      break;
    }
  }

  uploadCommonSceneUniforms();
}

void A3::processPositionOrOrientationChanges() {
  // std::cerr
  //   << "Process position or orientation changes"
  //   << std::endl;
}

void A3::processJointChanges() {
  if (mouse.isMiddleButtonPressed) {
    const double scale = 1.0 / 5;
    for (JointNode* joint : selectedJoints) {
      joint->rotateAboutY((mouse.x - mouse.prevX) * scale);
      joint->rotateAboutX(-(mouse.y - mouse.prevY) * scale);
    }
  }
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
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

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Application")) {
      if (ImGui::MenuItem("Reset Position (I)")) {
        resetPosition();
      }

      if (ImGui::MenuItem("Reset Orientation (O)")) {
        resetOrientation();
      }

      if (ImGui::MenuItem("Reset Joints (N)")) {
        resetJoints();
      }

      if (ImGui::MenuItem("Reset All (A)")) {
        resetAll();
      }

      if (ImGui::MenuItem("Quit (Q)")) {
        quit();
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Undo (U)")) {
        undo();
      }

      if (ImGui::MenuItem("Redo (R)")) {
        redo();
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options")) {
      ImGui::Checkbox("Circle (C)", &showCircle);
      ImGui::Checkbox("Z-buffer (Z)", &useZBuffer);
      ImGui::Checkbox("Backface culling (B)", &useBackfaceCulling);
      ImGui::Checkbox("Frontface culling (F)", &useFrontfaceCulling);
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    for (int m = PositionOrientation; m != LastInteractionMode; m++) {
      ImGui::PushID(m);
      if( ImGui::RadioButton( interactionModeNames[m].c_str(), (int*) &interactionMode, m ) ) {
        std::cerr << "Switching to " << m << std::endl;
      }
      ImGui::PopID();
    }

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

    ImGui::Text( "" );
    ImGui::Text( "Selected Joints:" );

    for (JointNode * joint : selectedJoints) {
      ImGui::Text( "%s", joint->m_name.c_str() );
    }
  ImGui::End();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

  glEnable( GL_DEPTH_TEST );
  renderSceneGraph(*m_rootNode);

  glDisable( GL_DEPTH_TEST );
  renderArcCircle();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

  // Bind the VAO once here, and reuse for all GeometryNode rendering below.
  glBindVertexArray(m_vao_meshData);

  // This is emphatically *not* how you should be drawing the scene graph in
  // your final implementation.  This is a non-hierarchical demonstration
  // in which we assume that there is a list of GeometryNodes living directly
  // underneath the root node, and that we can draw them in a loop.  It's
  // just enough to demonstrate how to get geometry and materials out of
  // a GeometryNode and onto the screen.

  // You'll want to turn this into recursive code that walks over the tree.
  // You can do that by putting a method in SceneNode, overridden in its
  // subclasses, that renders the subtree rooted at every node.  Or you
  // could put a set of mutually recursive functions in this class, which
  // walk down the tree from nodes of different types.

  draw(root, m_view);

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

void A3::draw(const SceneNode & root, const glm::mat4& parentModelView) {
  glm::mat4 modelView = parentModelView * root.trans;

  switch (root.m_nodeType) {
    case NodeType::GeometryNode: {
      setModelViewUniforms(m_shader, modelView);

      const GeometryNode * geometryNode = static_cast<const GeometryNode *>(&root);
      setShaderMaterialUniforms(m_shader, *geometryNode);

      // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
      BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

      //-- Now render the mesh:
      m_shader.enable();
      glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
      m_shader.disable();
      break;
    }
    case NodeType::SceneNode: {
      break;
    }
    case NodeType::JointNode: {
      const JointNode * jointNode = static_cast<const JointNode *>(&root);
      float xRotation = glm::radians(glm::clamp(
        jointNode->m_joint_x.init,
        jointNode->m_joint_x.min,
        jointNode->m_joint_x.max
      ));
      float yRotation = glm::radians(glm::clamp(
        jointNode->m_joint_y.init,
        jointNode->m_joint_y.min,
        jointNode->m_joint_y.max
      ));

      modelView = modelView * glm::rotate(glm::mat4(), yRotation, glm::vec3(0, 1, 0));
      modelView = modelView * glm::rotate(glm::mat4(), xRotation, glm::vec3(1, 0, 0));
      break;
    }
  }

  for (const SceneNode * child : root.children) {
    draw(*child, modelView);
  }
}

void A3::setModelViewUniforms(const ShaderProgram & shader, const glm::mat4 & modelView) {
  shader.enable();
  {
    //-- Set ModelView matrix:
    GLint location = shader.getUniformLocation("ModelView");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
    CHECK_GL_ERRORS;

    //-- Set NormMatrix:
    location = shader.getUniformLocation("NormalMatrix");
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
    glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
    CHECK_GL_ERRORS;

  }
  shader.disable();

}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::setShaderMaterialUniforms(const ShaderProgram & shader, const GeometryNode & node) {
  shader.enable();
  {
    glUniform1i( shader.getUniformLocation("isPicking"), isPicking ? 1 : 0 );

    if (isPicking) {
      unsigned int idx = node.m_nodeId;
      float r = float(idx&0xff) / 255.0f;
      float g = float((idx>>8)&0xff) / 255.0f;
      float b = float((idx>>16)&0xff) / 255.0f;

      glUniform3f(shader.getUniformLocation("material.kd"), r, g, b);
      CHECK_GL_ERRORS;
    } else {
      //-- Set Material values:
      vec3 kd = node.material.kd;
      glUniform3fv(shader.getUniformLocation("material.kd"), 1, glm::value_ptr(kd));
      CHECK_GL_ERRORS;
      vec3 ks = node.material.ks;
      glUniform3fv(shader.getUniformLocation("material.ks"), 1, glm::value_ptr(ks));
      CHECK_GL_ERRORS;
      glUniform1f(shader.getUniformLocation("material.shininess"), node.material.shininess);
      CHECK_GL_ERRORS;
    }
  }
  shader.disable();
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
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
void A3::cleanup() {

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
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
bool A3::mouseButtonInputEvent (
    int button,
    int action,
    int mods
) {
  if (action == GLFW_PRESS) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        mouse.isLeftButtonPressed = true;
        if (interactionMode == Joints) {
          pick();
        }
        return true;
      case GLFW_MOUSE_BUTTON_RIGHT:
        mouse.isRightButtonPressed = true;
        return true;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        mouse.isMiddleButtonPressed = true;
        return true;
    }
  }

  if (action == GLFW_RELEASE) {
    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        mouse.isLeftButtonPressed = false;
        return true;
      case GLFW_MOUSE_BUTTON_RIGHT:
        mouse.isRightButtonPressed = false;
        return true;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        mouse.isMiddleButtonPressed = false;
        return true;
    }
  }

  return false;
}

void A3::pick() {
  glClearColor(1.0, 1.0, 1.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glClearColor(0.35, 0.35, 0.35, 1.0);

  isPicking = true;
  draw();
  isPicking = false;

  CHECK_GL_ERRORS;

  GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
  glReadBuffer( GL_BACK );
  glReadPixels( int(mouse.x), int(mouse.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
  CHECK_GL_ERRORS;

  unsigned int selectedNodeId =  buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

  try {
    JointNode* joint = findJoint(selectedNodeId, *m_rootNode);
    if (selectedJoints.find(joint) != selectedJoints.end()) {
      std::cerr << "Unselecting " << *joint << std::endl;
      selectedJoints.erase(joint);
    } else {
      std::cerr << "Selecting " << *joint << std::endl;
      selectedJoints.insert(joint);
    }
  } catch (ChildNotFound& ex) {
  } catch (JointNotFound& ex) {
    if (ex.getSelectedNode().m_name != "torso") {
      throw ex;
    }
  }
}

JointNode* A3::findJoint(const unsigned int id, const SceneNode & root) {
  assert(root.m_nodeId != id);

  std::queue<const SceneNode*> fringe;
  fringe.push(&root);

  std::map<const SceneNode*, const SceneNode*> parents;
  parents[&root] = nullptr;

  while (!fringe.empty()) {
    const SceneNode* const parent = fringe.front();
    fringe.pop();

    for(SceneNode * child : parent->children) {
      parents[child] = parent;
      fringe.push(child);

      if (child->m_nodeId == id) {
        const SceneNode* ancestor = parent;
        while (ancestor != nullptr) {
          if (ancestor->m_nodeType == NodeType::JointNode) {
            return (JointNode *)(ancestor);
          }
          ancestor = parents[ancestor];
        }

        throw JointNotFound(child);
      }
    }
  }

  throw ChildNotFound();
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
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
bool A3::keyInputEvent (
  int key,
  int action,
  int mods
) {
  std::function<bool(bool)> toggle = [](bool x) -> bool {
    return !x;
  };

  if( action == GLFW_PRESS ) {
    switch (key) {
      /**
       * Application Menu
       */
      case GLFW_KEY_I: {
        resetPosition();
        return true;
      }
      case GLFW_KEY_O: {
        resetOrientation();
        return true;
      }
      case GLFW_KEY_N: {
        resetJoints();
        return true;
      }
      case GLFW_KEY_A: {
        resetAll();
        return true;
      }
      case GLFW_KEY_Q: {
        quit();
        return true;
      }

      /**
       * Edit Menu
       */

      case GLFW_KEY_U: {
        undo();
        return true;
      }
      case GLFW_KEY_R: {
        redo();
        return true;
      }

      /**
       * Options Menu
       */

      case GLFW_KEY_C: {
        updateCircle(toggle);
        return true;
      }
      case GLFW_KEY_Z: {
        updateZBuffer(toggle);
        return true;
      }
      case GLFW_KEY_B: {
        updateBackfaceCulling(toggle);
        return true;
      }
      case GLFW_KEY_F: {
        updateFrontfaceCulling(toggle);
        return true;
      }

      case GLFW_KEY_M: {
        show_gui = !show_gui;
        return true;
      }

      case GLFW_KEY_P: {
        interactionMode = PositionOrientation;
        return true;
      }
      case GLFW_KEY_J: {
        interactionMode = Joints;
        return true;
      }
    }

    if( key == GLFW_KEY_M ) {
      show_gui = !show_gui;
      return true;
    }
  }
  // Fill in with event handling code...

  return false;
}

void A3::redo() {
  std::cerr
    << "Redo"
    << std::endl;
}

void A3::undo() {
  std::cerr
    << "Undo"
    << std::endl;
}

void A3::quit() {
  glfwSetWindowShouldClose(m_window, GL_TRUE);
}

/**
 * Resets
 */

void A3::resetAll() {
  resetPosition();
  resetOrientation();
  resetJoints();
}

void A3::resetPosition() {
  std::cerr
    << "Resetting position"
    << std::endl;
}

void A3::resetOrientation() {
  std::cerr
    << "Resetting orientation"
    << std::endl;
}

void A3::resetJoints() {
  std::cerr
    << "Resetting joints"
    << std::endl;
}

/**
 * Switch rendering algorithms
 */

void A3::updateCircle(const std::function<bool(bool)> fn) {
  showCircle = fn(showCircle);
}

void A3::updateZBuffer(const std::function<bool(bool)> fn) {
  useZBuffer = fn(useZBuffer);
}

void A3::updateBackfaceCulling(const std::function<bool(bool)> fn) {
  useBackfaceCulling = fn(useBackfaceCulling);
}

void A3::updateFrontfaceCulling(const std::function<bool(bool)> fn) {
  useFrontfaceCulling = fn(useFrontfaceCulling);
}
