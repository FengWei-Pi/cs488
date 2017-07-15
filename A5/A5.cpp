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
#include "Clock.hpp"
#include "SceneNodeFunctor.hpp"
#include "TransformationCollector.hpp"
#include "AnimationTransformationReducer.hpp"
#include "StaticTransformationReducer.hpp"
#include "Util.hpp"
#include "lodepng/lodepng.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <stack>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <cmath>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A5::A5()
  : m_vao_meshData(0),
    m_vbo_vertexPositions(0),
    m_vbo_vertexNormals(0),
    animationStartTime(Clock::getTime()),
    playerWalkingAnimation(Animation::getPlayerWalkingAnimation()),
    playerStandingAnimation(Animation::getPlayerStandingAnimation()),
    playerPreparingToJumpAnimation(Animation::getPlayerPreparingToJumpAnimation()),
    playerJumpingAnimation(Animation::getPlayerJumpingAnimation()),
    currentAnimation(nullptr),
    gameCamera(glm::radians(0.0f), glm::radians(0.0f), 1),
    minimapCamera(glm::radians(135.0f), glm::radians(0.0f), 1),
    SHADOW_WIDTH(2048),
    SHADOW_HEIGHT(2048),
    playerStateManager(INIT)
{
  const uint size = 6;

  blocks.push_back(Platform(glm::vec3(-2, -1, -2), glm::vec3(size, 1, size), 1, 100));
  platformUpdateVFns[blocks.back().id] = createSinusoid(0.0, 4, 0, 0);
  platformTimes[blocks.back().id] = 0;

  blocks.push_back(Platform(glm::vec3(-2, -1, -2 + (size + 3)), glm::vec3(size, 1, size), 1, 100));
  platformUpdateVFns[blocks.back().id] = createSinusoid(2, 4, 2, 0);
  platformTimes[blocks.back().id] = 0;

  blocks.push_back(Platform(glm::vec3(-2, -1, + (size + 3) * 2), glm::vec3(size, 1, size), 1, 100));
  platformUpdateVFns[blocks.back().id] = createSinusoid(2, 4, 1, 0);
  platformTimes[blocks.back().id] = 0;

  player.mass = 1;
  player.speed = 6;
  player.g = world.F_g / player.mass;

  playerStateManager.addState(WALKING, [this](PlayerState oldState) -> void {
    animationStartTime = Clock::getTime();
    currentAnimation = &playerWalkingAnimation;
  });

  playerStateManager.addState(STANDING, [this](PlayerState oldState) -> void {
    animationStartTime = Clock::getTime();
    currentAnimation = &playerStandingAnimation;
  });

  playerStateManager.addState(PREPARING_TO_JUMP, [this](PlayerState oldState) -> void {
    animationStartTime = Clock::getTime();
    currentAnimation = &playerPreparingToJumpAnimation;
  });

  playerStateManager.addState(AIRBORN, [this](PlayerState oldState) -> void {
    animationStartTime = Clock::getTime();
    if (oldState == PREPARING_TO_JUMP) {
      currentAnimation = &playerJumpingAnimation;
    } else {
      currentAnimation = &playerStandingAnimation;
    }
  });

  playerStateManager.transition(AIRBORN);
}

std::function<glm::vec3(double)> A5::createSinusoid(
  const double A,
  const double period,
  const double k,
  const double offset
) {
  const double PI = glm::radians(180.0f);
  return [PI, A, k, period, offset](double t) -> glm::vec3 {
    return (A * std::sin(2 * PI / period * (t - k)) + offset) * glm::vec3(1, 0, 0);
  };
}

//----------------------------------------------------------------------------------------
// Destructor
A5::~A5() {

}

std::string convertInternalFormatToString(GLenum format);
std::string getTextureParameters(GLuint id);
std::string getRenderbufferParameters(GLuint id);

///////////////////////////////////////////////////////////////////////////////
// print out the FBO infos
///////////////////////////////////////////////////////////////////////////////
void printFramebufferInfo(GLuint fbo)
{
    // bind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::cout << "\n===== FBO STATUS =====\n";

    // print max # of colorbuffers supported by FBO
    int colorBufferCount = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &colorBufferCount);
    std::cout << "Max Number of Color Buffer Attachment Points: " << colorBufferCount << std::endl;

    // get max # of multi samples
    int multiSampleCount = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &multiSampleCount);
    std::cout << "Max Number of Samples for MSAA: " << multiSampleCount << std::endl;

    int objectType;
    int objectId;

    // print info of the colorbuffer attachable image
    for(int i = 0; i < colorBufferCount; ++i)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_COLOR_ATTACHMENT0+i,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                              &objectType);
        if(objectType != GL_NONE)
        {
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                  GL_COLOR_ATTACHMENT0+i,
                                                  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                  &objectId);

            std::string formatName;

            std::cout << "Color Attachment " << i << ": ";
            if(objectType == GL_TEXTURE)
            {
                std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            }
            else if(objectType == GL_RENDERBUFFER)
            {
                std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            }
        }
    }

    // print info of the depthbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_DEPTH_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                              &objectId);

        std::cout << "Depth Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            break;
        case GL_RENDERBUFFER:
            std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            break;
        }
    }

    // print info of the stencilbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                          GL_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_STENCIL_ATTACHMENT,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                              &objectId);

        std::cout << "Stencil Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            std::cout << "GL_TEXTURE, " << getTextureParameters(objectId) << std::endl;
            break;
        case GL_RENDERBUFFER:
            std::cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << std::endl;
            break;
        }
    }

    std::cout << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////
// return texture parameters as string using glGetTexLevelParameteriv()
///////////////////////////////////////////////////////////////////////////////
std::string getTextureParameters(GLuint id)
{
    if(glIsTexture(id) == GL_FALSE)
        return "Not texture object";

    int width, height, format;
    std::string formatName;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);            // get texture width
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);          // get texture height
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format); // get texture internal format
    glBindTexture(GL_TEXTURE_2D, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// return renderbuffer parameters as string using glGetRenderbufferParameteriv
///////////////////////////////////////////////////////////////////////////////
std::string getRenderbufferParameters(GLuint id)
{
    if(glIsRenderbuffer(id) == GL_FALSE)
        return "Not Renderbuffer object";

    int width, height, format, samples;
    std::string formatName;
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);       // get renderbuffer width
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);     // get renderbuffer height
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format); // get renderbuffer internal format
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);   // get multisample count
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName << ", MSAA(" << samples << ")";
    return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// convert OpenGL internal format enum to string
///////////////////////////////////////////////////////////////////////////////
std::string convertInternalFormatToString(GLenum format)
{
    std::string formatName;

    switch(format)
    {
    case GL_STENCIL_INDEX:      // 0x1901
        formatName = "GL_STENCIL_INDEX";
        break;
    case GL_DEPTH_COMPONENT:    // 0x1902
        formatName = "GL_DEPTH_COMPONENT";
        break;
    case GL_ALPHA:              // 0x1906
        formatName = "GL_ALPHA";
        break;
    case GL_RGB:                // 0x1907
        formatName = "GL_RGB";
        break;
    case GL_RGBA:               // 0x1908
        formatName = "GL_RGBA";
        break;
    case GL_R3_G3_B2:           // 0x2A10
        formatName = "GL_R3_G3_B2";
        break;
    case GL_RGB4:               // 0x804F
        formatName = "GL_RGB4";
        break;
    case GL_RGB5:               // 0x8050
        formatName = "GL_RGB5";
        break;
    case GL_RGB8:               // 0x8051
        formatName = "GL_RGB8";
        break;
    case GL_RGB10:              // 0x8052
        formatName = "GL_RGB10";
        break;
    case GL_RGB12:              // 0x8053
        formatName = "GL_RGB12";
        break;
    case GL_RGB16:              // 0x8054
        formatName = "GL_RGB16";
        break;
    case GL_RGBA2:              // 0x8055
        formatName = "GL_RGBA2";
        break;
    case GL_RGBA4:              // 0x8056
        formatName = "GL_RGBA4";
        break;
    case GL_RGB5_A1:            // 0x8057
        formatName = "GL_RGB5_A1";
        break;
    case GL_RGBA8:              // 0x8058
        formatName = "GL_RGBA8";
        break;
    case GL_RGB10_A2:           // 0x8059
        formatName = "GL_RGB10_A2";
        break;
    case GL_RGBA12:             // 0x805A
        formatName = "GL_RGBA12";
        break;
    case GL_RGBA16:             // 0x805B
        formatName = "GL_RGBA16";
        break;
    case GL_DEPTH_COMPONENT16:  // 0x81A5
        formatName = "GL_DEPTH_COMPONENT16";
        break;
    case GL_DEPTH_COMPONENT24:  // 0x81A6
        formatName = "GL_DEPTH_COMPONENT24";
        break;
    case GL_DEPTH_COMPONENT32:  // 0x81A7
        formatName = "GL_DEPTH_COMPONENT32";
        break;
    case GL_DEPTH_STENCIL:      // 0x84F9
        formatName = "GL_DEPTH_STENCIL";
        break;
    case GL_RGBA32F:            // 0x8814
        formatName = "GL_RGBA32F";
        break;
    case GL_RGB32F:             // 0x8815
        formatName = "GL_RGB32F";
        break;
    case GL_RGBA16F:            // 0x881A
        formatName = "GL_RGBA16F";
        break;
    case GL_RGB16F:             // 0x881B
        formatName = "GL_RGB16F";
        break;
    case GL_DEPTH24_STENCIL8:   // 0x88F0
        formatName = "GL_DEPTH24_STENCIL8";
        break;
    default:
        std::stringstream ss;
        ss << "Unknown Format(0x" << std::hex << format << ")" << std::ends;
        formatName = ss.str();
    }

    return formatName;
}

void printDepthTexture(const uint WIDTH, const uint HEIGHT) {
  float data[WIDTH * HEIGHT];
  glReadPixels(0, 0, WIDTH, HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, data);

  double sum = 0;
  double min = 1000;
  double max = -1000;
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      float v = data[i * WIDTH + j];
      sum += v;

      if (v < min) {
        min = v;
      }

      if (v > max) {
        max = v;
      }
    }
  }

  std::cerr << "Sum depth: " << sum << std::endl;
  std::cerr << "Min depth: " << min << std::endl;
  std::cerr << "Max depth: " << max << std::endl;
  std::cerr << "Average depth: " << sum / (WIDTH * HEIGHT) << std::endl;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A5::init()
{

  // Set the background colour.
  glClearColor(0.35, 0.35, 0.35, 1.0);

  initShaderProgram(m_shader, "Main");
  initShaderProgram(m_shader_depth, "simpleDepthShader");
  initShaderProgram(m_shader_quad, "quadShader");
  initShaderProgram(m_shader_skybox, "skybox");

  puppetSceneNode = readLuaSceneFile(getAssetFilePath("puppet.lua"));
  blockSceneNode = readLuaSceneFile(getAssetFilePath("block.lua"));

  // Load and decode all .obj files at once here.  You may add additional .obj files to
  // this list in order to support rendering additional mesh types.  All vertex
  // positions, and normals will be extracted and stored within the MeshConsolidator
  // class.
  unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
    getAssetFilePath("cube.obj")
  });


  // Acquire the BatchInfoMap from the MeshConsolidator.
  meshConsolidator->getBatchInfoMap(m_batchInfoMap);

  // Create vertex array for mesh data
  glGenVertexArrays(1, &m_vao_meshData);

  // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
  uploadVertexDataToVbos(*meshConsolidator);

  initPerspectiveMatrix();

  initViewMatrix();

  initLightSources();

  {
    // Skybox code
    std::vector<std::string> faces{
      getAssetFilePath("skybox/DarkStormy/darkstormy_rt.png"),
      getAssetFilePath("skybox/DarkStormy/darkstormy_lf.png"),
      getAssetFilePath("skybox/DarkStormy/darkstormy_up.png"),
      getAssetFilePath("skybox/DarkStormy/darkstormy_dn.png"),
      getAssetFilePath("skybox/DarkStormy/darkstormy_bk.png"),
      getAssetFilePath("skybox/DarkStormy/darkstormy_ft.png")
    };

    skyboxTexture = loadCubemap(faces);

    float skyboxVertices[] = {
      // positions
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    // Generate vertex buffer
    glGenBuffers(1, &skyboxVBO);

    // Upload data to vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    CHECK_GL_ERRORS;
  }

  {
    // Create a texture to render to
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Initialize depth buffer
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Initialize depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Make samples outside the border be equal to the border
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create a framebuffer
    glGenFramebuffers(1, &FramebufferName);
    CHECK_GL_ERRORS;

    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    CHECK_GL_ERRORS;

    // Set framebuffer attachements
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    if (false) {
      std::cerr << "Using depth render-buffer" << std::endl;
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    } else {
      std::cerr << "Using depth texture" << std::endl;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    }

    assert(
      glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
    );

    printFramebufferInfo(FramebufferName);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // The fullscreen quad's FBO
  static const GLfloat g_quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
  };

  // Generate vertex buffer
  glGenBuffers(1, &quad_vertexbuffer);

  // Upload data to vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
  CHECK_GL_ERRORS;

  refreshMinimapViewportDimensions();

  /**
   * Create device and context
   */

   device = alcOpenDevice(NULL);
   assert(device);
   checkOpenALErrors();

   context = alcCreateContext(device, NULL);
   assert(alcMakeContextCurrent(context));
   checkOpenALErrors();

   // One listener in the application. Refresh position and velocity of listener.
   repositionAndReorientListener();

   alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
   checkOpenALErrors();

   // Player source
   alGenSources((ALuint)1, &playerSource);
   checkOpenALErrors();
   repositionPlayerSource();

   // Player's noises don't loop
   alSourcei(playerSource, AL_LOOPING, AL_FALSE);
   checkOpenALErrors();

   alSourcef(playerSource, AL_ROLLOFF_FACTOR, 1);
   alSourcef(playerSource, AL_REFERENCE_DISTANCE, 10);

   // Wind source
   alGenSources(1, &windSource);
   checkOpenALErrors();
   repositionWindSource();

   // Wind sound loops
   alSourcei(windSource, AL_LOOPING, AL_TRUE);
   checkOpenALErrors();

   alSourcef(windSource, AL_ROLLOFF_FACTOR, 1);
   alSourcef(windSource, AL_REFERENCE_DISTANCE, 15);

   // https://www.zapsplat.com/music/strong-howling-wind-internal-recording/
   playSoundWithSource(windSource, "wind.wav");

   // Texture
   tileTexture = createTexture2D(getAssetFilePath("tiles.png"));
}


void A5::repositionAndReorientListener() {
  glm::vec3 lookAt = -(gameCamera.getXYRotationMatrix() * glm::vec3(0, 5, -10));
  glm::vec3 lookFrom = player.position - lookAt;
  glm::vec3 up = glm::vec3{0, 1, 0};

  // OpenAL expects the lookAt and up vectors to be linearly independent
  assert(glm::length(glm::cross(lookAt, up)) >= 0.0001);

  alListenerfv(AL_POSITION, glm::value_ptr(lookFrom));
  checkOpenALErrors();

  alListenerfv(AL_VELOCITY, glm::value_ptr(player.getVelocity()));
  checkOpenALErrors();

  ALfloat orientation[] = {lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z};
  alListenerfv(AL_ORIENTATION, orientation);
  checkOpenALErrors();
}

void A5::repositionWindSource() {
  alSourcefv(windSource, AL_POSITION, glm::value_ptr(player.position));
  checkOpenALErrors();

  alSourcefv(windSource, AL_VELOCITY, glm::value_ptr(player.getVelocity()));
  checkOpenALErrors();
}

void A5::repositionPlayerSource() {
  alSourcefv(playerSource, AL_POSITION, glm::value_ptr(player.position));
  checkOpenALErrors();

  alSourcefv(playerSource, AL_VELOCITY, glm::value_ptr(player.getVelocity()));
  checkOpenALErrors();
}

void A5::checkOpenALErrors() {
  int error = alGetError();
  if (error) {
    printf("%s\n", alutGetErrorString(error));
    exit(1);
  }
}

GLuint A5::loadCubemap(std::vector<std::string> faces) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

  for (unsigned int i = 0; i < faces.size(); i += 1) {
    std::vector<unsigned char> image; //the raw pixels
    unsigned int width, height;

    unsigned int error = lodepng::decode(image, width, height, faces.at(i).c_str());

    //if there's an error, display it
    if(error) {
      std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
      assert(false);
    }

    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      &image[0]
    );
    CHECK_GL_ERRORS;
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureId;
}

GLuint A5::createTexture2D(std::string filename) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  std::vector<unsigned char> image; //the raw pixels
  unsigned int width, height;

  unsigned int error = lodepng::decode(image, width, height, filename.c_str());

  //if there's an error, display it
  if(error) {
    std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    assert(false);
  }

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    width,
    height,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    &image[0]
  );
  CHECK_GL_ERRORS;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureId;
}

std::shared_ptr<SceneNode> A5::readLuaSceneFile(const std::string& filename) {
  SceneNode* root = import_lua(filename);
  assert(root != NULL);
  return std::shared_ptr<SceneNode>(root);
}

void A5::initShaderProgram(ShaderProgram& program, const std::string& name) {
  program.generateProgramObject();
  program.attachVertexShader( getAssetFilePath((name + ".vsh").c_str()).c_str() );
  program.attachFragmentShader( getAssetFilePath((name + ".fsh").c_str()).c_str() );
  program.link();
}

//----------------------------------------------------------------------------------------
void A5::uploadVertexDataToVbos (const MeshConsolidator & meshConsolidator) {
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

  // Generate VBO to store all vertex normal data
  {
    glGenBuffers(1, &m_vbo_uvCoords);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_uvCoords);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumUVBytes(),
                 meshConsolidator.getUVDataPtr(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
void A5::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perpsective = glm::perspective(degreesToRadians(60.0f) * float(gameCamera.zoom), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A5::initViewMatrix() {
  m_view = glm::lookAt(
    player.position + gameCamera.getXYRotationMatrix() * glm::vec3(0, 5, -10.0f), // eye
    player.position + glm::vec3(0.0f, 3.0f, 1.0f), // center
    glm::vec3(0, 1, 0) // up
  );
}

//----------------------------------------------------------------------------------------
void A5::initLightSources() {
  // World-space position
  m_light.position = gameCamera.getYRotationMatrix() * vec3(-1.0f, 5.0f, -2.0f);
  m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
glm::mat4 A5::createMinimapPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  return glm::perspective(degreesToRadians(60.0f) * float(minimapCamera.zoom), aspect, 0.1f, 100.0f);
}

//----------------------------------------------------------------------------------------
glm::mat4 A5::createMinimapViewMatrix() {
  return glm::lookAt(
    player.position + minimapCamera.getXYRotationMatrix() * glm::vec3(0, 5, -10.0f), // eye
    player.position +  glm::vec3(0.0f, 3.0f, 1.0f), // center
    glm::vec3(0, 1, 0) // up
  );
}

glm::vec3 createVec3(int i, float v) {
  float data[3] = {0, 0, 0};
  data[i] = v;
  return glm::vec3(data[0], data[1], data[2]);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A5::appLogic() {

  // Place per frame, application logic here ...
  if (!gameState.isPlaying || gameState.lives <= 0) {
    return;
  }

  if (mouse.isRightButtonPressed) {
    float dispX = mouse.x - mouse.prevX;
    float dispY = mouse.y - mouse.prevY;
    const double PI = glm::radians(180.0f);

    if (mouse.isControllingMinimap) {
      minimapCamera.yAngle -= dispX / 200;
      minimapCamera.xAngle = glm::clamp(minimapCamera.xAngle - dispY / 200, -PI/9, PI/9);
    } else {
      gameCamera.yAngle -= dispX / 200;
      gameCamera.xAngle = glm::clamp(gameCamera.xAngle - dispY / 200, -PI/9, PI/9);
      if (
           playerStateManager.getCurrentState() != AIRBORN
        && playerStateManager.getCurrentState() != PREPARING_TO_JUMP
      ) {
        refreshPlayerInputVelocity();
      }
    }
  }

  updateWindSourceGain();
  repositionAndReorientListener();
  repositionPlayerSource();
  repositionWindSource();
  initViewMatrix();
  initPerspectiveMatrix();
  initLightSources();

  // Friction
  glm::vec3 netAppliedForce{0};

  if (playerStateManager.getCurrentState() == AIRBORN) {
    netAppliedForce = world.F_wind;
  } else {
    double N = (-world.F_g.y) - world.F_wind.y;
    double staticForce = N * world.ufs;
    double windXZForce = glm::length(glm::vec3(world.F_wind.x, 0, world.F_wind.z));

    if (staticForce <= windXZForce) {
      double kineticForce = N * world.ufk;
      double netXZForce = std::max(windXZForce - kineticForce, 0.0);

      glm::vec3 windXZDir = Util::normalize(glm::vec3(world.F_wind.x, 0, world.F_wind.z));

      netAppliedForce = netXZForce * windXZDir + glm::vec3(0, world.F_wind.y, 0);
    }
  }

  player.acceleration = (world.F_g + netAppliedForce) / player.mass;

  const float t = 1 / ImGui::GetIO().Framerate;
  player.position = 0.5f * player.acceleration * t * t + player.getVelocity() * t + player.position;
  player.setVelocity(player.acceleration * t + player.getVelocity());

  if (playerStateManager.getCurrentState() != AIRBORN) {
    assert(ground != nullptr);
    if (ground->getTTL() > 0) {
      ground->acceleration = glm::vec3(0);
    } else {
      ground->acceleration = (world.F_g + world.F_wind) / ground->mass;
    }
  }

  for (Platform& block: blocks) {
    const double blink = (1 - block.getTTL()/block.getInitTTL());
    platformTimes.at(block.id) += t * (1 + 9 * blink);

    block.setInputVelocity(platformUpdateVFns.at(block.id)(Clock::getTime()));
    block.position = 0.5 * t * t * block.acceleration + t * block.getVelocity() + block.position;
    block.setVelocity(block.getVelocity() + t * block.acceleration);
  }

  const Hitbox playerHitbox = player.getHitbox();

  for (Platform& block : blocks) {
    Hitbox collision = playerHitbox.getIntersection(block.getHitbox());
    if (!collision.isTrivial()) {
      // Reject collision
      uint argmin = 0;

      for (uint i = 1; i < 3; i++) {
        if (collision.size[i] < collision.size[argmin]) {
          argmin = i;
        }
      }

      glm::vec3 direction {
        player.getVelocity().x >= 0 ? 1 : -1,
        player.getVelocity().y >= 0 ? 1 : -1,
        player.getVelocity().z >= 0 ? 1 : -1
      };

      // Efficiently undo collision
      player.position = player.position - direction * createVec3(argmin, collision.size[argmin]);

      if (playerStateManager.getCurrentState() != PREPARING_TO_JUMP) {
        refreshPlayerInputVelocity();
      }

      const bool isGroundCollision = argmin == 1;

      if (isGroundCollision) {
        ground = &block;
        ground->decreaseTTL(t);

        player.setInertialVelocity(ground->getVelocity());
        goto UpdateCursor;
      }
    }
  }

  ground = nullptr;
  playerStateManager.transition(AIRBORN);

  UpdateCursor:
  mouse.prevX = mouse.x;
  mouse.prevY = mouse.y;
}

void A5::updateWindSourceGain() {
  const double x = glm::length(world.F_wind);
  const double gain = -0.9 * std::exp(-x / 5) + 1;
  alSourcef(windSource, AL_GAIN, gain);
  checkOpenALErrors();
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

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
  // Add more gui elements here here ...

  ImGui::Text("Environment");

  float minFg = -24;
  float maxFg = -0.1;

  float minMass = 0.1;
  float maxMass = 25;

  float minG = minFg / minMass;
  float maxG = maxFg / maxMass;

  if (ImGui::DragFloat("Gravity", &world.F_g.y, 0.1f, minFg, maxFg, "%.3f N")) {
    player.g.y = world.F_g.y / player.mass;
  }

  ImGui::DragFloat("Static friction", &world.ufs, 0.01f, 0.005f, 1.0f);
  ImGui::DragFloat("Kinetic friction", &world.ufk, 0.01f, 0.0005f, 1.0f);
  ImGui::DragFloat3("Wind force", glm::value_ptr(world.F_wind), 0.1, -100.0, 100.0, "%.3f N");

  ImGui::Text("\nPlayer");
  if (ImGui::DragFloat("g", &player.g.y, 0.1f, minG, maxG, "%.3f m/s^2")) {
    player.mass = world.F_g.y / player.g.y;
  }

  if (ImGui::DragFloat("Mass", &player.mass, 0.1f, minMass, maxMass, "%.3f kg")) {
    player.g.y = world.F_g.y / player.mass;
  }

  ImGui::DragFloat("Speed", &player.speed, 0.1f, 0.1f, 10.0f, "%.3f m/s");

  if (!gameState.isPlaying) {
    if (ImGui::Button("Start")) {
      gameState.isPlaying = true;
    }
  } else {
    if (ImGui::Button("Pause")) {
      gameState.isPlaying = false;
    }
  }

  if (ground != nullptr) {
    ImGui::Text("Platform Life: %.3f\n", ground->getTTL());
  }

  // Create Button, and check if it was clicked:
  if( ImGui::Button( "Quit Application" ) ) {
    glfwSetWindowShouldClose(m_window, GL_TRUE);
  }

  ImGui::Text("Cursor: (%.1f, %.1f)", mouse.x, mouse.y);
  ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

class Renderer : public SceneNodeFunctor<void, glm::mat4> {
  ShaderProgram& shader;
  BatchInfoMap& batchInfoMap;
public:
  Renderer(
    ShaderProgram& shader,
    BatchInfoMap& batchInfoMap
  ) : shader(shader), batchInfoMap(batchInfoMap) {}

  void operator()(glm::mat4& M, SceneNode& node) {}
  void operator()(glm::mat4& M, JointNode& node) {}
  void operator()(glm::mat4& M, GeometryNode& node) {
    shader.enable();
    {
      //-- Set Model matrix:
      GLint location = shader.getUniformLocation("Model");
      glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(M));
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


    // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
    BatchInfo batchInfo = batchInfoMap[node.meshId];

    //-- Now render the mesh:
    shader.enable();
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
    CHECK_GL_ERRORS;
    shader.disable();
  }
};

class DepthRenderer : public SceneNodeFunctor<void, glm::mat4> {
  ShaderProgram& shader;
  BatchInfoMap& batchInfoMap;
public:
  DepthRenderer(
    ShaderProgram& shader,
    BatchInfoMap& batchInfoMap
  ) : shader(shader), batchInfoMap(batchInfoMap) {}

  void operator()(glm::mat4& M, SceneNode& node) {}
  void operator()(glm::mat4& M, JointNode& node) {}
  void operator()(glm::mat4& M, GeometryNode& node) {
    shader.enable();
    {
      glUniformMatrix4fv(shader.getUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(M));
      CHECK_GL_ERRORS;
    }
    shader.disable();

    BatchInfo batchInfo = batchInfoMap[node.meshId];

    shader.enable();
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
    CHECK_GL_ERRORS;
    shader.disable();
  }
};

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A5::draw() {
  glEnable(GL_DEPTH_TEST);

  float near_plane = -100000.0f, far_plane = 2000000.0f;
  glm::mat4 LightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
  glm::mat4 LightView = glm::lookAt(
    m_light.position * 10000,
    player.position,
    glm::vec3(0, 1, 0)
  );

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  fillDepthTexture(LightProjection, LightView);

  // Bind offscreen framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

  // Cleanup from fillDepthTexture
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Resize render texture to screen dimensions
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_framebufferWidth, m_framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Resize depth renderbuffer to screen dimensions
  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_framebufferWidth, m_framebufferHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Render minimap
  glm::mat4 minimapProjection = createMinimapPerspectiveMatrix();
  glm::mat4 minimapView = createMinimapViewMatrix();

  renderSkybox(minimapProjection, minimapView);
  renderSceneNormally(minimapProjection, minimapView, LightProjection, LightView);

  // Reset framebuffer to default
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  renderSkybox(m_perpsective, m_view);
  renderSceneNormally(m_perpsective, m_view, LightProjection, LightView);

  renderRenderTexture(minimapViewport.position, minimapViewport.size);

  // Reset the size of the render texture
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Reset the size of the render depth buffer
  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glDisable(GL_DEPTH_TEST);
}

void A5::fillDepthTexture(const glm::mat4& LightProjection, const glm::mat4& LightView) {
  GLint m_viewport[4];
  glGetIntegerv(GL_VIEWPORT, m_viewport);

  const GLint SCR_WIDTH = m_viewport[2];
  const GLint SCR_HEIGHT = m_viewport[3];

  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

  // Clear the screen
  glClearDepth(1.0);
  glClear(GL_DEPTH_BUFFER_BIT);

  m_shader_depth.enable();
  {
    GLuint location = m_shader_depth.getUniformLocation("Projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(LightProjection));
    CHECK_GL_ERRORS;

    location = m_shader_depth.getUniformLocation("View");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(LightView));
    CHECK_GL_ERRORS;
  }
  m_shader_depth.disable();


  glBindVertexArray(m_vao_meshData);
  glEnableVertexAttribArray(m_shader_depth.getAttribLocation("position"));

  // Map Position
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
  glVertexAttribPointer(m_shader_depth.getAttribLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);

  DepthRenderer renderer(m_shader_depth, m_batchInfoMap);
  renderPuppet(renderer);

  // Sort these from increasing
  for (auto it = blocks.rbegin(); it != blocks.rend(); it++) {
    renderPlatform(*it, renderer);
  }

  glDisable(GL_CULL_FACE);

  glDisableVertexAttribArray(m_shader_depth.getAttribLocation("position"));
  glBindVertexArray(0);
  CHECK_GL_ERRORS;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void A5::renderRenderTexture(const glm::vec2& position, const glm::vec2& size) {
  GLint m_viewport[4];
  glGetIntegerv(GL_VIEWPORT, m_viewport);

  const GLint SCR_WIDTH = m_viewport[2];
  const GLint SCR_HEIGHT = m_viewport[3];

  // Render on the whole framebuffer, complete from the lower left corner to the upper right
  glViewport(position.x, position.y, size.x, size.y);

  // Clear the screen
  glClear(GL_DEPTH_BUFFER_BIT);

  m_shader_quad.enable();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  glUniform1i(m_shader_quad.getUniformLocation("renderedTexture"), 0);

  // glActiveTexture(GL_TEXTURE1);
  // glBindTexture(GL_TEXTURE_2D, depthTexture);
  // glUniform1i(m_shader_quad.getUniformLocation("depthTexture"), 1);

  glBindVertexArray(VertexArrayID);
  // glUniform1f(m_shader_quad.getUniformLocation("time"), (float)(glfwGetTime()*10.0f) );

  // Enable vertex shader input slot
  glEnableVertexAttribArray(m_shader_quad.getAttribLocation("vertexPosition_modelspace"));
  CHECK_GL_ERRORS;

  // Map vbo data to shader input
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glVertexAttribPointer(
    m_shader_quad.getAttribLocation("vertexPosition_modelspace"),                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );
  CHECK_GL_ERRORS;

  // Draw the triangles !
  glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

  glDisableVertexAttribArray(m_shader_quad.getAttribLocation("vertexPosition_modelspace"));
  // glDisableVertexAttribArray(m_shader_quad.getAttribLocation("vertexUV"));
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // glActiveTexture(GL_TEXTURE1);
  // glBindTexture(GL_TEXTURE_2D, 0);

  m_shader_quad.disable();

  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void A5::renderSceneNormally(
  const glm::mat4& Projection,
  const glm::mat4& View,
  const glm::mat4& LightProjection,
  const glm::mat4& LightView
) {
  glClearDepth(1.0);
  glClear(GL_DEPTH_BUFFER_BIT);

  m_shader.enable();
  {
    //-- Set Perpsective matrix uniform for the scene:
    GLint location = m_shader.getUniformLocation("Projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(Projection));
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("View");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(View));
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("LightProjection");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(LightProjection));
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("LightView");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(LightView));
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

    // Ensure transparency is 1
    glUniform1f(m_shader.getUniformLocation("alpha"), 1.0f);

    // Bind depth texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glUniform1i(m_shader.getUniformLocation("depthTexture"), 0);
    CHECK_GL_ERRORS;

    // Bind tile texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tileTexture);

    glUniform1i(m_shader.getUniformLocation("picture"), 1);
    CHECK_GL_ERRORS;

  }
  m_shader.disable();

  // Enable Position and Normal
  glBindVertexArray(m_vao_meshData);
  glEnableVertexAttribArray(m_shader.getAttribLocation("position"));
  glEnableVertexAttribArray(m_shader.getAttribLocation("normal"));
  glEnableVertexAttribArray(m_shader.getAttribLocation("uv"));

  // Map Position
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
  glVertexAttribPointer(m_shader.getAttribLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Map Normals
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
  glVertexAttribPointer(m_shader.getAttribLocation("normal"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Map uv
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_uvCoords);
  glVertexAttribPointer(m_shader.getAttribLocation("uv"), 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  Renderer renderer{m_shader, m_batchInfoMap};

  renderPuppet(renderer);

  // Sort these from increasing
  for (auto it = blocks.rbegin(); it != blocks.rend(); it++) {
    Platform& block = *it;
    m_shader.enable();
      const double period = 4;
      const double PI = glm::radians(180.0f);
      const double t = platformTimes.at(block.id);
      const float alpha = 0.25 * std::sin(2 * PI / period * t) + 0.5;
      glUniform1f(m_shader.getUniformLocation("alpha"), alpha);
    m_shader.disable();

    renderPlatform(block, renderer);
  }

  glDisable(GL_CULL_FACE);

  glDisableVertexAttribArray(m_shader.getAttribLocation("position"));
  glDisableVertexAttribArray(m_shader.getAttribLocation("normal"));
  glDisableVertexAttribArray(m_shader.getAttribLocation("uv"));
  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

void A5::renderSkybox(const glm::mat4& Projection, const glm::mat4& View) {
  // Render skybox
  glClearDepth(1.0);
  glClear(GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_FALSE);

  glm::mat4 view = glm::mat4(glm::mat3(View));
  glm::mat4 projection = Projection;

  glDepthFunc(GL_LEQUAL);

  m_shader_skybox.enable();
    glUniformMatrix4fv(m_shader_skybox.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    CHECK_GL_ERRORS;

    glUniformMatrix4fv(m_shader_skybox.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    CHECK_GL_ERRORS;

    glUniform1i(m_shader_skybox.getUniformLocation("skybox"), 0);
  m_shader_skybox.disable();

  GLuint positionAttrib = m_shader_skybox.getAttribLocation("position");

  // Enable
  glBindVertexArray(skyboxVAO);
  glEnableVertexAttribArray(positionAttrib);

  // Map VBO data to attribs
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
  CHECK_GL_ERRORS;

  m_shader_skybox.enable();
    glDrawArrays(GL_TRIANGLES, 0, 36);
  m_shader_skybox.disable();

  glDisableVertexAttribArray(positionAttrib);
  glBindVertexArray(0);

  glDepthFunc(GL_LESS);

  glDepthMask(GL_TRUE);
}

void A5::renderPuppet(SceneNodeFunctor<void, glm::mat4>& renderer) {
  // Draw player
  glm::mat4 rotation = glm::rotate(float(player.getDirection()), glm::vec3(0, 1, 0));
  glm::mat4 translation = glm::translate(glm::vec3(player.position));
  glm::mat4 modelView = translation * rotation;

  std::tuple<Keyframe, Frame> animationFrames = currentAnimation->get(Clock::getTime() - animationStartTime);
  Keyframe keyframe = std::get<0>(animationFrames);
  Frame frame = std::get<1>(animationFrames);

  if (oldKeyframeId != keyframe.id && keyframe.sound != "") {
    playSoundWithSource(playerSource, keyframe.sound);
  }

  oldKeyframeId = keyframe.id;

  AnimationTransformationReducer transformReducer{frame};

  TransformationCollector dynamicRenderer{transformReducer, renderer, modelView};
  puppetSceneNode->accept(dynamicRenderer);
}

void A5::playSoundWithSource(ALuint source, std::string filename) {
  if (soundBuffers.find(filename) == soundBuffers.end()) {
    ALuint buffer = alutCreateBufferFromFile(getAssetFilePath(filename.c_str()).c_str());
    soundBuffers[filename] = buffer;
  }

  ALint source_state;
  alGetSourcei(source, AL_SOURCE_STATE, &source_state);

  if (source_state == AL_PLAYING) {
    return;
  }

  alSourcei(source, AL_BUFFER, soundBuffers[filename]);
  alSourcePlay(source);
  checkOpenALErrors();
}

void A5::renderPlatform(Platform& block, SceneNodeFunctor<void, glm::mat4>& renderer) {
  static StaticTransformationReducer transformReducer;

  glm::mat4 scale = glm::scale(block.size);
  glm::mat4 translate = glm::translate(block.position);
  glm::mat4 modelView = translate * scale;

  TransformationCollector staticRenderer{transformReducer, renderer, modelView};
  blockSceneNode->accept(staticRenderer);
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
      case GLFW_MOUSE_BUTTON_LEFT:
      case GLFW_MOUSE_BUTTON_RIGHT:
      case GLFW_MOUSE_BUTTON_MIDDLE: {
        if (isMouseOnMinimap()) {
          mouse.isControllingMinimap = true;
        }
      }
    }

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
    mouse.isControllingMinimap = false;
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

bool A5::isMouseOnMinimap() const {
  const double mouseX = mouse.x;
  const double mouseY = mouse.y;
  return minimapViewport.position.x <= mouseX && mouseX <= (minimapViewport.size + minimapViewport.position).x
    && minimapViewport.position.y <= mouseY && mouseY <= (minimapViewport.size + minimapViewport.position).y;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A5::mouseScrollEvent (
  double xOffset,
  double yOffset
) {
  if (isMouseOnMinimap()) {
    minimapCamera.zoom = glm::clamp(minimapCamera.zoom + yOffset / 50, 0.75, 1.5);
  } else {
    gameCamera.zoom = glm::clamp(gameCamera.zoom + yOffset / 50, 0.75, 1.5);
  }
  return true;
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
  refreshMinimapViewportDimensions();

  return eventHandled;
}

void A5::refreshMinimapViewportDimensions() {
  const double width = m_framebufferWidth / 4;
  const double height = m_framebufferHeight / 4;
  minimapViewport.size = glm::vec2{width, height};
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
  switch (action) {
    case GLFW_PRESS: {
      keysPressed.insert(key);

      switch (key) {
        case GLFW_KEY_M: {
          show_gui = !show_gui;
          break;
        }
        case GLFW_KEY_SPACE: {
          PlayerState currentState = playerStateManager.getCurrentState();
          if (currentState != AIRBORN) {
            playerStateManager.transition(PREPARING_TO_JUMP);
            player.setInputVelocity(glm::vec3(0));
          }
          break;
        }
      }
      break;
    }
    case GLFW_RELEASE: {
      keysPressed.erase(key);

      switch (key) {
        case GLFW_KEY_SPACE: {
          if (playerStateManager.getCurrentState() == PREPARING_TO_JUMP) {
            playerStateManager.transition(AIRBORN);
            player.setInputVelocity(calculatePlayerInputVelocity());
            player.setVelocity(glm::vec3(player.getVelocity().x, 6, player.getVelocity().z));
          }
          break;
        }
      }
      break;
    }
  }

  if (action == GLFW_PRESS || action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W:
      case GLFW_KEY_A:
      case GLFW_KEY_S:
      case GLFW_KEY_D:
      case GLFW_KEY_UP:
      case GLFW_KEY_DOWN:
      case GLFW_KEY_LEFT:
      case GLFW_KEY_RIGHT: {
        if (
             playerStateManager.getCurrentState() != AIRBORN
          && playerStateManager.getCurrentState() != PREPARING_TO_JUMP
        ) {
          refreshPlayerInputVelocity();
        }
        return true;
      }
    }
  }

  return true;
}

bool A5::isKeyPressed(int key) {
  return keysPressed.find(key) != keysPressed.end();
}

// Reset player's XZ velocity (based on key inputs) and update animations
void A5::refreshPlayerInputVelocity() {
  const double epsilon = 0.0001;
  glm::vec3 inputV = calculatePlayerInputVelocity();
  player.setInputVelocity(inputV);

  if (glm::length(glm::vec3(inputV.x, 0, inputV.z)) >= epsilon) {
    playerStateManager.transition(WALKING);
  } else {
    playerStateManager.transition(STANDING);
  }
}

glm::vec3 A5::calculatePlayerInputVelocity() {
  double vx = 0;
  double vz = 0;

  bool isMovingLeft = isKeyPressed(GLFW_KEY_LEFT) || isKeyPressed(GLFW_KEY_A);
  bool isMovingRight = isKeyPressed(GLFW_KEY_RIGHT) || isKeyPressed(GLFW_KEY_D);
  bool isMovingForward = isKeyPressed(GLFW_KEY_UP) || isKeyPressed(GLFW_KEY_W);
  bool isMovingBackward = isKeyPressed(GLFW_KEY_DOWN) || isKeyPressed(GLFW_KEY_S);
  bool isMoving = isMovingLeft || isMovingRight || isMovingForward || isMovingBackward;

  if (!isMoving) {
    return glm::vec3(0);
  }

  if (isMovingLeft) {
    vx += 1;
  }

  if (isMovingRight) {
    vx -= 1;
  }

  if (isMovingForward) {
    vz += 1;
  }

  if (isMovingBackward) {
    vz -= 1;
  }

  glm::vec3 velocity = Util::normalize(glm::vec3(vx, 0, vz)) * player.speed;
  return gameCamera.getYRotationMatrix() * velocity;
}
