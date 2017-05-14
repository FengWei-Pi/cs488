#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <chrono>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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

private:
	void initGrid();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object
  GLuint m_grid_color_vbo;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	int selected_color;
  glm::vec3 colors[8];
  int ** colorAssignment;

  bool isMouseButtonLeftPressed;
  int previousMouseX;
  int activeX;
  int activeZ;
  bool shiftPressed;
  float currentZoom;
  std::chrono::high_resolution_clock::time_point t_start;

  glm::mat4 getInitialView();
  glm::mat4 getInitialPerspective();
  void initState();
  void updateActiveBarHeight(std::function<float(float)> fn);
  void changeActiveBar(int z, int x);
  void updateBarColor(int z, int x, std::function<glm::vec4(glm::vec4)> fn);
  void updateActiveBarToSelectedColor();
  float getTime();
  void reset();
};
