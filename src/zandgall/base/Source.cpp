/*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                  PROJECT TEMPLATE v0.0.7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is an C++ OpenGL Project template
Project specific code files go into a seperate folder (or just filter if desired),
renamed to whatever the project is, of course.
Any framework can be kept in the "base" folder, these can
later be added to the Project Template for use in future
projects.

Cheers! Happy Coding!
    ~ Zandgall
*/
#define GLM_FORCE_SWIZZLE
#include "glhelper.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <stb_easy_font.h>
#include "handler.h"
#include <thread>
#include "../application/App.h"
#include <windows.h>

int main(const char* args) {
    START_OPEN_GL(4, 6, "Super Mario Bros. v0.1", NES_WIDTH * GAME_SCALE, NES_HEIGHT * GAME_SCALE);
    _addGLObject("default shader", loadShader("res/shaders/shader.shader"));
    glUseProgram(_getGLObject("default shader"));

    uniVec("col", glm::vec3(1, 0, 0));
    // Square VAO definitions
    float vertices[] = {
      1.0f,  1.0f, 0.0f,  // top right
      1.0f, -1.0f, 0.0f,  // bottom right
     -1.0f, -1.0f, 0.0f,  // bottom left
     -1.0f,  1.0f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    _addGLObject("Square VAO", 0);
    _addGLObject("Square VBO", 0);
    _addGLObject("Square EBO", 0);
    createVAO(&GLOBAL_GL_OBJECTS["Square VAO"]);
    createVBO(&GLOBAL_GL_OBJECTS["Square VBO"], vertices, sizeof(vertices));
    createEBO(&GLOBAL_GL_OBJECTS["Square EBO"], indices, sizeof(indices));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    using fps60 = std::chrono::duration<int64_t, std::ratio<1, 60>>;
    using clock = std::chrono::high_resolution_clock;
    auto currentTime = clock::now() - fps60{1}, frame_next = clock::now() + fps60{1};
    double delta = 0, pre_delta = 0, fps_last = glfwGetTime(), frame_start = glfwGetTime();
    int frames = 0, fps = 0;


    App app = App();
    // Main game loop
    while (!glfwWindowShouldClose(WINDOW)) {
        currentTime = clock::now();
        frame_next += fps60{1};

        double currentGLFWTime = glfwGetTime();
        pre_delta = delta;
        delta = std::min(currentGLFWTime - frame_start, 1.0);
        frame_start = currentGLFWTime;
        //std::cout << "Delta: " << delta << std::endl;
        if (delta < 0.01)
            frame_next = currentTime + fps60{ 1 };
        frames++;
        if (currentGLFWTime - fps_last >= 1) {
            std::cout << frames << " fps" << std::endl;
            fps = frames;
            frames = 0;
            fps_last = currentGLFWTime;
        }
        
        //Tick
        app.tick(pre_delta, delta);
        //Render
        if(WINDOW_WIDTH !=0 && WINDOW_HEIGHT != 0)
            app.render();

        glfwSwapBuffers(WINDOW);
        std::copy(keys, &keys[GLFW_KEY_LAST-1], pKeys);
        pmouseLeft = mouseLeft;
        pmouseRight = mouseRight;
        pmouseMiddle = mouseMiddle;
        pmouseX = mouseX;
        pmouseY = mouseY;
        pmouseScroll = mouseScroll;
        glfwPollEvents(); 

        std::this_thread::sleep_until(frame_next);
    }

    glfwTerminate();
    return 0;
}