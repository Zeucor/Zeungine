#include <iostream>
#include <vector>
#include <cmath> // For spherical calculations and std::fmod, std::abs
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
#include <zg/crypto/Random.hpp>
#include <zg/components/scenes/Bloom.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional> // Required for std::function

using namespace zg;

// Constants for SphereFactory
const int NUM_SPHERE_CUBES = 1600; // Total number of cubes for the sphere
const float SPHERE_RADIUS = 80.0f; // Radius of the initial sphere
const float SPHERE_CUBE_INITIAL_SCALE = 4.0f; // Initial uniform scale for sphere cubes

// Constants for CubeGridFactory
const int GRID_SIZE = 15; // Number of cubes along each axis for the grid
const float GRID_SPACING = 10.0f; // Spacing between cubes in the grid
const float GRID_CUBE_INITIAL_SCALE = 3.0f; // Initial uniform scale for grid cubes

// Constants for RandomCubesFactory
const int NUM_RANDOM_CUBES = 1000; // Total number of cubes for random distribution
const float RANDOM_BOUNDING_BOX_SIZE = 100.0f; // Size of the bounding box for random cubes
const float RANDOM_CUBE_INITIAL_SCALE = 2.0f; // Initial uniform scale for random cubes

// Constants for SpiralFactory
const int NUM_SPIRAL_CUBES = 800; // Total number of cubes for the spiral
const float SPIRAL_RADIUS_START = 10.0f; // Starting radius of the spiral
const float SPIRAL_RADIUS_END = 60.0f; // Ending radius of the spiral
const float SPIRAL_HEIGHT = 100.0f; // Total height of the spiral
const float SPIRAL_TURNS = 10.0f; // Number of turns in the spiral
const float SPIRAL_CUBE_INITIAL_SCALE = 3.0f; // Initial uniform scale for spiral cubes

// Base cube create info (can be customized per factory)
auto baseCubeCreateInfo = entities::CubeFactory("Base Cube", {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});

// Function declarations for scene factories
SceneCreateInfo SphereFactory();
SceneCreateInfo CubeGridFactory();
SceneCreateInfo RandomCubesFactory();
SceneCreateInfo SpiralFactory();

// Helper function for HSL to RGB conversion (simplified)
glm::vec3 hslToRgb(float h, float s, float l) {
    float c = (1.0f - glm::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - glm::abs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    glm::vec3 rgb;
    if (h < 1.0f / 6.0f) rgb = glm::vec3(c, x, 0);
    else if (h < 2.0f / 6.0f) rgb = glm::vec3(x, c, 0);
    else if (h < 3.0f / 6.0f) rgb = glm::vec3(0, c, x);
    else if (h < 4.0f / 6.0f) rgb = glm::vec3(0, x, c);
    else if (h < 5.0f / 6.0f) rgb = glm::vec3(x, 0, c);
    else rgb = glm::vec3(c, 0, x);

    return rgb + glm::vec3(m);
}

int main()
{
    // Window creation info
    WindowCreateInfo windowCreateInfo{.title = "Beautiful 3D Scene Factories Test", .borderless = true, .vsync = false, .framerate = 144};
    auto window_tuple = zg::Registry::addWindow(windowCreateInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);

    size_t current_scene_id = 0; // Initialize with 0
    size_t scene_factory_index = 0;

    // Vector of scene factory functions
    std::vector<std::function<SceneCreateInfo()>> scene_factories = {
        SphereFactory,
        CubeGridFactory,
        RandomCubesFactory,
        SpiralFactory
    };

    // Lambda function to switch to the next scene
    auto nextScene = [&]() mutable {
        if (current_scene_id != 0) // Check if a scene is currently attached
        {
            window.removeScene(current_scene_id);
        }

        // Get the create info for the next scene
        auto sceneCreateInfo = scene_factories[scene_factory_index]();

        // Increment the factory index, loop back if necessary
        scene_factory_index++;
        if (scene_factory_index == scene_factories.size())
            scene_factory_index = 0;

        // Add the new scene to the window
        auto scene_tuple = window.addScene(sceneCreateInfo);
        current_scene_id = std::get<KEY_ID_VECTOR_ID_INDEX>(scene_tuple);
    };

    // Run the initial scene setup on the window's thread
    window.runOnThread([&](auto& window) mutable {
        nextScene();
    });

    // Add a key press handler for the right arrow key to switch scenes
    window.addKeyPressHandler(KEYCODE_RIGHT, [&](auto pressed) {
        if (!pressed)
            return;
        nextScene();
    });

    // Start the window's main loop
    window.run();
}

// Sphere Scene Factory: Creates a pulsating sphere of cubes with color based on position.
SceneCreateInfo SphereFactory()
{
    SceneCreateInfo info{
        .name = "SphereScene",
        .cameraPosition = glm::vec3(0, 0, SPHERE_RADIUS * 2.5f), // Position camera to view the sphere
        .cameraDirection = glm::normalize(glm::vec3(0, 0, -1)), // Look towards the center
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);

            // Set a dark clear color for contrast
            scene.clearColor = {0.05f, 0.05f, 0.1f, 1.0f};

            // Store cube IDs and initial spherical coordinates
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_SPHERE_CUBES);
            auto& initialSphericalCoords = scene.template make<std::vector<glm::vec2>>("InitialSphericalCoords");
            initialSphericalCoords.reserve(NUM_SPHERE_CUBES);

            // Create cubes arranged on a sphere using Fibonacci sphere distribution
            for (int i = 0; i < NUM_SPHERE_CUBES; ++i)
            {
                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Sphere Cube";
                usingCubeCreateInfo.scale = {SPHERE_CUBE_INITIAL_SCALE, SPHERE_CUBE_INITIAL_SCALE, SPHERE_CUBE_INITIAL_SCALE};

                // Calculate spherical coordinates
                float phi = glm::acos(-1.0f + (2.0f * i) / (NUM_SPHERE_CUBES - 1)); // Latitude (0 to pi)
                float theta = glm::pi<float>() * (1.0f + glm::sqrt(5.0f)) * i; // Longitude (0 to 2*pi, using golden angle)

                // Store initial spherical coordinates
                initialSphericalCoords.push_back(glm::vec2(phi, theta));

                // Convert spherical to Cartesian coordinates
                float x = SPHERE_RADIUS * glm::sin(phi) * glm::cos(theta);
                float y = SPHERE_RADIUS * glm::cos(phi);
                float z = SPHERE_RADIUS * glm::sin(phi) * glm::sin(theta);

                usingCubeCreateInfo.position = glm::vec3(x, y, z);

                // Calculate color based on spherical coordinates (e.g., hue based on longitude, saturation based on latitude)
                float hue = glm::mod(theta / (2.0f * glm::pi<float>()), 1.0f); // Map longitude to hue (0 to 1)
                float saturation = 1.0f - glm::abs(glm::cos(phi)); // Map latitude to saturation (0 to 1)
                float lightness = 0.5f; // Keep lightness moderate

                glm::vec3 color = hslToRgb(hue, saturation, lightness);

                usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                    return { glm::vec4(color, 1.0f), 0 };
                };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Add the 'q' key press handler to close the window
            scene.template setData<zg::UniqueIdentifier>(
                "qPressID",
                window.addKeyPressHandler('q',
                    [INDEX_STACK = scene.INDEX_STACK](auto pressed)
                    {
                        if (pressed)
                        {
                            auto& window = Registry::getWindow(INDEX_STACK);
                            window.close();
                        }
                    }
                )
            );

            // Attach scene components for camera control and effects
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f); // Counter for animation time
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
            // Remove the 'q' key press handler when the scene is detached
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialSphericalCoords = scene.template getData<std::vector<glm::vec2>>("InitialSphericalCoords");
            auto cubesSize = cubes.size();

            // Parameters for the animation
            float pulse_speed = 1.0f; // Speed of the sphere pulsation
            float pulse_amplitude = 10.0f; // Amplitude of the sphere pulsation
            float wave_speed = 5.0f; // Speed of the wave on the surface
            float wave_frequency = 0.2f; // Frequency of the wave
            float wave_amplitude = 5.0f; // Amplitude of the surface wave
            float scale_pulse_speed = 2.0f; // Speed of the scale pulsation
            float scale_min = 0.5f; // Minimum scale factor
            float scale_max = 1.5f; // Maximum scale factor

            // Calculate the current pulsating radius
            float current_radius = SPHERE_RADIUS + pulse_amplitude * glm::sin(deltaTimeCounter * pulse_speed);

            // Update cube positions and scales
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::getEntity(cubeID);
                const auto& initialCoords = initialSphericalCoords[i]; // Initial (phi, theta)

                float phi = initialCoords.x;
                float theta = initialCoords.y;

                // Calculate perturbation for the surface wave
                float surface_perturbation = wave_amplitude * glm::sin(theta * wave_frequency + deltaTimeCounter * wave_speed);

                // Calculate the new position on the pulsating sphere with surface waves
                float current_sphere_radius = current_radius + surface_perturbation;

                float x = current_sphere_radius * glm::sin(phi) * glm::cos(theta);
                float y = current_sphere_radius * glm::cos(phi);
                float z = current_sphere_radius * glm::sin(phi) * glm::sin(theta);

                cube.position = glm::vec3(x, y, z);

                // Calculate scale factor based on a different time/position function
                float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + phi) * 0.5f + 0.5f));
                cube.scale = glm::vec3(SPHERE_CUBE_INITIAL_SCALE * scale_factor);
            }
        }
    };
    return info;
}

// Cube Grid Scene Factory: Creates a grid of cubes with a wave animation.
SceneCreateInfo CubeGridFactory()
{
    SceneCreateInfo info{
        .name = "CubeGridScene",
        .cameraPosition = glm::vec3(GRID_SIZE * GRID_SPACING * 0.5f, GRID_SIZE * GRID_SPACING * 0.8f, GRID_SIZE * GRID_SPACING * 1.2f), // Position camera to view the grid
        .cameraDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -1.0f)), // Look towards the center of the grid
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.1f, 0.1f, 0.15f, 1.0f};

            // Store cube IDs and initial positions
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(GRID_SIZE * GRID_SIZE * GRID_SIZE);
            auto& initialPositions = scene.template make<std::vector<glm::vec3>>("InitialPositions");
            initialPositions.reserve(GRID_SIZE * GRID_SIZE * GRID_SIZE);

            // Create cubes in a grid
            for (int x = 0; x < GRID_SIZE; ++x) {
                for (int y = 0; y < GRID_SIZE; ++y) {
                    for (int z = 0; z < GRID_SIZE; ++z) {
                        auto usingCubeCreateInfo = baseCubeCreateInfo;
                        usingCubeCreateInfo.name = "Grid Cube";
                        usingCubeCreateInfo.scale = {GRID_CUBE_INITIAL_SCALE, GRID_CUBE_INITIAL_SCALE, GRID_CUBE_INITIAL_SCALE};

                        // Calculate initial position
                        glm::vec3 initialPos = glm::vec3(
                            (x - GRID_SIZE * 0.5f) * GRID_SPACING,
                            (y - GRID_SIZE * 0.5f) * GRID_SPACING,
                            (z - GRID_SIZE * 0.5f) * GRID_SPACING
                        );
                        usingCubeCreateInfo.position = initialPos;
                        initialPositions.push_back(initialPos);

                        // Calculate color based on position
                        glm::vec3 color = glm::vec3(
                            (float)x / (GRID_SIZE - 1),
                            (float)y / (GRID_SIZE - 1),
                            (float)z / (GRID_SIZE - 1)
                        );

                         usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                            return { glm::vec4(color, 1.0f), 0 };
                        };

                        // Add the cube and store its ID
                        cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
                    }
                }
            }

            // Add the 'q' key press handler
            scene.template setData<zg::UniqueIdentifier>(
                "qPressID",
                window.addKeyPressHandler('q',
                    [INDEX_STACK = scene.INDEX_STACK](auto pressed)
                    {
                        if (pressed)
                        {
                            auto& window = Registry::getWindow(INDEX_STACK);
                            window.close();
                        }
                    }
                )
            );

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
            // Remove the 'q' key press handler
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialPositions = scene.template getData<std::vector<glm::vec3>>("InitialPositions");
            auto cubesSize = cubes.size();

            // Animation parameters
            float wave_speed = 3.0f;
            float wave_frequency = 0.1f;
            float wave_amplitude = 5.0f;

            // Update cube positions with a wave effect
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::getEntity(cubeID);
                const auto& initialPos = initialPositions[i];

                // Calculate wave offset based on initial position and time
                float wave_offset = wave_amplitude * glm::sin((initialPos.x + initialPos.y + initialPos.z) * wave_frequency + deltaTimeCounter * wave_speed);

                // Apply the wave offset along the Y-axis
                cube.position = initialPos + glm::vec3(0.0f, wave_offset, 0.0f);
            }
        }
    };
    return info;
}

// Random Cubes Scene Factory: Creates cubes at random positions with random scaling and rotation animation.
SceneCreateInfo RandomCubesFactory()
{
    SceneCreateInfo info{
        .name = "RandomCubesScene",
        .cameraPosition = glm::vec3(RANDOM_BOUNDING_BOX_SIZE * 1.5f, RANDOM_BOUNDING_BOX_SIZE * 1.5f, RANDOM_BOUNDING_BOX_SIZE * 1.5f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)), // Look towards the center
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.15f, 0.1f, 0.1f, 1.0f};

            // Store cube IDs and initial random rotations
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_RANDOM_CUBES);
            auto& initialRotations = scene.template make<std::vector<glm::vec3>>("InitialRotations");
            initialRotations.reserve(NUM_RANDOM_CUBES);

            // Create cubes at random positions
            for (int i = 0; i < NUM_RANDOM_CUBES; ++i)
            {
                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Random Cube";
                usingCubeCreateInfo.scale = {RANDOM_CUBE_INITIAL_SCALE, RANDOM_CUBE_INITIAL_SCALE, RANDOM_CUBE_INITIAL_SCALE};

                // Generate random position within the bounding box
                usingCubeCreateInfo.position = glm::vec3(
                    zg::crypto::Random::value<float>(-RANDOM_BOUNDING_BOX_SIZE * 0.5f, RANDOM_BOUNDING_BOX_SIZE * 0.5f),
                    zg::crypto::Random::value<float>(-RANDOM_BOUNDING_BOX_SIZE * 0.5f, RANDOM_BOUNDING_BOX_SIZE * 0.5f),
                    zg::crypto::Random::value<float>(-RANDOM_BOUNDING_BOX_SIZE * 0.5f, RANDOM_BOUNDING_BOX_SIZE * 0.5f)
                );

                // Generate random initial rotation
                glm::vec3 initialRot = glm::vec3(
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>()),
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>()),
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>())
                );
                initialRotations.push_back(initialRot);

                // Set a random color
                glm::vec3 color = glm::vec3(
                    zg::crypto::Random::value<float>(0.2f, 1.0f),
                    zg::crypto::Random::value<float>(0.2f, 1.0f),
                    zg::crypto::Random::value<float>(0.2f, 1.0f)
                );

                 usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                    return { glm::vec4(color, 1.0f), 0 };
                };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Add the 'q' key press handler
            scene.template setData<zg::UniqueIdentifier>(
                "qPressID",
                window.addKeyPressHandler('q',
                    [INDEX_STACK = scene.INDEX_STACK](auto pressed)
                    {
                        if (pressed)
                        {
                            auto& window = Registry::getWindow(INDEX_STACK);
                            window.close();
                        }
                    }
                )
            );

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
            // Remove the 'q' key press handler
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialRotations = scene.template getData<std::vector<glm::vec3>>("InitialRotations");
            auto cubesSize = cubes.size();

            // Animation parameters
            float rotation_speed = 0.5f; // Speed of rotation

            // Update cube rotations
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::getEntity(cubeID);
                const auto& initialRot = initialRotations[i];

                // Apply a continuous rotation based on initial random rotation and time
                cube.rotation = glm::quat(glm::vec3(
                    initialRot.x + deltaTimeCounter * rotation_speed,
                    initialRot.y + deltaTimeCounter * rotation_speed * 0.5f, // Different speed for y-axis
                    initialRot.z + deltaTimeCounter * rotation_speed * 0.75f // Different speed for z-axis
                ));
            }
        }
    };
    return info;
}

// Spiral Scene Factory: Creates cubes arranged along a spiral with a pulsing scale animation.
SceneCreateInfo SpiralFactory()
{
    SceneCreateInfo info{
        .name = "SpiralScene",
        .cameraPosition = glm::vec3(SPIRAL_RADIUS_END * 2.0f, SPIRAL_HEIGHT * 0.5f, SPIRAL_RADIUS_END * 2.0f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-1.0f, -0.2f, -1.0f)), // Look towards the center of the spiral
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.1f, 0.15f, 0.1f, 1.0f};

            // Store cube IDs
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_SPIRAL_CUBES);

            // Create cubes along a spiral
            for (int i = 0; i < NUM_SPIRAL_CUBES; ++i)
            {
                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Spiral Cube";
                usingCubeCreateInfo.scale = {SPIRAL_CUBE_INITIAL_SCALE, SPIRAL_CUBE_INITIAL_SCALE, SPIRAL_CUBE_INITIAL_SCALE};

                // Calculate position along the spiral
                float t = (float)i / (NUM_SPIRAL_CUBES - 1); // Parameter from 0 to 1
                float radius = glm::mix(SPIRAL_RADIUS_START, SPIRAL_RADIUS_END, t);
                float angle = t * SPIRAL_TURNS * glm::two_pi<float>();
                float y = t * SPIRAL_HEIGHT - SPIRAL_HEIGHT * 0.5f; // Center the spiral vertically

                glm::vec3 position = glm::vec3(
                    radius * glm::cos(angle),
                    y,
                    radius * glm::sin(angle)
                );
                usingCubeCreateInfo.position = position;

                // Calculate color based on position along the spiral
                float hue = glm::mod(t * 2.0f, 1.0f); // Cycle through hues along the spiral
                float saturation = 1.0f;
                float lightness = 0.5f;
                 glm::vec3 color = hslToRgb(hue, saturation, lightness);

                 usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                    return { glm::vec4(color, 1.0f), 0 };
                };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Add the 'q' key press handler
            scene.template setData<zg::UniqueIdentifier>(
                "qPressID",
                window.addKeyPressHandler('q',
                    [INDEX_STACK = scene.INDEX_STACK](auto pressed)
                    {
                        if (pressed)
                        {
                            auto& window = Registry::getWindow(INDEX_STACK);
                            window.close();
                        }
                    }
                )
            );

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
            // Remove the 'q' key press handler
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto cubesSize = cubes.size();

            // Animation parameters
            float scale_pulse_speed = 4.0f;
            float scale_min = 0.7f;
            float scale_max = 1.3f;

            // Update cube scales with a pulsing effect based on their position in the spiral
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::getEntity(cubeID);

                // Calculate scale factor based on index and time
                float t = (float)i / (NUM_SPIRAL_CUBES - 1);
                float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + t * glm::two_pi<float>()) * 0.5f + 0.5f));

                cube.scale = glm::vec3(SPIRAL_CUBE_INITIAL_SCALE * scale_factor);
            }
        }
    };
    return info;
}
