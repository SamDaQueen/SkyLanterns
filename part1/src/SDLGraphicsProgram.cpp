#include "SDLGraphicsProgram.hpp"
#include "Camera.hpp"
#include "Terrain.hpp"
#include "Sphere.hpp"
#include "SimplexNoise.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// screen size globals
int screenWidth;
int screenHeight;

// Initialization function
// Returns a true or false value based on successful completion of setup.
// Takes in dimensions of window.
SDLGraphicsProgram::SDLGraphicsProgram(int w, int h) {
    // Initialization flag
    bool success = true;
    // String to hold any errors that occur.
    std::stringstream errorStream;
    // The window we'll be rendering to
    m_window = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        errorStream << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
        //Use OpenGL 3.3 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        // We want to request a double buffer for smooth updating.
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        //Create window
        m_window = SDL_CreateWindow("Lab", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        // Check if Window did not create.
        if (m_window == NULL) {
            errorStream << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        //Create an OpenGL Graphics Context
        m_openGLContext = SDL_GL_CreateContext(m_window);
        if (m_openGLContext == NULL) {
            errorStream << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        // Initialize GLAD Library
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
            errorStream << "Failed to iniitalize GLAD\n";
            success = false;
        }

        //Initialize OpenGL
        if (!InitGL()) {
            errorStream << "Unable to initialize OpenGL!\n";
            success = false;
        }
    }

    // If initialization did not work, then print out a list of errors in the constructor.
    if (!success) {
        errorStream << "SDLGraphicsProgram::SDLGraphicsProgram - Failed to initialize!\n";
        std::string errors = errorStream.str();
        SDL_Log("%s\n", errors.c_str());
    } else {
        SDL_Log("SDLGraphicsProgram::SDLGraphicsProgram - No SDL, GLAD, or OpenGL, errors detected during initialization\n\n");
    }

    // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN); // Uncomment to enable extra debug support!
    GetOpenGLVersionInfo();

    screenWidth = w;
    screenHeight = h;

    // Setup our Renderer
    m_renderer = new Renderer(screenWidth, screenHeight);
}


// Proper shutdown of SDL and destroy initialized objects
SDLGraphicsProgram::~SDLGraphicsProgram() {
    if (m_renderer != nullptr) {
        delete m_renderer;
    }


    //Destroy window
    SDL_DestroyWindow(m_window);
    // Point m_window to NULL to ensure it points to nothing.
    m_window = nullptr;
    //Quit SDL subsystems
    SDL_Quit();
}


// Initialize OpenGL
// Setup any of our shaders here.
bool SDLGraphicsProgram::InitGL() {
    //Success flag
    bool success = true;

    return success;
}

// Create the objects
SceneNode *Sky;
// Create the anchor, anchor 2 will replace anchor 1 when it goes
// beyond the screen and anchor 1 will start again at the bottom.
SceneNode *Anchor;
SceneNode *Anchor2;
std::vector<SceneNode *> lanterns;
std::vector<SceneNode *> lanterns2;


//Loops forever!
void SDLGraphicsProgram::Loop() {

    // ================== Initialize the lantern scene ===============

    // flag for first run
    static bool first = true;
    // the number of lanterns we want in our scene ( we will double this as there are two anchors)
    static int number_of_lanterns = 100;


    if (first) {
        // create the sky sphere
        Object *sky = new Sphere();
        sky->LoadTexture("stars.ppm");
        Sky = new SceneNode(sky, 1);

        // Create the first lantern object
        Object *lantern = new Sphere();
        lantern->LoadTexture("lantern.ppm");

        // Create the second lantern object
        Object *lantern2 = new Sphere();
        lantern2->LoadTexture("lantern2.ppm");

        // Create the lantern object
        Object *anchor = new Sphere();
        anchor->LoadTexture("black.ppm");

        // Create the first Anchor
        Anchor = new SceneNode(anchor, 1);

        // Create the second Anchor
        Anchor2 = new SceneNode(anchor, 1);

        // Render our scene starting from the anchor.
        m_renderer->setRoot(Sky);
        Sky->AddChild(Anchor);
        Sky->AddChild(Anchor2);

        // load the scene nodes
        for (int i = 0; i < number_of_lanterns; i++) {
            // using 1-D perlin noise for the random brighness
            float brightness = SimplexNoise::noise((float) i / 10);
            int lNum = (float) std::rand() * 10 / RAND_MAX;
            SceneNode *Lantern;
            SceneNode *Lantern2;
            if (lNum < 7) {
                Lantern = new SceneNode(lantern, brightness);
                Lantern2 = new SceneNode(lantern, brightness);
            } else {
                Lantern = new SceneNode(lantern2, brightness);
                Lantern2 = new SceneNode(lantern2, brightness);
            }
            lanterns.push_back(Lantern);
            lanterns2.push_back(Lantern2);
            Anchor->AddChild(Lantern);
            Anchor2->AddChild(Lantern2);
        }

        // Set a default position for our camera
        m_renderer->GetCamera(0)->SetCameraEyePosition(0.0f, 0.0f, 70.0f);
    }

    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();

    // Set the camera speed for how fast we move.
    float cameraSpeed = 5.0f;

    // While application is running
    while (!quit) {

        //Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User posts an event to quit
            // An example is hitting the "x" in the corner of the window.
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Handle keyboard input for the camera class
            switch (e.type) {
                // Handle keyboard presses
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_LEFT:
                            m_renderer->GetCamera(0)->MoveLeft(cameraSpeed);
                            break;
                        case SDLK_RIGHT:
                            m_renderer->GetCamera(0)->MoveRight(cameraSpeed);
                            break;
                        case SDLK_UP:
                            m_renderer->GetCamera(0)->MoveForward(cameraSpeed);
                            break;
                        case SDLK_DOWN:
                            m_renderer->GetCamera(0)->MoveBackward(cameraSpeed);
                            break;
                        case SDLK_RSHIFT:
                            m_renderer->GetCamera(0)->MoveUp(cameraSpeed);
                            break;
                        case SDLK_RCTRL:
                            m_renderer->GetCamera(0)->MoveDown(cameraSpeed);
                            break;
                    }
                    break;
            }
        } // End SDL_PollEvent loop.

        // to rotate lanterns
        static float rotate = 0.0f;
        rotate += 0.01;
        if (rotate > 360) {
            rotate = 0;
        }


        // for upward movement of the lanterns of both the anchors
        static float moveUp = -1.3f;
        static float moveUp2 = -2.2f;
        moveUp += 0.005f;
        moveUp2 += 0.005f;

        // reset anchor when it goes out of the screen
        if (moveUp > 0.45f) {
            moveUp = -1.55f;
        }
        if (moveUp2 > 0.45f) {
            moveUp2 = -1.55f;
        }

        // set precision to four digits
        moveUp = roundf(moveUp * 1000) / 1000;
        moveUp2 = roundf(moveUp2 * 1000) / 1000;

        // std::cout << moveUp << " " << moveUp2 << "\n";


        // load the sky and make it large
        Sky->GetLocalTransform().LoadIdentity();
        Sky->GetLocalTransform().Scale(100, 100, 100);

        // the scale of the anchors
        float anchorScale = 0.006f;

        // load first anchor and make it move up with rotation
        Anchor->GetLocalTransform().LoadIdentity();
        Anchor->GetLocalTransform().Translate(0.0f, moveUp, 0);
        Anchor->GetLocalTransform().Rotate(rotate, 0.0f, 1.0f, 0.0f);
        Anchor->GetLocalTransform().Scale(anchorScale, anchorScale, anchorScale);

        // load second anchor and make it move up with rotation
        Anchor2->GetLocalTransform().LoadIdentity();
        Anchor2->GetLocalTransform().Translate(0.0f, moveUp2, 0);
        Anchor2->GetLocalTransform().Rotate(rotate, 0.0f, 1.0f, 0.0f);
        Anchor2->GetLocalTransform().Scale(anchorScale, anchorScale, anchorScale);

        // the co-ordinates of the lanterns
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        static std::vector <glm::vec3> randoms;
        static std::vector<float> size;

        // set the positions for the first time
        if (first) {
            for (int i = 0; i < number_of_lanterns; i++) {
                glm::vec3 coord = glm::vec3((float) std::rand() * 80 / RAND_MAX, (float) std::rand() * 200 / RAND_MAX,
                                            (float) std::rand() * 80 / RAND_MAX);
                randoms.push_back(coord);
                size.push_back((float) std::rand() * 5 / RAND_MAX + 3);
            }
            first = false;
        }

        // to change x and z positions of lanterns
        int signX = 1;
        int signZ = 1;

        // translate all lanterns according to random numbers
        for (int j = 1; j < 5; j++) {
            for (int i = (j - 1) * number_of_lanterns / 4; i < j * number_of_lanterns / 4; i++) {
                x = randoms[i].x;
                y = randoms[i].y;
                z = randoms[i].z;
                lanterns[i]->GetLocalTransform().LoadIdentity();
                lanterns[i]->GetLocalTransform().Translate((x) * signX, y, (z) * signZ);
                lanterns[i]->GetLocalTransform().Rotate(signX * signZ * rotate * 10, 0.0f, 1.0f, 0.0f);
                lanterns[i]->GetLocalTransform().Scale(size[i], size[i], size[i]);
                lanterns2[i]->GetLocalTransform().LoadIdentity();
                lanterns2[i]->GetLocalTransform().Translate((x) * signX, y, (z) * signZ);
                lanterns2[i]->GetLocalTransform().Rotate(signX * signZ * rotate * 10, 0.0f, 1.0f, 0.0f);
                lanterns2[i]->GetLocalTransform().Scale(size[i], size[i], size[i]);
            }
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            // invert the signs so that some lanterns have positive x and some have negative (same with z)
            signX *= -1.0f;
            if (j == 2) {
                signZ *= -1.0f;
            }
        }

        // Update our scene through our renderer
        m_renderer->Update();

        // Render our scene using our selected renderer
        m_renderer->Render();

        // Delay to slow things down just a bit!
        SDL_Delay(50);  // TODO: You can change this or implement a frame
        // independent movement method if you like.
        //Update screen of our specified window
        SDL_GL_SwapWindow(GetSDLWindow());
    }

//Disable text input
    SDL_StopTextInput();

}


// Get Pointer to Window
SDL_Window *SDLGraphicsProgram::GetSDLWindow() {
    return m_window;
}

// Helper Function to get OpenGL Version Information
void SDLGraphicsProgram::GetOpenGLVersionInfo() {
    SDL_Log("(Note: If you have two GPU's, make sure the correct one is selected)");
    SDL_Log("Vendor: %s", (const char *) glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", (const char *) glGetString(GL_RENDERER));
    SDL_Log("Version: %s", (const char *) glGetString(GL_VERSION));
    SDL_Log("Shading language: %s", (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));
}
