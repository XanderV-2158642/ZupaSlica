#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader/Shader.hpp"
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
#include "DrawSTL/DrawSTL.hpp"
#include "SlicingPlane/SlicingPlane.hpp"
#include <clipper2/clipper.h>
#include "SlicerSettings/SlicerSettings.hpp"
#include "Slicing/TriangleIntersections/CalculateIntersections.hpp"
#include "Intersection/Intersection.hpp"
#include "Slicing/Gcode/GcodeWriter.hpp"
#include "Slicing/Slicing.hpp"
#include "Slicing/Infill/CreateInfill.hpp"
#include <time.h>
#include "PathOptimization/PathOptimization.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// initial window dimensions
unsigned int SCR_WIDTH = 1980;
unsigned int SCR_HEIGHT = 1080;

unsigned int SCENE_WIDTH = 1280;
unsigned int SCENE_HEIGHT = 720;

unsigned int INTERSECTION_WIDTH = 1280;
unsigned int INTERSECTION_HEIGHT = 720;

bool resizeBuffer = false;

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader program
    // ------------------------------------
    Shader BuildplateShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/BuildplateShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/BuildplateShader.fs");
    Shader slicingPlaneShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/SlicingPlaneShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/SlicingPlaneShader.fs");
    Shader objectShader("/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/ObjectShader.vs", "/home/xandervaes/Code/ZupaSlica/src/ShaderFiles/ObjectShader.fs");


    //Setup environment
    glm::mat4 model = glm::mat4(1.0f);
    Mesh buildPlate = Mesh(Buildplate::GetVertices(), Buildplate::GetIndices(), std::vector<Texture>());

    SlicingPlane slicingPlane = SlicingPlane(slicingPlaneShader);

    // load models
    // -----------
    Model ourModel("/home/xandervaes/Code/ZupaSlica/school_stuff/COFAB-models-set1/COFAB-models-set1/hole-test(easy).stl");

    // move vertices up by lowest point
    float lowest = DrawSTL::GetLowestPoint(ourModel);
    glm::vec3 center = DrawSTL::GetXYCenterPoint(ourModel);
    for (int i = 0; i < ourModel.meshes[0].vertices.size(); i++)
    {
        ourModel.meshes[0].vertices[i].Position.z -= lowest;
        ourModel.meshes[0].vertices[i].Position.x -= center.x;
        ourModel.meshes[0].vertices[i].Position.y -= center.y;
    }

    glm::vec3 translation = glm::vec3(center.x, center.y, lowest);

    Intersection intersection = Intersection();

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
    FrameBuffer intersectionBuffer(SCR_WIDTH, SCR_HEIGHT);

    SlicerSettings slicerSettings = SlicerSettings();
    GCodeWriter gcodeWriter = GCodeWriter(slicerSettings);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        if (resizeBuffer)
        {
            sceneBuffer.RescaleFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
            intersectionBuffer.RescaleFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
            resizeBuffer = false;
        }

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
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!glClear(GL_COLOR_BUFFER_BIT);

        //projection and view matrix
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCENE_WIDTH/ (float)SCENE_HEIGHT, 0.1f, 100.0f);
        

        //buildplate
        Buildplate::Draw(BuildplateShader, buildPlate, view, projection, camera);

        //object
        DrawSTL::Draw(objectShader, ourModel, view, projection, camera, translation);

        //slice plane (draw last because it is transparent)
        float height = intersection.GetSlicingPlaneHeight(slicerSettings.GetLayerHeight());
        slicingPlane.SetPosition(glm::vec3(0.0f, height/10, 0.0f));
        slicingPlane.Draw(view, projection, camera);

        sceneBuffer.Unbind();

        // draw 2d intersection
        intersectionBuffer.Bind();
        glDisable(GL_DEPTH_TEST);


        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!glClear(GL_COLOR_BUFFER_BIT);

        // draw the intersection
        intersection.DrawIntersection((float) INTERSECTION_WIDTH / (float) INTERSECTION_HEIGHT, slicerSettings);

        glEnable(GL_DEPTH_TEST);
        intersectionBuffer.Unbind();
        

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
        
        
        // Dockspace ID - unique identifier for the dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

        // Set up the dockspace within this main ImGui window
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), 0);

        
        // render your GUI
        ImGui::Begin("Inputs");
        {
            float layerHeight = slicerSettings.GetLayerHeight();
            float nozzleDiameter = slicerSettings.GetNozzleDiameter();
            int shells = slicerSettings.GetShells();
            int roofs = slicerSettings.GetRoofs();
            int floors = slicerSettings.GetFloors();
            float infillPercentage = slicerSettings.GetInfill();
            //layer height
            if(ImGui::InputFloat("Layer height", &layerHeight, 0.02f, 0.1f, "%.2f mm"))
                slicerSettings.SetLayerHeight(layerHeight);


            //nozzle diameter
            if(ImGui::InputFloat("Nozzle diameter", &nozzleDiameter, 0.1f, 0.1f, "%.1f mm"))
                slicerSettings.SetNozzleDiameter(nozzleDiameter);

            // number of shells 
            if(ImGui::InputInt("Shells", &shells, 1, 1))
                slicerSettings.SetShells(shells);


            if (ImGui::InputInt("Number of roofs", &roofs, 1, 1))
                slicerSettings.SetRoofs(roofs);
            
            if (ImGui::InputInt("Number of floors", &floors, 1, 1))
                slicerSettings.SetFloors(floors);

            if(ImGui::InputFloat("Infill percentage", &infillPercentage, 0.5f, 1.0f, "%.1f mm"))
                slicerSettings.SetInfill(infillPercentage);
            

            //slicing plane height
            int shownPlane = intersection.GetHeight();
            if(ImGui::SliderInt("Slicing plane height", &shownPlane, 0, intersection.GetMaxHeight()-1))
                intersection.SetHeight(shownPlane);
            
            

            // button to calculate intersection
            if (ImGui::Button("Slice")) {
                time_t start, end;
                time(&start);
                vector<Slice> sliceMap = Slicing::SliceModel(ourModel.meshes[0].vertices, slicerSettings);
                time(&end);
                double dif = difftime(end, start);
                printf("Elapsed time is %.2lf seconds.\n", dif);
                PathOptimization optimizer(sliceMap);
                optimizer.OptimizePaths();
                vector<Slice> optimizedSlices = optimizer.GetSlices();
                intersection.SetSliceMap(optimizedSlices);
            }

            // button to export to gcode
            if (ImGui::Button("Export to Gcode")) {
                vector<Slice> slices = intersection.GetSliceMap();
                gcodeWriter.WriteGCode("/home/xandervaes/Code/ZupaSlica/GCodeOut", slices);
            }
        } 
        ImGui::End();

        
        ImGui::Begin("Scene");
        {
            ImGui::BeginChild("GameRender");

            SCENE_WIDTH = ImGui::GetContentRegionAvail().x;
            SCENE_HEIGHT = ImGui::GetContentRegionAvail().y;

            ImGui::Image(
                (ImTextureID)sceneBuffer.getFrameTexture(), 
                ImGui::GetContentRegionAvail(), 
                ImVec2(0, 1), 
                ImVec2(1, 0)
            );
            ImGui::EndChild();
        }
        ImGui::End();

        
        ImGui::Begin("Intersection");
        {
            ImGui::BeginChild("IntersectionRender");
            
            INTERSECTION_WIDTH = ImGui::GetContentRegionAvail().x;
            INTERSECTION_HEIGHT = ImGui::GetContentRegionAvail().y;

            ImGui::Image(
                (ImTextureID)intersectionBuffer.getFrameTexture(), 
                ImGui::GetContentRegionAvail(), 
                ImVec2(0, 1), 
                ImVec2(1, 0)
            );
            ImGui::EndChild();
        }
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
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    resizeBuffer = true;
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