#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shaders/Shader.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera/Camera.hpp"
#include "Model/Model.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "FrameBuffer/FrameBuffer.hpp"
#include "Buildplate/Buildplate.hpp"
#include <clipper2/clipper.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// initial window dimensions
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(18.0f, 15.0f, 18.0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool rescale = false;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZupaSlica", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader BuildplateShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/BuildplateShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/BuildplateShader.fs");
    Shader objectShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/ObjectShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/ObjectShader.fs");


    //Setup environment
    glm::mat4 model = glm::mat4(1.0f);
    Mesh buildPlate = Mesh(Buildplate::GetVertices(), Buildplate::GetIndices(), std::vector<Texture>());

    // load models
    // -----------
    Model ourModel("/home/xandervaes/Code/ZupaSlica/school_stuff/COFAB-models-set1/COFAB-models-set1/support-test.stl");

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    //imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    FrameBuffer sceneBuffer(SCR_WIDTH, SCR_HEIGHT);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

        sceneBuffer.Bind();

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.80f, 1.0f, 0.80f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!glClear(GL_COLOR_BUFFER_BIT);

        //projection and view matrix
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        

        //buildplate
        BuildplateShader.use();
        //light properties
        BuildplateShader.setVec3("objectColor", 0.8f, 0.8f, 0.7f);
        BuildplateShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        BuildplateShader.setVec3("lightPos", camera.Position);
        BuildplateShader.setVec3("viewPos", camera.Position);

        BuildplateShader.setMat4("view", view);
        BuildplateShader.setMat4("projection", projection);

        model = glm::mat4(1.0f);
        //scale to buildplate dimensions
        //dimension for ender3 buildplate: 22x22
        model = glm::scale(model, glm::vec3(22.0f, 1.0f, 22.0f));
        BuildplateShader.setMat4("model", model);

        buildPlate.Draw(BuildplateShader);



        // be sure to activate shader when setting uniforms/drawing objects
        objectShader.use();

        // light properties
        objectShader.setVec3("viewPos", camera.Position);
        objectShader.setVec3("lightPos", camera.Position);
        objectShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        objectShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);

        // view/projection transformations
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        objectShader.setMat4("model", model);

        ourModel.Draw(objectShader);
        sceneBuffer.Unbind();
        

        // start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set the window to fill the viewport
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        // Set up the main ImGui window (often covers the full application window)
        ImGui::Begin("ZupaSlica", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoBringToFrontOnFocus |
                                             ImGuiWindowFlags_NoNavFocus);
        
        //add menu to docking window to disable the docking
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Docking"))
            {
                if (ImGui::MenuItem("Disable Docking"))
                {
                    io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        

        // Dockspace ID - unique identifier for the dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

        // Set up the dockspace within this main ImGui window
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), 0);

        

        // render your GUI
        ImGui::Begin("Inputs");
        //get aspect ratio
        float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        ImGui::Text("Aspect ratio: %f", aspect);
        ImGui::Text("Camera position: %f, %f, %f", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::Text("xoffset: %f, yoffset: %f", lastX, lastY);

        //distance of camera to origin
        float distance = glm::length(camera.Position);
        ImGui::Text("Distance to origin: %f", distance);
        ImGui::End();

        
        ImGui::Begin("Scene");
        {
            ImGui::BeginChild("GameRender");

            SCR_WIDTH = ImGui::GetContentRegionAvail().x;
            SCR_HEIGHT = ImGui::GetContentRegionAvail().y;

            ImGui::Image(
                (ImTextureID)sceneBuffer.getFrameTexture(), 
                ImGui::GetContentRegionAvail(), 
                ImVec2(0, 1), 
                ImVec2(1, 0)
            );
        }
        ImGui::EndChild();
        ImGui::End();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // imgui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        firstMouse = true;
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, -yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}