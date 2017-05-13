#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <algorithm>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1() :
  current_col( 0 ),
  previousMouseX(0),
  activeX(0),
  activeY(0),
  isMouseButtonLeftPressed(false)
{
  colour[0] = 0.0f;
  colour[1] = 0.0f;
  colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1() {}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
  // Set the background colour.
  glClearColor( 0.3, 0.5, 0.7, 1.0 );

  // Build the shader
  m_shader.generateProgramObject();
  m_shader.attachVertexShader(
    getAssetFilePath( "VertexShader.vs" ).c_str() );
  m_shader.attachFragmentShader(
    getAssetFilePath( "FragmentShader.fs" ).c_str() );
  m_shader.link();

  // Set up the uniforms
  P_uni = m_shader.getUniformLocation( "P" );
  V_uni = m_shader.getUniformLocation( "V" );
  M_uni = m_shader.getUniformLocation( "M" );
  col_uni = m_shader.getUniformLocation( "colour" );

  initGrid();

  // Set up initial view and projection matrices (need to do this here,
  // since it depends on the GLFW window being set up correctly).
  view = glm::lookAt(
    glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
    glm::vec3( 0.0f, 0.0f, 0.0f ),
    glm::vec3( 0.0f, 1.0f, 0.0f )
  );

  proj = glm::perspective(
    glm::radians( 45.0f ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f,
    1000.0f
  );
}

void A1::initGrid()
{
  // size_t sz = 3 * 2 * 2 * (DIM+3);
  //
  // float *verts = new float[ sz ];
  // size_t ct = 0;
  // // TODO: Why doesn't int idx = -1 work?
  // for( float idx = -1; idx < DIM+2; ++idx ) {
  //   // Left
  //   verts[ ct++ ] = -1;
  //   verts[ ct++ ] = 0;
  //   verts[ ct++ ] = idx;
  //
  //   // Right
  //   verts[ ct++ ] = DIM+1;
  //   verts[ ct++ ] = 0;
  //   verts[ ct++ ] = idx;
  //
  //   // Top
  //   verts[ ct++ ] = idx;
  //   verts[ ct++ ] = 0;
  //   verts[ ct++ ] = -1;
  //
  //   // Bottom
  //   verts[ ct++ ] = idx;
  //   verts[ ct++ ] = 0;
  //   verts[ ct++ ] = DIM+1;
  // }

  size_t sz = 24 * DIM * DIM;
  glm::vec3* verts = new glm::vec3[sz];
  size_t ct = 0;

  for (float z = 0; z < DIM; z += 1) {
    for (float x = 0; x < DIM; x += 1) {
      /**
       * Bottom Square
       */

      // Top Line
      verts[ct++] = glm::vec3(x, 0, z);
      verts[ct++] = glm::vec3(x, 0, z + 1);

      // Left Line
      verts[ct++] = glm::vec3(x, 0, z);
      verts[ct++] = glm::vec3(x + 1, 0, z);

      // Bottom Line
      verts[ct++] = glm::vec3(x + 1, 0, z);
      verts[ct++] = glm::vec3(x + 1, 0, z + 1);

      // Right Line
      verts[ct++] = glm::vec3(x, 0, z + 1);
      verts[ct++] = glm::vec3(x + 1, 0, z + 1);

      /**
       * Top Square
       */

      // Top Line
      verts[ct++] = glm::vec3(x, 1, z);
      verts[ct++] = glm::vec3(x, 1, z + 1);

      // Left Line
      verts[ct++] = glm::vec3(x, 1, z);
      verts[ct++] = glm::vec3(x + 1, 1, z);

      // Bottom Line
      verts[ct++] = glm::vec3(x + 1, 1, z);
      verts[ct++] = glm::vec3(x + 1, 1, z + 1);

      // Right Line
      verts[ct++] = glm::vec3(x, 1, z + 1);
      verts[ct++] = glm::vec3(x + 1, 1, z + 1);

      /**
       * Pillars
       */

      // Top-Left
      verts[ct++] = glm::vec3(x, 0, z);
      verts[ct++] = glm::vec3(x, 1, z);

      // Top-Right
      verts[ct++] = glm::vec3(x + 1, 0, z);
      verts[ct++] = glm::vec3(x + 1, 1, z);

      // Bottom-Left
      verts[ct++] = glm::vec3(x, 0, z + 1);
      verts[ct++] = glm::vec3(x, 1, z + 1);

      // Bottom-Right
      verts[ct++] = glm::vec3(x + 1, 0, z + 1);
      verts[ct++] = glm::vec3(x + 1, 1, z + 1);
    }
  }

  // Create the vertex array to record buffer assignments.
  glGenVertexArrays( 1, &m_grid_vao );
  glBindVertexArray( m_grid_vao );

  // Create the cube vertex buffer
  glGenBuffers( 1, &m_grid_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
  glBufferData( GL_ARRAY_BUFFER, sz*sizeof(glm::vec3), verts, GL_STATIC_DRAW );

  // Specify the means of extracting the position values properly.
  GLint posAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( posAttrib );
  glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

  // Reset state to prevent rogue code from messing with *my*
  // stuff!
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // OpenGL has the buffer now, there's no need for us to keep a copy.
  delete [] verts;

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
  // Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
  // We already know there's only going to be one window, so for
  // simplicity we'll store button states in static local variables.
  // If there was ever a possibility of having multiple instances of
  // A1 running simultaneously, this would break; you'd want to make
  // this into instance fields of A1.
  static bool showTestWindow(false);
  static bool showDebugWindow(true);

  ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
  float opacity(0.5f);

  ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    // Eventually you'll create multiple colour widgets with
    // radio buttons.  If you use PushID/PopID to give them all
    // unique IDs, then ImGui will be able to keep them separate.
    // This is unnecessary with a single colour selector and
    // radio button, but I'm leaving it in as an example.

    // Prefixing a widget name with "##" keeps it from being
    // displayed.

    ImGui::PushID( 0 );
    ImGui::ColorEdit3( "##Colour", colour );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
      // Select this colour.
    }
    ImGui::PopID();

/*
    // For convenience, you can uncomment this to show ImGui's massive
    // demonstration window right in your application.  Very handy for
    // browsing around to get the widget you want.  Then look in
    // shared/imgui/imgui_demo.cpp to see how it's done.
    if( ImGui::Button( "Test Window" ) ) {
      showTestWindow = !showTestWindow;
    }
*/

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();

  if( showTestWindow ) {
    ImGui::ShowTestWindow( &showTestWindow );
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
  // Create a global transformation for the model (centre it).
  mat4 W;
  W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

  m_shader.enable();
    glEnable( GL_DEPTH_TEST );

    glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
    glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

    // Just draw the grid for now.
    glBindVertexArray( m_grid_vao );
    glUniform3f( col_uni, 1, 1, 1 );
    glDrawArrays( GL_LINES, 0, 24 * DIM * DIM );

    // Draw the cubes
    // Highlight the active square.
  m_shader.disable();

  // Restore defaults
  glBindVertexArray( 0 );

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup() {

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) {

  if (!ImGui::IsMouseHoveringAnyWindow()) {
    // Put some code here to handle rotations.  Probably need to
    // check whether we're *dragging*, not just moving the mouse.
    // Probably need some instance variables to track the current
    // rotation amount, and maybe the previous X position (so
    // that you can rotate relative to the *change* in X.

    if (isMouseButtonLeftPressed) {
      float diff = (xPos - previousMouseX) / 200;
      view = glm::rotate(view, diff, glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }

  previousMouseX = xPos;
  return true;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
  if (!ImGui::IsMouseHoveringAnyWindow()) {
    // The user clicked in the window.  If it's the left
    // mouse button, initiate a rotation.
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (actions == GLFW_PRESS) {
        isMouseButtonLeftPressed = true;
        return true;
      }

      if (actions == GLFW_RELEASE) {
        isMouseButtonLeftPressed = false;
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
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
  bool eventHandled(false);

  // Zoom in or out.

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
  // Fill in with event handling code...
  if( action == GLFW_PRESS ) {
    switch (key) {
      case GLFW_KEY_LEFT:
        activeX = std::max(0, activeX - 1);
        return true;
      case GLFW_KEY_UP:
        activeY = std::max(0, activeY - 1);
        return true;
      case GLFW_KEY_RIGHT:
        activeX = std::min((int)DIM, activeX + 1);
        return true;
      case GLFW_KEY_DOWN:
        activeY = std::min((int)DIM, activeY + 1);
        return true;
    }
  }

  return true;
}
