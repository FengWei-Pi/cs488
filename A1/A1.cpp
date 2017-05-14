#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>

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
  selected_color(0),
  previousMouseX(0),
  activeX(0),
  activeZ(0),
  isMouseButtonLeftPressed(false),
  colorAssignment(new int*[DIM]),
  t_start(std::chrono::high_resolution_clock::now()),
  shiftPressed(false),
  currentZoom(1) {
  for (int z = 0; z < DIM; z += 1) {
    colorAssignment[z] = new int[DIM];
  }

  initState();
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1() {
  for (int z = 0; z < DIM; z += 1) {
    delete [] colorAssignment[z];
  }

  delete [] colorAssignment;
}

void A1::initState() {
  colors[0] = glm::vec3(118, 176, 65);
  colors[1] = glm::vec3(23, 190, 187);
  colors[2] = glm::vec3(255, 201, 20);
  colors[3] = glm::vec3(46, 40, 42);
  colors[4] = glm::vec3(228, 87, 46);
  colors[5] = glm::vec3(0, 21, 20);
  colors[6] = glm::vec3(251, 255, 254);
  colors[7] = glm::vec3(163, 0, 0);

  for (glm::vec3& color : colors) {
    color.y = color.y / 255.0;
    color.x = color.x / 255.0;
    color.z = color.z / 255.0;
  }

  for (int z = 0; z < DIM; z += 1) {
    for (int x = 0; x < DIM; x += 1) {
      colorAssignment[z][x] = 3;
    }
  }

  colorAssignment[activeZ][activeX] = selected_color;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init() {
  // Set the background color.
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

  initGrid();

  // Set up initial view and projection matrices (need to do this here,
  // since it depends on the GLFW window being set up correctly).
  view = getInitialView();
  proj = getInitialPerspective();
}

glm::mat4 A1::getInitialView() {
  return glm::lookAt(
    glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
    glm::vec3( 0.0f, 0.0f, 0.0f ),
    glm::vec3( 0.0f, 1.0f, 0.0f )
  );
}

glm::mat4 A1::getInitialPerspective() {
  return glm::perspective(
    glm::radians( 45.0f ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f,
    1000.0f
  );
}

void A1::initGrid() {
  size_t sz = 36 * (DIM * DIM) + 24;
  glm::vec3* verts = new glm::vec3[sz];
  glm::vec4* vertexColors = new glm::vec4[sz];
  size_t ct = 0;
  float initialHeight = 0;

  for (int z = 0; z < DIM; z += 1) {
    for (int x = 0; x < DIM; x += 1, ct += 36) {
      /**
       * Bottom Square
       */

      verts[ct + 0] = glm::vec3(x, 0, z);
      verts[ct + 1] = glm::vec3(x + 1, 0, z);
      verts[ct + 2] = glm::vec3(x, 0, z + 1);

      verts[ct + 3] = glm::vec3(x + 1, 0, z + 1);
      verts[ct + 4] = glm::vec3(x + 1, 0, z);
      verts[ct + 5] = glm::vec3(x, 0, z + 1);

      /**
       * Top Square
       */

      verts[ct + 6] = glm::vec3(x, initialHeight, z);
      verts[ct + 7] = glm::vec3(x + 1, initialHeight, z);
      verts[ct + 8] = glm::vec3(x, initialHeight, z + 1);

      verts[ct + 9] = glm::vec3(x + 1, initialHeight, z + 1);
      verts[ct + 10] = glm::vec3(x + 1, initialHeight, z);
      verts[ct + 11] = glm::vec3(x, initialHeight, z + 1);

      /**
       * Left Square
       */
      verts[ct + 12] = glm::vec3(x, 0, z + 1);
      verts[ct + 13] = glm::vec3(x, 0, z);
      verts[ct + 14] = glm::vec3(x, initialHeight, z + 1);

      verts[ct + 15] = glm::vec3(x, initialHeight, z);
      verts[ct + 16] = glm::vec3(x, 0, z);
      verts[ct + 17] = glm::vec3(x, initialHeight, z + 1);

      /**
       * Right Square
       */
      verts[ct + 18] = glm::vec3(x + 1, 0, z + 1);
      verts[ct + 19] = glm::vec3(x + 1, 0, z);
      verts[ct + 20] = glm::vec3(x + 1, initialHeight, z + 1);

      verts[ct + 21] = glm::vec3(x + 1, initialHeight, z);
      verts[ct + 22] = glm::vec3(x + 1, 0, z);
      verts[ct + 23] = glm::vec3(x + 1, initialHeight, z + 1);

      /**
       * Back Square
       */
      verts[ct + 24] = glm::vec3(x, 0, z);
      verts[ct + 25] = glm::vec3(x, initialHeight, z);
      verts[ct + 26] = glm::vec3(x + 1, 0, z);

      verts[ct + 27] = glm::vec3(x + 1, initialHeight, z);
      verts[ct + 28] = glm::vec3(x, initialHeight, z);
      verts[ct + 29] = glm::vec3(x + 1, 0, z);

      /**
       * Front Square
       */
      verts[ct + 30] = glm::vec3(x, 0, z + 1);
      verts[ct + 31] = glm::vec3(x, initialHeight, z + 1);
      verts[ct + 32] = glm::vec3(x + 1, 0, z + 1);

      verts[ct + 33] = glm::vec3(x + 1, initialHeight, z + 1);
      verts[ct + 34] = glm::vec3(x, initialHeight, z + 1);
      verts[ct + 35] = glm::vec3(x + 1, 0, z + 1);

      /**
       * Assign colors
       */
      for (int i = 0; i < 36; i++) {
        glm::vec3& assignedColor = colors[colorAssignment[z][x]];
        vertexColors[ct + i][0] = assignedColor.x;
        vertexColors[ct + i][1] = assignedColor.y;
        vertexColors[ct + i][2] = assignedColor.z;
        vertexColors[ct + i][3] = 1;
      }
    }
  }

  /**
   * Left Dead Tile
   */

  verts[ct + 0] = glm::vec3(-1, 0, -1);
  verts[ct + 1] = glm::vec3(0, 0, -1);
  verts[ct + 2] = glm::vec3(-1, 0, DIM + 1);

  verts[ct + 3] = glm::vec3(0, 0, DIM + 1);
  verts[ct + 4] = glm::vec3(0, 0, -1);
  verts[ct + 5] = glm::vec3(-1, 0, DIM + 1);

  /**
   * Right Dead tile
   */

  verts[ct + 6] = glm::vec3(DIM, 0, -1);
  verts[ct + 7] = glm::vec3(DIM + 1, 0, -1);
  verts[ct + 8] = glm::vec3(DIM, 0, DIM + 1);

  verts[ct + 9] = glm::vec3(DIM + 1, 0, DIM + 1);
  verts[ct + 10] = glm::vec3(DIM + 1, 0, -1);
  verts[ct + 11] = glm::vec3(DIM, 0, DIM + 1);

  /**
   * Top Dead Tile
   */

  verts[ct + 12] = glm::vec3(0, 0, -1);
  verts[ct + 13] = glm::vec3(DIM, 0, -1);
  verts[ct + 14] = glm::vec3(0, 0, 0);

  verts[ct + 15] = glm::vec3(DIM, 0, 0);
  verts[ct + 16] = glm::vec3(DIM, 0, -1);
  verts[ct + 17] = glm::vec3(0, 0, 0);

  /**
   * Bottom Dead tile
   */

  verts[ct + 18] = glm::vec3(0, 0, DIM);
  verts[ct + 19] = glm::vec3(DIM, 0, DIM);
  verts[ct + 20] = glm::vec3(0, 0, DIM + 1);

  verts[ct + 21] = glm::vec3(DIM, 0, DIM + 1);
  verts[ct + 22] = glm::vec3(DIM, 0, DIM);
  verts[ct + 23] = glm::vec3(0, 0, DIM + 1);

  for (int i = 0; i < 24; i++) {
    vertexColors[ct + i] = glm::vec4(1, 1, 1, 0.75);
  }

  // Create the vertex array to record buffer assignments.
  glGenVertexArrays( 1, &m_grid_vao );
  glBindVertexArray( m_grid_vao );

  /**
   * Pass Vertices to OpenGL
   */

  // Create the cube vertex buffer
  glGenBuffers( 1, &m_grid_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
  glBufferData( GL_ARRAY_BUFFER, sz*sizeof(glm::vec3), verts, GL_DYNAMIC_DRAW );

  // Specify the means of extracting the position values properly.
  GLint posAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( posAttrib );
  glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

  /**
   * Pass colors to OpenGL
   */

  // Create the cube vertex buffer for colors
  glGenBuffers( 1, &m_grid_color_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_color_vbo );
  glBufferData( GL_ARRAY_BUFFER, sz*sizeof(glm::vec4), vertexColors, GL_DYNAMIC_DRAW );

  // Specify the means of extracting the position values properly.
  GLint vertexColorAttrib = m_shader.getAttribLocation( "vertexColor" );
  glEnableVertexAttribArray( vertexColorAttrib );
  glVertexAttribPointer( vertexColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, nullptr );

  // Reset state to prevent rogue code from messing with *my*
  // stuff!
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // OpenGL has the buffer now, there's no need for us to keep a copy.
  delete [] verts;
  delete [] vertexColors;

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic() {
  float time = getTime();
  const float y = std::sin(time * 4.0);
  const glm::vec3& color = colors[selected_color];
  const float r = 0.05 * y + color.r;
  const float g = 0.05 * y + color.g;
  const float b = 0.05 * y + color.b;
  const float a = 0.05 * y + 0.5f;

  updateBarColor(activeZ, activeX, [r, g, b, a](glm::vec4 _) -> glm::vec4 {
    return glm::vec4(r, g, b, a);
  });
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
    if( ImGui::Button( "Reset" ) ) {
      reset();
    }

    ImGui::SameLine();

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

    int id = 0;
    for (glm::vec3 &color : colors) {
      ImGui::PushID( id );
      if ( ImGui::ColorEdit3( "##Color", glm::value_ptr(color) ) ) {
        for (int z = 0; z < DIM; z += 1) {
          for (int x = 0; x < DIM; x += 1) {
            if (colorAssignment[z][x] == id) {
              updateBarColor(z, x, [this, id](glm::vec4 old) -> glm::vec4 {
                return glm::vec4(colors[id], old.a);
              });
            }
          }
        }
      };
      ImGui::SameLine();
      if( ImGui::RadioButton( "##Col", &selected_color, id ) ) {
        updateActiveBarToSelectedColor();
      }
      ImGui::PopID();

      id += 1;
    }

    // For convenience, you can uncomment this to show ImGui's massive
    // demonstration window right in your application.  Very handy for
    // browsing around to get the widget you want.  Then look in
    // shared/imgui/imgui_demo.cpp to see how it's done.
    // if( ImGui::Button( "Test Window" ) ) {
    //   showTestWindow = !showTestWindow;
    // }

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
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawArrays( GL_TRIANGLES, 0, 36 * (DIM * DIM) + 24 );
    glDrawArrays( GL_LINES, 0, 36 * (DIM * DIM) + 24 );

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
bool A1::mouseScrollEvent(double xOffset, double yOffset) {
  bool eventHandled(false);
  float factor = 1.05f;

  if (yOffset < 0) {
    if (currentZoom * factor < 1.5) {
      currentZoom = currentZoom * factor;
      view = glm::scale(view, glm::vec3(factor, factor, factor));
    }
  } else if (currentZoom / factor > 0.5) {
    currentZoom = currentZoom / factor;
    view = glm::scale(view, glm::vec3(1/factor, 1/factor, 1/factor));
  }

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
  //
  if( action == GLFW_PRESS ) {
    switch (key) {
      case GLFW_KEY_LEFT:
        changeActiveBar(activeZ, activeX - 1);
        return true;
      case GLFW_KEY_UP:
        changeActiveBar(activeZ - 1, activeX);
        return true;
      case GLFW_KEY_RIGHT:
        changeActiveBar(activeZ, activeX + 1);
        return true;
      case GLFW_KEY_DOWN:
        changeActiveBar(activeZ + 1, activeX);
        return true;
      case GLFW_KEY_SPACE:
        updateActiveBarHeight([](float old) -> float {
          return old + 1;
        });
        return true;
      case GLFW_KEY_BACKSPACE:
        updateActiveBarHeight([](float old) -> float {
          return old - 1;
        });
        return true;
      case GLFW_KEY_LEFT_SHIFT:
      case GLFW_KEY_RIGHT_SHIFT:
        shiftPressed = true;
        return true;
      case GLFW_KEY_R:
        reset();
        return true;
    }
  }

  if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_LEFT_SHIFT:
      case GLFW_KEY_RIGHT_SHIFT:
        shiftPressed = false;
        return true;
    }
  }

  return true;
}

void A1::updateActiveBarHeight(std::function<float(float)> fn) {
  glBindVertexArray( m_grid_vao );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );

  int cubeSize = sizeof(glm::vec3) * 36;
  GLintptr offset = (DIM * activeZ + activeX) * cubeSize;
  GLsizeiptr length = cubeSize;

  glm::vec3* verts = (glm::vec3*)glMapBufferRange(
    GL_ARRAY_BUFFER,
    offset,
    length,
    GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
  );

  float minHeight = 0;
  float maxHeight = 8;

  int elevatedPoints[] = {
    6, 7, 8, 9, 10, 11, 14, 15, 17, 20, 21, 23, 25, 27, 28, 31, 33, 34
  };

  for (const int &i : elevatedPoints) {
    verts[i].y = glm::clamp(fn(verts[i].y), minHeight, maxHeight);
  }

  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

void A1::updateBarColor(int z, int x, std::function<glm::vec4(glm::vec4)> fn) {
  glBindVertexArray( m_grid_vao );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_color_vbo );

  int cubeSize = sizeof(glm::vec4) * 36;
  GLintptr offset = (DIM * z + x) * cubeSize;
  GLsizeiptr length = cubeSize;

  glm::vec4* colors = (glm::vec4*)glMapBufferRange(
    GL_ARRAY_BUFFER,
    offset,
    length,
    GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
  );

  for (int i = 0; i < 36; i++) {
    colors[i] = fn(colors[i]);
  }

  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

void A1::changeActiveBar(int z, int x) {
  if (z == activeZ && x == activeX) {
    return;
  }

  int oldActiveX = activeX;
  int oldActiveZ = activeZ;

  /**
   * Restore previously assigned color alpha
   */

  updateBarColor(activeZ, activeX, [this](glm::vec4 _) -> glm::vec4 {
    return glm::vec4(colors[colorAssignment[activeZ][activeX]], 1);
  });

  activeZ = glm::clamp(z, 0, (int) (DIM - 1));
  activeX = glm::clamp(x, 0, (int) (DIM - 1));

  updateActiveBarToSelectedColor();

  if (shiftPressed) {
    glBindVertexArray( m_grid_vao );
    glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );

    int cubeSize = sizeof(glm::vec3) * 36;

    glm::vec3* oldActiveBar = (glm::vec3*)glMapBufferRange(
      GL_ARRAY_BUFFER,
      (DIM * oldActiveZ + oldActiveX) * cubeSize,
      cubeSize,
      GL_MAP_READ_BIT | GL_MAP_WRITE_BIT
    );

    int oldActiveBarHeight = oldActiveBar[6].y;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    updateActiveBarHeight([oldActiveBarHeight](float _) -> float {
      return oldActiveBarHeight;
    });
  }
}

void A1::updateActiveBarToSelectedColor() {
  colorAssignment[activeZ][activeX] = selected_color;
  updateBarColor(activeZ, activeX, [this](glm::vec4 _) -> glm::vec4 {
    return glm::vec4(colors[selected_color], 1);
  });
}

float A1::getTime() {
  auto t_now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
}

void A1::reset() {
  selected_color = 0;
  previousMouseX = 0;
  activeX = 0;
  activeZ = 0;
  isMouseButtonLeftPressed = false;
  t_start = std::chrono::high_resolution_clock::now();
  shiftPressed = false;
  currentZoom = 1;

  initState();
  initGrid();

  view = getInitialView();
  proj = getInitialPerspective();
}
