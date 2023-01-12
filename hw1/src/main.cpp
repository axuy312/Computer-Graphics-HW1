#include <algorithm>
#include <memory>
#include <vector>

#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION
#include <glm/glm.hpp>

#include "camera.h"
#include "opengl_context.h"
#include "utils.h"

#define ANGEL_TO_RADIAN(x) (float)((x)*M_PI / 180.0f) 
#define RADIAN_TO_ANGEL(x) (float)((x)*180.0f / M_PI) 

#define CIRCLE_SEGMENT 64

/* Key definition 
#define GLFW_KEY_U 85
#define GLFW_KEY_J 74
#define GLFW_KEY_I 73
#define GLFW_KEY_K 75
#define GLFW_KEY_O 79
#define GLFW_KEY_L 76
#define GLFW_KEY_SPACE 32 */

/* Components size definition */
#define ARM_LEN 1.0f
#define ARM_RADIUS 0.05f
#define ARM_DIAMETER (ARM_RADIUS * 2)
#define JOINT_RADIUS 0.05f
#define JOINT_DIAMETER (JOINT_RADIUS * 2)
#define JOINT_WIDTH 0.1f
#define BASE_RADIUS 0.5f
#define BASEE_DIAMETER (BASE_RADIUS * 2)
#define BASE_HEIGHT 0.1f
#define TARGET_RADIUS 0.05f
#define TARGET_DIAMETER (TARGET_RADIUS * 2)
#define TARGET_HEIGHT 0.1f
#define ROTATE_SPEED 0.3f 
#define CATCH_POSITION_OFFSET 0.1f
#define TOLERANCE 0.1f

#define RED 0.905f, 0.298f, 0.235f
#define BLUE 0.203f, 0.596f, 0.858f
#define GREEN 0.18f, 0.8f, 0.443f

float joint0_degree = 0;
float joint1_degree = 0;
float joint2_degree = 0;
glm::vec3 target_pos(0.0f, 0.05f, 1.0f);
bool space_down = false;
bool g_down = false;

void resizeCallback(GLFWwindow* window, int width, int height) {
  OpenGLContext::framebufferResizeCallback(window, width, height);
  auto ptr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
  if (ptr) {
    ptr->updateProjectionMatrix(OpenGLContext::getAspectRatio());
  }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
  // There are three actions: press, release, hold(repeat)
  if (action == GLFW_REPEAT) return;
  // Press ESC to close the window.
  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    return;
  }
  /* TODO#4-1: Detect key-events, perform rotation or catch target object
   *       1. Use switch/case to find the key you want.
   *       2. Define and modify some global variable to trigger update in rendering loop
   * Hint:
   *       glfw3's key list (https://www.glfw.org/docs/3.3/group__keys.html)
   *       glfw3's action codes (https://www.glfw.org/docs/3.3/group__input.html#gada11d965c4da13090ad336e030e4d11f)
   * Note:
   *       You should finish your robotic arm first.
   *       Otherwise you will spend a lot of time debugging this with a black screen.
   */
  switch (key) {
    case GLFW_KEY_U:
      joint0_degree += 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_J:
      joint0_degree -= 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_I:
      joint1_degree -= 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_K:
      joint1_degree += 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_O:
      joint2_degree -= 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_L:
      joint2_degree += 10 * ROTATE_SPEED;
      break;
    case GLFW_KEY_G:
      if (action == GLFW_RELEASE) {
        if (g_down) {
          printf("down disable\n");
        } else {
          printf("down enable\n");
        }
        g_down = !g_down;
      }
      break;
    case GLFW_KEY_SPACE:
      switch (action) {
        case GLFW_PRESS:
          space_down = true;
          break;
        case GLFW_RELEASE:
          space_down = false;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void initOpenGL() {
  // Initialize OpenGL context, details are wrapped in class.
#ifdef __APPLE__
  // MacOS need explicit request legacy support
  OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
#else
  OpenGLContext::createContext(21, GLFW_OPENGL_ANY_PROFILE);
//  OpenGLContext::createContext(43, GLFW_OPENGL_COMPAT_PROFILE);
#endif
  GLFWwindow* window = OpenGLContext::getWindow();
  /* TODO#0: Change window title to "HW1 - `your student id`"
   *         Ex. HW1 - 311550000 
   */
  glfwSetWindowTitle(window, "HW1 - 311551144");
  glfwSetKeyCallback(window, keyCallback);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#ifndef NDEBUG
  OpenGLContext::printSystemInfo();
  // This is useful if you want to debug your OpenGL API calls.
  OpenGLContext::enableDebugCallback();
#endif
}

void drawUnitCylinder() {
  /* TODO#2-1: Render a unit cylinder
   * Hint:
   *       glBegin/glEnd (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
   *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
   *       glVertex3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml)
   *       glNormal (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glNormal.xml)
   *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
   * Note:
   *       You can refer to ppt "Draw Cylinder" page and `CIRCLE_SEGMENT`
   *       You should set normal for lighting
   */

  float innerAngle = 360.f;
  float innerRadian;

  glPushMatrix();
  glTranslatef(target_pos.x, target_pos.y, target_pos.z);
  glScalef(TARGET_DIAMETER/2, TARGET_HEIGHT, TARGET_DIAMETER/2);
  glBegin(GL_TRIANGLE_STRIP);
  glColor3f(RED);
  for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
    innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
    float z = cos(innerRadian);
    float x = sin(innerRadian);
    innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
    float nz = cos(innerRadian);
    float nx = sin(innerRadian);
    glNormal3f(nx, 0.0f, nz);
    glVertex3f(x, 0.5f, z);
    glVertex3f(x, -0.5f, z);
  }
  glEnd();
  glPopMatrix();

  glPushMatrix();
  glTranslatef(target_pos.x, target_pos.y, target_pos.z);
  glScalef(TARGET_DIAMETER / 2, TARGET_HEIGHT, TARGET_DIAMETER / 2);
  glBegin(GL_POLYGON);
  glNormal3f(0.0f, 1.0f, 0.0f);
  for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
    innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
    float z = cos(innerRadian);
    float x = sin(innerRadian);
    glVertex3f(x, 0.5f, z);
  }
  glEnd();
  glPopMatrix();

  glPushMatrix();
  glTranslatef(target_pos.x, target_pos.y , target_pos.z);
  glScalef(TARGET_DIAMETER / 2, TARGET_HEIGHT, TARGET_DIAMETER / 2);
  glBegin(GL_POLYGON);
  glNormal3f(0.0f, -1.0f, 0.0f);
  for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
    innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
    float z = cos(innerRadian);
    float x = sin(innerRadian);
    glVertex3f(x, -0.5f, z);
  }
  glEnd();
  glPopMatrix();
}

void light() {
  GLfloat light_specular[] = {0.6, 0.6, 0.6, 1};
  GLfloat light_diffuse[] = {0.6, 0.6, 0.6, 1};
  GLfloat light_ambient[] = {0.4, 0.4, 0.4, 1};
  GLfloat light_position[] = {50.0, 75.0, 80.0, 1.0};
  // z buffer enable
  glEnable(GL_DEPTH_TEST);
  // enable lighting
  glEnable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_NORMALIZE);
  // set light property
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
}

int main() {
  initOpenGL();
  GLFWwindow* window = OpenGLContext::getWindow();

  // Init Camera helper
  Camera camera(glm::vec3(0, 2, 5));
  camera.initialize(OpenGLContext::getAspectRatio());
  // Store camera as glfw global variable for callbasks use
  glfwSetWindowUserPointer(window, &camera);

  // Main rendering loop
  while (!glfwWindowShouldClose(window)) {
    // Polling events.
    glfwPollEvents();
    // Update camera position and view
    camera.move(window);
    // GL_XXX_BIT can simply "OR" together to use.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /// TO DO Enable DepthTest
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera.getProjectionMatrix());
    // ModelView Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera.getViewMatrix());

#ifndef DISABLE_LIGHT   
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0f);
    light();
#endif

    /* TODO#4-2: Update joint degrees
     *       1. Finish keyCallback to detect key events
     *       2. Update jointx_degree if the correspond key is pressed
     * Note:
     *       You can use `ROTATE_SPEED` as the speed constant. 
     *       If the rotate speed is too slow or too fast, please change `ROTATE_SPEED` value
     */

    /* TODO#5: Catch the target object with robotic arm
     *       1. Calculate coordinate of the robotic arm endpoint
     *       2. Test if arm endpoint and the target object are close enough
     *       3. Update coordinate fo the target object to the arm endpoint
     *          if the space key is pressed
     * Hint: 
     *       GLM fransform API (https://glm.g-truc.net/0.9.4/api/a00206.html)
     * Note: 
     *       You might use `ANGEL_TO_RADIAN`
     *       and refer to `CATCH_POSITION_OFFSET` and `TOLERANCE`
     */
    glm::vec4 arm_endpoint(0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 trasformMatrix_arm_endpoint(1.0f);
    
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, BASE_HEIGHT, 0.0f));
    trasformMatrix_arm_endpoint = glm::rotate(trasformMatrix_arm_endpoint, ANGEL_TO_RADIAN(joint0_degree), glm::vec3(0.0f, 1.0f, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, ARM_LEN, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, JOINT_RADIUS, 0.0f));
    trasformMatrix_arm_endpoint = glm::rotate(trasformMatrix_arm_endpoint, ANGEL_TO_RADIAN(joint1_degree), glm::vec3(1.0f, 0.0f, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, JOINT_RADIUS, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, ARM_LEN, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, JOINT_RADIUS, 0.0f));
    trasformMatrix_arm_endpoint = glm::rotate(trasformMatrix_arm_endpoint, ANGEL_TO_RADIAN(joint2_degree), glm::vec3(1.0f, 0.0f, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, JOINT_RADIUS, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, ARM_LEN, 0.0f));
    trasformMatrix_arm_endpoint = glm::translate(trasformMatrix_arm_endpoint, glm::vec3(0.0f, CATCH_POSITION_OFFSET, 0.0f));
    
    arm_endpoint = trasformMatrix_arm_endpoint * arm_endpoint;
    float distance_target = powf(arm_endpoint.x - target_pos.x, 2.0f);
    distance_target += powf(arm_endpoint.y - target_pos.y, 2.0f);
    distance_target += powf(arm_endpoint.z - target_pos.z, 2.0f);
    distance_target = sqrtf(distance_target);
    if (space_down && distance_target <= TOLERANCE) 
    {
      target_pos = glm::vec3(arm_endpoint.x, arm_endpoint.y, arm_endpoint.z);
    } else if (g_down && target_pos.y > 0) {
      target_pos.y = (target_pos.y - 0.005f < TARGET_HEIGHT / 2) ? TARGET_HEIGHT / 2 : target_pos.y - 0.005f;
    }


    // Render a white board
    glPushMatrix();
    glScalef(3, 1, 3);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 0.0f, -1.0f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glEnd();
    glPopMatrix();
    drawUnitCylinder();
    /* TODO#2: Render a cylinder at target_pos
     *       1. Translate to target_pos
     *       2. Setup vertex color
     *       3. Setup cylinder scale
     *       4. Call drawUnitCylinder
     * Hint: 
     *       glTranslatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml)
     *       glColor3f (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glColor.xml)
     *       glScalef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glScale.xml)
     * Note:
     *       The coordinates of the cylinder are `target_pos`
     *       The cylinder's size can refer to `TARGET_RADIUS`, `TARGET_DIAMETER` and `TARGET_DIAMETER`
     *       The cylinder's color can refer to `RED`
     */

    /* TODO#3: Render the robotic arm
     *       1. Render the base
     *       2. Translate to top of the base
     *       3. Render an arm
     *       4. Translate to top of the arm
     *       5. Render the joint
     *       6. Translate and rotate to top of the join
     *       7. Repeat step 3-6
     * Hint:
     *       glPushMatrix/glPopMatrix (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml)
     *       glRotatef (https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml)
     * Note:
     *       The size of every component can refer to `Components size definition` section
     *       Rotate degree for joints are `joint0_degree`, `joint1_degree` and `joint2_degree`
     *       You may implement drawBase, drawArm and drawJoin first
     */

    float innerAngle = 360.f;
    float innerRadian;

    //BASE==============================================================
    glPushMatrix();
    glScalef(BASEE_DIAMETER / 2, BASE_HEIGHT, BASEE_DIAMETER / 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(GREEN);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
        innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
        float z = cos(innerRadian);
        float x = sin(innerRadian);
        innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
        float nz = cos(innerRadian);
        float nx = sin(innerRadian);
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(x, 1.0f, z);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glScalef(BASEE_DIAMETER / 2, BASE_HEIGHT, BASEE_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
        innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
        float z = cos(innerRadian);
        float x = sin(innerRadian);
        glVertex3f(x, 1.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glScalef(BASEE_DIAMETER / 2, BASE_HEIGHT, BASEE_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
        innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
        float z = cos(innerRadian);
        float x = sin(innerRadian);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    // Arm1==============================================================
    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER/ 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(BLUE);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
      float nz = cos(innerRadian);
      float nx = sin(innerRadian);
      glNormal3f(nx, 0.0f, nz);
      glVertex3f(x, 1.0f, z);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      glVertex3f(x, 1.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    // Joint1==============================================================
    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint0_degree, 0, 1, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(GREEN);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float y = sin(innerRadian);
      innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
      float nz = cos(innerRadian);
      float ny = sin(innerRadian);
      glNormal3f(0.0f, ny, nz);
      glVertex3f(-0.5f, y, z);
      glVertex3f(0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint0_degree, 0, 1, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(1.0f, 0.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float y = cos(innerRadian);
      float z = sin(innerRadian);
      glVertex3f(0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint0_degree, 0, 1, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float y = sin(innerRadian);
      glVertex3f(-0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    // Arm2==============================================================
    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(BLUE);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
      float nz = cos(innerRadian);
      float nx = sin(innerRadian);
      glNormal3f(nx, 0.0f, nz);
      glVertex3f(x, 1.0f, z);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      glVertex3f(x, 1.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float x = cos(innerRadian);
      float z = sin(innerRadian);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    // Joint2==============================================================
    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(GREEN);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float y = sin(innerRadian);
      innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
      float nz = cos(innerRadian);
      float ny = sin(innerRadian);
      glNormal3f(0.0f, ny, nz);
      glVertex3f(-0.5f, y, z);
      glVertex3f(0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(1.0f, 0.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float y = cos(innerRadian);
      float z = sin(innerRadian);
      glVertex3f(0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glScalef(JOINT_WIDTH, JOINT_DIAMETER / 2, JOINT_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float y = sin(innerRadian);
      glVertex3f(-0.5f, y, z);
    }
    glEnd();
    glPopMatrix();

    // Arm3==============================================================
    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(BLUE);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      innerRadian = ANGEL_TO_RADIAN(innerAngle * (i - 0.5) / CIRCLE_SEGMENT);
      float nz = cos(innerRadian);
      float nx = sin(innerRadian);
      glNormal3f(nx, 0.0f, nz);
      glVertex3f(x, 1.0f, z);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float z = cos(innerRadian);
      float x = sin(innerRadian);
      glVertex3f(x, 1.0f, z);
    }
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glRotatef(joint0_degree, 0, 1, 0);
    glTranslatef(0.0f, BASE_HEIGHT + ARM_LEN + JOINT_RADIUS, 0.0f);
    glRotatef(joint1_degree, 1, 0, 0);
    glTranslatef(0.0f, ARM_LEN + JOINT_DIAMETER, 0.0f);
    glRotatef(joint2_degree, 1, 0, 0);
    glTranslatef(0.0f, JOINT_RADIUS, 0.0f);
    glScalef(ARM_DIAMETER / 2, ARM_LEN, ARM_DIAMETER / 2);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (float i = 0; i <= CIRCLE_SEGMENT; i += 1.0f) {
      innerRadian = ANGEL_TO_RADIAN(innerAngle * i / CIRCLE_SEGMENT);
      float x = cos(innerRadian);
      float z = sin(innerRadian);
      glVertex3f(x, 0.0f, z);
    }
    glEnd();
    glPopMatrix();

#ifdef __APPLE__
    // Some platform need explicit glFlush
    glFlush();
#endif
    glfwSwapBuffers(window);
  }
  return 0;
}
