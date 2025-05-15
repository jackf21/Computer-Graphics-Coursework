#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

#define PI 3.1415926536

// Function prototypes
void keyboardInput(GLFWwindow* window);
void mouseInput(GLFWwindow* window);

// Frame timers
float previousTime = 0.0f;  // time of previous iteration of the loop
float deltaTime = 0.0f;  // time elapsed since the previous frame

// Create camera object
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f));
glm::vec3 previousCameraPosition = camera.eye;

// Object types used to differenciate objects
enum objectType {
    OBJECT,
    ENVINOMENT,
    INTERACTIVE
};

// Object struct
struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle = 0.0f;
    float width = 0.0f;
    std::string name;
    objectType type;
};

// Player attributes
glm::vec3 playerPosition = camera.eye;
glm::vec3 playerDirection = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 playerRotation = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 previousPlayerPosition = playerPosition;
float playerAngle = 0.0f;
float playerTargetAngle = 360.0f;

// Object vector to collect objects
std::vector<Object> objects;

// Light object that contains all of the lights
Light lightSources;

//Bullet object (needs to be outside main to be accessed by key inputs)
Object bulletObject;
glm::vec3 bulletDirection = glm::vec3(1.0f, 0.0f, 0.0f);

// Game variables
bool playerCollided = false;
bool teapotTrigger = false;
float tick;
float cameraBaseY;

int main(void)
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Graphics Coursework", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use back face culling
    glEnable(GL_CULL_FACE);

    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Capture mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Compile shader program
    unsigned int shaderID;
    shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Activate shader
    glUseProgram(shaderID);

    // Load models
    Model lightSphere("../assets/sphere.obj");
    Model teapot("../assets/teapot.obj");
    Model teapotGun("../assets/teapotGun.obj");
    Model catSphere("../assets/sphere.obj");
    Model bullet("../assets/bullet.obj");
    Model walls("../assets/wall.obj");
    Model floor("../assets/floor.obj");

    // Load the textures
    lightSphere.addTexture("../assets/LightSource.png", "diffuse");
    lightSphere.addTexture("../assets/LightSource.png", "normal");
    lightSphere.addTexture("../assets/LightSource.png", "specular");

    teapot.addTexture("../assets/gold.bmp", "diffuse");
    teapot.addTexture("../assets/droplet_normal.png", "normal");
    teapot.addTexture("../assets/droplet_specular.png", "specular");

    teapotGun.addTexture("../assets/gold.bmp", "diffuse");
    teapotGun.addTexture("../assets/droplet_normal.png", "normal");
    teapotGun.addTexture("../assets/droplet_specular.png", "specular");

    catSphere.addTexture("../assets/Smile.png", "diffuse");
    catSphere.addTexture("../assets/neutral_normal.png", "normal");
    catSphere.addTexture("../assets/neutral_specular.png", "specular");

    bullet.addTexture("../assets/grey.png", "diffuse");
    bullet.addTexture("../assets/neutral_normal.png", "normal");
    bullet.addTexture("../assets/neutral_specular.png", "specular");

    walls.addTexture("../assets/crate.jpg", "diffuse");
    walls.addTexture("../assets/neutral_normal.png", "normal");
    walls.addTexture("../assets/neutral_specular.png", "specular");

    floor.addTexture("../assets/crate.jpg", "diffuse");
    floor.addTexture("../assets/neutral_normal.png", "normal");
    floor.addTexture("../assets/neutral_specular.png", "specular");

    // Define object lighting properties
    float ambient = 0.2f;

    lightSphere.ka = 1.0f;
    lightSphere.kd = 0.7f;
    lightSphere.ks = 1.0f;
    lightSphere.Ns = 20.0f;

    teapot.ka = ambient;
    teapot.kd = 0.7f;
    teapot.ks = 1.0f;
    teapot.Ns = 20.0f;

    teapotGun.ka = ambient;
    teapotGun.kd = 0.7f;
    teapotGun.ks = 1.0f;
    teapotGun.Ns = 20.0f;

    catSphere.ka = 0.5f;
    catSphere.kd = 0.7f;
    catSphere.ks = 1.0f;
    catSphere.Ns = 20.0f;

    bullet.ka = ambient;
    bullet.kd = 0.7f;
    bullet.ks = 1.0f;
    bullet.Ns = 20.0f;

    walls.ka = ambient;
    walls.kd = 0.7f;
    walls.ks = 1.0f;
    walls.Ns = 20.0f;

    floor.ka = ambient;
    floor.kd = 0.7f;
    floor.ks = 1.0f;
    floor.Ns = 20.0f;

    // Add light sources

    lightSources.addSpotLight(glm::vec3(0.0f, 5.0f, 0.0f),          // position
                              glm::vec3(0.0f, -1.0f, 0.0f),         // direction
                              glm::vec3(1.0f, 1.0f, 1.0f),          // colour
                              1.0f, 0.1f, 0.02f,                    // attenuation
                              std::cos(Maths::radians(20.0f)));     // cos(phi)
    lightSources.addDirectionalLight(glm::vec3(0.0f, -1.0f, 0.0f),  // direction
                                    glm::vec3(1.0f, 1.0f, 1.0f));  // colour
    
    lightSources.lightSources[1].enabled = false;
    
    // Flashlight to be enabled after the teapot has been picked up;
    //LightSource flashLight;
    //flashLight.position = camera.eye;
    //flashLight.colour = glm::vec3(1.0f, 1.0f, 1.0f);
    //flashLight.direction = camera.front;
    //flashLight.constant = 1.0f;
    //flashLight.linear = 0.1f;
    //flashLight.quadratic = 0.02f;
    //flashLight.cosPhi = std::cos(Maths::radians(40.0f));
    //flashLight.type = 2;
    //flashLight.name = "playerFlashlight";
    //flashLight.enabled = false;
    //lightSources.lightSources.push_back(flashLight);

    // Add objects to objects vector
    Object object;

    // staticTeapot
    object.name = "staticTeapot";
    object.type = OBJECT;
    object.position = glm::vec3(0.0f, 0.0f, 0.0f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.angle = 0.0f;
    object.width = 1.0f;
    object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    objects.push_back(object);

    // <Room>
    // Walls
    object.name = "walls";
    object.type = ENVINOMENT;
    //object.position = glm::vec3(0.0f, -1.0f, 0.0f);
    object.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    object.scale = glm::vec3(3.0f, 3.0f, 3.0f);
    float wallAngle = 0.0f;
    glm::vec3 xWallPosition = glm::vec3(-3.5f * object.scale.x, -1.0f, 0.0f);
    glm::vec3 zWallPosition = glm::vec3(0.0f, -1.0f, -3.5f * object.scale.z);
    glm::vec3 wallPosition = glm::vec3(xWallPosition.x, 0.0f, zWallPosition.z); // Positions used for "collision"
    for (unsigned int i = 0; i < 4; i++)
    {
        object.angle = Maths::radians(wallAngle);
        if (wallAngle == 0.0f || wallAngle == 180.0f)
        {
            object.position = zWallPosition;
            zWallPosition.z *= -1.0f;
        }
        else
        {
            object.position = xWallPosition;
            xWallPosition.x *= -1.0f;
        }
        objects.push_back(object);
        wallAngle += 90.0f;
        
    }

    // Floor
    object.name = "floor";
    object.position = glm::vec3(0.0f, -1.0f, 0.0f);
    object.angle = 0.0f;
    object.width = 0.0f;
    objects.push_back(object);

    // Roof
    object.name = "roof";
    object.position = glm::vec3(0.0f, 20.0f, 0.0f);
    object.rotation = glm::vec3(0.0f, 0.0f, 1.0f);
    object.angle = Maths::radians(180.0f);
    objects.push_back(object);
    // </Room>
    
    // Player object used for the player model 
    Object player;
    player.name = "player";
    player.position = playerPosition;
    player.rotation = playerRotation;
    player.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    player.angle = Maths::radians(180.0f);

    // Teapot held by the player
    Object playerGun;
    playerGun.name = "teapotGun";
    playerGun.position = camera.eye + glm::vec3(1.0f, 0.0f, 1.0f);
    playerGun.rotation = glm::vec3(0.0f, -1.0f, 0.0f);
    playerGun.angle = 0.0f;
    playerGun.scale = glm::vec3(0.2f, 0.2f, 0.2f);

    // Bullet
    bulletObject.name = "bullet";
    bulletObject.position = camera.eye;
    bulletObject.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    bulletObject.angle = 0.0f;
    bulletObject.width = 0.4f;
    bulletObject.scale = glm::vec3(0.05f, 0.05f, 0.05f);
    //objects.push_back(bulletObject);

    std::cout << "Load time: " << glfwGetTime() * 1000 << "ms" << std::endl;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Update timer
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        // Get inputs
        keyboardInput(window);
        mouseInput(window);

        if (camera.isJumping) {
            tick += 1.0f * deltaTime;
            camera.eye.y = 3.0f * sinf(PI * tick);
            playerPosition.y = camera.eye.y;
            //std::cout << camera.eye.y << std::endl;
            if (camera.eye.y <= cameraBaseY) {
                camera.eye.y = cameraBaseY;
                playerPosition.y = cameraBaseY;
                tick = 0.0f;
                camera.isJumping = false;
            }
        }

        // Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate view and projection matrices
        camera.target = camera.eye + camera.front;
        camera.quaternionCamera();

        // Activate shader
        glUseProgram(shaderID);

        // Send light source properties to the shader
        lightSources.toShader(shaderID, camera.view);
        
        // Send view matrix to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "V"), 1, GL_FALSE, &camera.view[0][0]);


        // Only draw the player model if in 3rd person
        if (camera.isThird == true)
        {
            camera.eye = playerPosition;

            // Drawing the player model
            //glm::normalize(playerDirection);
            glm::mat4 translate = Maths::translate(playerPosition);
            glm::mat4 scale = Maths::scale(player.scale);
            glm::mat4 rotate = Maths::rotate(playerAngle, playerRotation);
            glm::mat4 model = translate * rotate * scale;
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);
            catSphere.draw(shaderID);
        }


        // =============================================================
        // OBJECT LOOP
        // =============================================================
        for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            if (&objects[i] == NULL)
            {
                printf("Object at %u not found\n", i);
                continue;
            }

            // modifying object properties during runtime
            if (objects[i].name == "staticTeapot")
            {
                objects[i].position.y = 0.5 * Maths::square(sinf(glfwGetTime() * 2));
                objects[i].angle = glfwGetTime() * 2;
            }

            if (objects[i].name == "teapotGun")
            {
                objects[i].position = camera.eye + Maths::normalise(camera.front) * glm::vec3(1.0f, 0.0f, 1.0f);
                //objects[i].rotation = playerRotation;
                objects[i].angle = camera.yaw;
                //objects[i].position = glm::vec3(1.0f + cosf(Maths::radians(camera.yaw)), 0.0f, 1.0f + sinf(Maths::radians(camera.yaw)));
            }

            if (objects[i].name == "bullet") 
            {
                objects[i].position += bulletDirection * 20.0f * deltaTime;
            }

            // Calculate model matrix
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            glm::mat4 model = translate * rotate * scale;

            // Send the MVP and MV matrices to the vertex shader
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            // Draw the model
            if (objects[i].name == "staticTeapot")
                teapot.draw(shaderID);
            if (objects[i].name == "teapotGun")
                teapotGun.draw(shaderID);
            if (objects[i].name == "bullet")
                bullet.draw(shaderID);
            if (objects[i].name == "walls")
                walls.draw(shaderID);
            if (objects[i].name == "floor" || objects[i].name == "roof")
                floor.draw(shaderID);

            // Check for collision (ignoring y bcs I am lazy :D) none if in free cam
            if(!camera.isFreeCam)
            {
                switch (objects[i].type)
                {
                case OBJECT:
                    glm::vec3 positionDiff = glm::vec3(camera.eye.x, 0.0f, camera.eye.z) - glm::vec3(objects[i].position.x, 0.0f, objects[i].position.z);
                    if (Maths::length(positionDiff) <= objects[i].width)
                    {
                        playerCollided = true;
                        //camera.eye += glm::vec3(Maths::normalise(positionDiff).x, 0.0f, Maths::normalise(positionDiff).z) * 0.1f; // Moving the camera back (rather than resetiing the position)
                        camera.eye = previousCameraPosition;
                        playerPosition = previousPlayerPosition;
                    }
                    else if (Maths::length(positionDiff) > objects[i].width)
                    {
                        playerCollided = false;
                    }

                    if (playerCollided)
                    {
                        if (objects[i].name == "staticTeapot")
                        {
                            teapotTrigger = true;
                            objects[i] = playerGun;
                        }
                    }
                    break;
                }

                if (camera.eye.x - 0.5 < wallPosition.x || camera.eye.z - 0.5 < wallPosition.z || camera.eye.x + 0.5 > -wallPosition.x || camera.eye.z + 0.5 > -wallPosition.z)
                {
                    playerCollided = true;
                    camera.eye = previousCameraPosition;
                    playerPosition = previousPlayerPosition;
                }
                else
                {
                    playerCollided = false;
                }
            }
        }
        // =============================================================
        // END OF OBJECT LOOP
        // =============================================================

        //std::cout << camera.eye << std::endl;
        //std::cout << playerCollided << std::endl;

        if (playerCollided) {
            
        }

        // Draw light sources
        if (teapotTrigger)
        {
            // This is a better way  to do it vv
            // Enable player flashlight
            //if (lightSources.lightSources[i].name == "playerFlashlight") {
            //    lightSources.lightSources[i].enabled = true;
            //    lightSources.lightSources[i].position = camera.eye;
            //    lightSources.lightSources[i].direction = camera.front;
            //}
            //else
            //{
            //    // Disable all other lighting
            //    lightSources.lightSources[i].enabled = false;
            //}
            // Easier (bad) way to swap to the flashlight by moving the spotlight
            lightSources.lightSources[0].position = camera.eye;
            lightSources.lightSources[0].direction = camera.front;
            lightSources.lightSources[0].cosPhi = Maths::radians(40.0f);
            lightSources.lightSources[0].drawSource = false;
        }

        lightSources.draw(shaderID, camera.view, camera.projection, lightSphere);

        // Update previous positions
        previousCameraPosition = camera.eye;
        previousPlayerPosition = playerPosition;

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    lightSphere.deleteBuffers();
    catSphere.deleteBuffers();
    teapot.deleteBuffers();
    teapotGun.deleteBuffers();
    bullet.deleteBuffers();
    walls.deleteBuffers();
    floor.deleteBuffers();
    glDeleteProgram(shaderID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Reseting movement vector when the key is released
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
    {
        playerDirection.z = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
    {
        playerDirection.x = 0.0f;
    }

    glm::vec3 movementVector = Maths::normalise(glm::vec3(camera.front.x, 0.0f, camera.front.z)); // Camera front vector with no y (used to lock movement to x and z axis)

    // Move the camera / player using WSAD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if (camera.isThird)
        {
            playerPosition += 5.0f * deltaTime * movementVector;

            // Special case required for diagonal movement (repeated in A key press) as subtracting 1/4 results in the same diagonal as S and D
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                playerAngle = Maths::lerp(playerAngle, -camera.yaw + (3 * PI / 4), glfwGetTime() * deltaTime);
            else
                playerAngle = Maths::lerp(playerAngle, -camera.yaw + (PI / 2), glfwGetTime() * deltaTime);
            //playerAngle = -camera.yaw;
            //std::cout << -camera.yaw << std::endl;
        }
        else if (camera.isFreeCam)
        {
            camera.eye += 5.0f * deltaTime * camera.front;
        }
        else 
        {
            camera.eye += 5.0f * deltaTime * movementVector;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (camera.isThird)
        {
            playerPosition -= 5.0f * deltaTime * movementVector;
            playerAngle = Maths::lerp(playerAngle, -camera.yaw - (PI / 2), glfwGetTime() * deltaTime);
            //playerAngle = -camera.yaw - PI;
        }
        else if (camera.isFreeCam)
        {
            camera.eye -= 5.0f * deltaTime * camera.front;
        }
        else
        {
            camera.eye -= 5.0f * deltaTime * movementVector;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (camera.isThird)
        {
            playerPosition -= 5.0f * deltaTime * camera.right;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                playerAngle = Maths::lerp(playerAngle, -camera.yaw + (3 * PI / 4), glfwGetTime() * deltaTime);
            else
                playerAngle = Maths::lerp(playerAngle, -camera.yaw - PI, glfwGetTime() * deltaTime);
            //playerAngle = -camera.yaw + (PI / 2);
        }
        else
        {
            camera.eye -= 5.0f * deltaTime * camera.right;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (camera.isThird)
        {
            playerPosition += 5.0f * deltaTime * camera.right;
            playerAngle = Maths::lerp(playerAngle, -camera.yaw, glfwGetTime() * deltaTime);
            //playerAngle = -camera.yaw - (PI / 2);
        }
        else
        {
            camera.eye += 5.0f * deltaTime * camera.right;
        }
    }
    
    // Jumping
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!camera.isJumping) {
            cameraBaseY = camera.eye.y;
            camera.isJumping = true;
        }
    }

    // Shooting
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE)
        {
            if (teapotTrigger)
            {
                bulletObject.position = camera.eye + movementVector * 1.2f;
                bulletObject.angle = -camera.yaw;
                bulletDirection = movementVector;
                objects.push_back(bulletObject);
            }
        }
    }

    // Third / first person swap
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE && !camera.isFreeCam)
        {
            playerPosition = camera.eye;
            playerAngle = -camera.yaw + (PI / 2);
            camera.isThird = !camera.isThird;

        }
    }

    // Free cam
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE)
        {
            camera.isThird = false;
            camera.isFreeCam = !camera.isFreeCam;
            if (camera.isFreeCam) {
                std::cout << "Enabling free cam" << std::endl;
            }
            else {
                std::cout << "Disabling free cam" << std::endl;
            }
        }
    }


    // Changing light colours
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        {
            lightSources.lightSources[0].colour = glm::vec3(0.0f, 0.0f, 0.0f);
            std::cout << "Changing light colour to black" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        {
            lightSources.lightSources[0].colour = glm::vec3(1.0f, 1.0f, 1.0f);
            std::cout << "Changing light colour to white" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
        {
            lightSources.lightSources[0].colour = glm::vec3(1.0f, 0.0f, 0.0f);
            std::cout << "Changing light colour to red" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
        {
            lightSources.lightSources[0].colour = glm::vec3(0.0f, 1.0f, 0.0f);
            std::cout << "Changing light colour to green" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
        {
            lightSources.lightSources[0].colour = glm::vec3(0.0f, 0.0f, 1.0f);
            std::cout << "Changing light colour to blue" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) 
    {
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) 
        {
            lightSources.lightSources[1].enabled = !lightSources.lightSources[1].enabled;
        }
    }
}

void mouseInput(GLFWwindow* window)
{
    // Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Update yaw and pitch angles
    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    //std::cout << asinf(sinf(camera.yaw)) << std::endl;
    //playerTargetAngle += camera.yaw * camera.pitch;
    camera.pitch += 0.005f * float(768 / 2 - yPos);
    if (camera.isThird)
        camera.pitch = Maths::clamp(camera.pitch, -1.2f, 0.5f);
    else
        camera.pitch = Maths::clamp(camera.pitch, -1.2, 1.2);

    // Calculate camera vectors from the yaw and pitch angles
    camera.calculateCameraVectors();
}