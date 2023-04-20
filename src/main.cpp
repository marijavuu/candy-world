#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>1

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

//skajboks
unsigned int loadTexture(char const * path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1600;//izmene
const unsigned int SCR_HEIGHT = 800;//izmene

// camera
Camera camera(glm::vec3(4.0f, 5.0f, 25.0f));//izmena
bool CameraMouseMovementUpdateEnabled = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/*
//hamburgeri
glm::vec3 hamburgeripoz = glm::vec3(0.0f);
float hamburgeriscale = 1.0f;

//tortica
glm::vec3 torticapoz = glm::vec3(0.0f);
float torticascale = 1.0f;

//keksici
glm::vec3 keksicipoz = glm::vec3(0.0f);
float keksiciscale = 1.0f;

//cheezespider
glm::vec3 cheezespiderpoz = glm::vec3(0.0f);
float cheezespiderscale = 1.0f; //ne valja

//ananas
glm::vec3 ananaspoz = glm::vec3(0.0f);
float ananasscale = 1.0f;

//cocacola1
glm::vec3 cocacola1poz = glm::vec3(0.0f);
float cocacola1scale = 1.0f;

//cocacola2
glm::vec3 cocacola2poz = glm::vec3(0.0f);
float cocacola2scale = 1.0f;
*/
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};
struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}
    void SaveToFile(std::string filename);
    void LoadFromFile(std::string filename);
};
void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}
void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}
ProgramState *programState;
void DrawImGui(ProgramState *programState);

int main() {
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);//vrv ovo remeti ,pa naopacke

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);//p

    // build and compile shaders
    // -------------------------
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader tanjirShader("resources/shaders/tanjir.vs","resources/shaders/tanjir.fs");


    // tanjir
    float vertices[] = {          //naopacke !!!!!!
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    //vao i vbo za tanjir
    unsigned int tanjirVAO, tanjirVBO;
    glGenVertexArrays(1, &tanjirVAO);
    glGenBuffers(1, &tanjirVBO);
    glBindVertexArray(tanjirVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tanjirVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);




    /*
    //pod
    float floorVertices[] = {
            // positions          // normals          // texture coords
            0.5f,  0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  1.0f,  1.0f,  // top right
            0.5f, -0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  1.0f,  0.0f,  // bottom right
            -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,  // bottom left
            -0.5f,  0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f,  1.0f   // top left
    };

    // floor vertices for use in EBO
    unsigned int floorIndices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };
    */

    glm::vec3 tanjiric = glm::vec3(-4.0f,8.0f,-60.0f);


    // load models
    // -----------
    Model hamburgeri("resources/objects/model16/hamburgeres.obj");
    hamburgeri.SetShaderTextureNamePrefix("material.");

    //Model tortica("resources/objects/model20/cakeSlice+2xPlates+strawberry.obj");
    //tortica.SetShaderTextureNamePrefix("material.");

    Model keksici("resources/objects/model22/Biscuit.obj");
    keksici.SetShaderTextureNamePrefix("material.");

    Model cheezespider("resources/objects/model24/cheezespider.obj");
    cheezespider.SetShaderTextureNamePrefix("material.");

    Model ananas("resources/objects/model25/10200_Pineapple_v1-L2.obj");
    ananas.SetShaderTextureNamePrefix("material.");

    //nesto me ova koca zeza ,nema ispunjenu casu ,nzm sto,a volim ovaj model
    Model cocacola1("resources/objects/model27/cup OBJ.obj");
    cocacola1.SetShaderTextureNamePrefix("material.");

    //Model cocacola2("resources/objects/model28/coke.obj");
    //cocacola2.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 0.5f;//proba
    pointLight.linear = 0.00009f;//proba
    pointLight.quadratic = 0.000032f;//proba

    // load textures
    //unsigned int cocacola1tex = loadTexture(FileSystem::getPath("resources/objects/model27/drinktex.png").c_str());
    //***********************************************************************************


    // load textures
    unsigned int  tanjirictex= loadTexture(FileSystem::getPath("resources/textures/tanjir5.png").c_str());



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

    /*
    // Floor setup
    unsigned int floorVAO, floorVBO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //ovo mozda ugasi
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    //glBindVertexArray(0); mozda ne treba

     */


    // skybox VAO, VBO, and loading textures
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //ucitavanje teksture kokalkole probaa zbog soka
    /*
    unsigned int cocacola11 = loadTexture(FileSystem::getPath("resources/objects/model27/Cocacolatexture.jpg").c_str());
    unsigned int cocacola12 = loadTexture(FileSystem::getPath("resources/objects/model27/drinktex.png").c_str()); //!!!!!!!!!!!!!!!!!!!!!!!!
    unsigned int cocacola13 = loadTexture(FileSystem::getPath("resources/objects/model27/IceNormalMap.jpg").c_str());
    unsigned int cocacola14 = loadTexture(FileSystem::getPath("resources/objects/model27/IceTexture.jpg").c_str());
    */
     // floor
    //unsigned int floorDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/crna.jpg").c_str());


    vector<std::string> faces {
      /*
          //ovo ti je neki skybox sa neta ,nisu namestene ivice kocke,proba ,bolji tvoj!

                   FileSystem::getPath("resources/textures/skybox/0003_0.jpg"),
                    FileSystem::getPath("resources/textures/skybox/0001_0.jpg"),
                    FileSystem::getPath("resources/textures/skybox/0005_0.jpg"),
                    FileSystem::getPath("resources/textures/skybox/0004_0.jpg"),
                    FileSystem::getPath("resources/textures/skybox/0002_0.jpg"),
                    FileSystem::getPath("resources/textures/skybox/0006_0.jpg")
      */




            FileSystem::getPath("resources/textures/skybox/12.jpg"),
            FileSystem::getPath("resources/textures/skybox/12.jpg"),
            FileSystem::getPath("resources/textures/skybox/12.jpg"),
            FileSystem::getPath("resources/textures/skybox/12.jpg"),
            FileSystem::getPath("resources/textures/skybox/2.jpg"),
            FileSystem::getPath("resources/textures/skybox/5.jpg")



    };

    //stbi_set_flip_vertically_on_load(true);

//***************************************************************

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    //Shader activation
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    unsigned int cubemapTexture = loadCubemap(faces);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);//izmeni
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);


        // don't forget to enable shader before setting uniforms
        ourShader.use();

        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);



        ourShader.setVec3("viewPosition",programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);


        // view/projection transformations
        //glm::mat4 model = glm::mat4(1.0f);//vidi ovo !!!!!!!!! za pod
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model

        //hamburgeri

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(10.0f,(5.0f+ sin(glfwGetTime())/6),1.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.2f,1.5f,1.2f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        hamburgeri.Draw(ourShader);

        /*
        //tortica
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(15.0f,(5.0f+ sin(glfwGetTime())/6),7.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.4f,0.4f,0.4f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        tortica.Draw(ourShader);

        */

        //keksici
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-7.0f,(10.0f+ sin(glfwGetTime())/6),-15.0f)
                               ); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.2f,1.5f,1.2f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        keksici.Draw(ourShader);


        //cheezespider
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,(7.0f+ sin(glfwGetTime())/6),15.0f)
                               ); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.02f,0.02f,0.02f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        cheezespider.Draw(ourShader);


        //ananas
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-2.0f,(10.0f+ sin(glfwGetTime())/6),-15.0f)
                               ); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f,0.2f,0.2f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ananas.Draw(ourShader);


        //cocacola1
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(5.0f,(10.0f+ sin(glfwGetTime())/6),-15.0f) ); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f,0.2f,0.2f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        /*
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cocacola11);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, cocacola12);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, cocacola13);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, cocacola14);
         */
        cocacola1.Draw(ourShader);

         /*
        //cocacola2
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               cocacola2poz); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(cocacola2scale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        cocacola2.Draw(ourShader);
         */

       if (programState->ImGuiEnabled)
           DrawImGui(programState);

        /*
        //PODDDDD,KOJI IPAK VRV NECEMO
        // floor setup
        // light properties
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        //floor world transformation
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.51f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(25.0f));
        ourShader.setMat4("model", model);
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffuseMap);
        // render floor
        glBindVertexArray(floorVAO);
        glEnable(GL_CULL_FACE);     // floor won't be visible if looked from bellow
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glDisable(GL_CULL_FACE);
        */

        //Enabling back face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);


        //tanjir
        tanjirShader.use();
        tanjirShader.setMat4("projection", projection);
        tanjirShader.setMat4("view", view);

        glBindVertexArray(tanjirVAO);
        glBindTexture(GL_TEXTURE_2D, tanjirictex);
        model = glm::mat4(1.0f);
        model = glm::translate(model,tanjiric);
        model = glm::scale(model, glm::vec3(30.0f,10.0f,30.0f));
        tanjirShader.setMat4("model",model);
        glDrawArrays(GL_TRIANGLES,0,6);



        //*************************************************************************
        // draw skybox as last
        // change depth function so depth test passes when values are equal to depth buffer's content
        //glDepthMask(GL_FALSE);//!!!!!!

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        model = glm::mat4(1.0f);//MOZDA JE OVO PROBLEM
        projection=glm::perspective(glm::radians(programState->camera.Zoom),(float )SCR_WIDTH/(float )SCR_HEIGHT , 0.1f ,100.0f);
        //view=glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        //skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("view", glm::mat4(glm::mat3 (view)));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        //glDepthMask(GL_TRUE);//!!!!!
        glDepthFunc(GL_LESS); // set depth function back to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //*********************************************

        // view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //deleting arrays and buffers
    //glDeleteVertexArrays(1, &floorVAO);
    glDeleteVertexArrays(1, &skyboxVAO);//~~~~~~~~~~~~~~~~~~

    //glDeleteBuffers(1, &floorVBO);
    //glDeleteBuffers(1, &floorEBO);
    glDeleteBuffers(1, &skyboxVAO);//~~~~~~~~~~~~~~~~~~~~~~~~
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {

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
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (CameraMouseMovementUpdateEnabled)//bolje bez ovoga
        camera.ProcessMouseMovement(xoffset, yoffset);//izmeni

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);
        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }
    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
//Cubemap loading function
//------------------------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
//Texture loading function
//-----------------------------------------------------------

unsigned int loadTexture(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
