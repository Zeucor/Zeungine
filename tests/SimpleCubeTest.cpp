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
#include <numeric> // Required for std::iota
#include <zg/shaders/ShaderFactory.hpp>

using namespace zg;
using namespace zg::shaders;

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

// Constants for WaveformGridFactory
const int WAVE_GRID_SIZE = 20; // Number of cubes along X and Z axes for the waveform grid
const float WAVE_GRID_SPACING = 8.0f; // Spacing between cubes in the waveform grid
const float WAVE_CUBE_INITIAL_SCALE = 2.5f; // Initial uniform scale for waveform cubes

// Constants for RingFactory
const int NUM_RING_CUBES = 200; // Number of cubes in the ring
const float RING_INITIAL_RADIUS = 50.0f; // Initial radius of the ring
const float RING_EXPANSION_AMPLITUDE = 20.0f; // Amplitude of ring expansion
const float RING_EXPANSION_SPEED = 0.8f; // Speed of ring expansion
const float RING_CUBE_INITIAL_SCALE = 3.0f; // Initial uniform scale for ring cubes

// Constants for FountainFactory
const int NUM_FOUNTAIN_PARTICLES = 500; // Number of cubes in the fountain
const float FOUNTAIN_EMISSION_RATE = 100.0f; // Cubes emitted per second
const float FOUNTAIN_PARTICLE_LIFETIME = 5.0f; // Lifetime of each cube in seconds
const float FOUNTAIN_INITIAL_VELOCITY_Y = 30.0f; // Initial upward velocity
const float FOUNTAIN_SPREAD_ANGLE = glm::radians(30.0f); // Angle of spread
const float FOUNTAIN_GRAVITY = -20.0f; // Gravity effect
const float FOUNTAIN_CUBE_INITIAL_SCALE = 1.5f; // Initial uniform scale for fountain cubes

// Constants for DNAHelixFactory
const int NUM_DNA_PAIRS = 100; // Number of base pairs (two cubes per pair)
const float DNA_RADIUS = 15.0f; // Radius of the helix
const float DNA_HEIGHT = 10.f;
const float DNA_PITCH = 10.0f; // Vertical distance per full turn
const float DNA_CUBE_INITIAL_SCALE = 2.0f; // Initial uniform scale for DNA cubes

// Constants for SwarmFactory
const int NUM_SWARM_CUBES = 600; // Number of cubes in the swarm
const float SWARM_BOUNDING_BOX_SIZE = 80.0f; // Size of the swarm volume
const float SWARM_MOVEMENT_SPEED = 5.0f; // Speed of swarm movement
const float SWARM_RANDOMNESS = 0.5f; // Factor for random movement
const float SWARM_CUBE_INITIAL_SCALE = 2.0f; // Initial uniform scale for swarm cubes

// Constants for TorusFactory
const int NUM_TORUS_SEGMENTS = 40; // Number of segments around the main ring
const int NUM_TORUS_TUBES = 20; // Number of cubes in each tube segment
const float TORUS_MAJOR_RADIUS = 40.0f; // Radius of the main ring
const float TORUS_MINOR_RADIUS = 15.0f; // Radius of the tube
const float TORUS_ROTATION_SPEED = 0.3f; // Speed of torus rotation
const float TORUS_CUBE_INITIAL_SCALE = 2.0f; // Initial uniform scale for torus cubes

// Constants for FallingCubesFactory
const int NUM_FALLING_CUBES = 300; // Maximum number of falling cubes
const float FALLING_CUBE_SPAWN_RATE = 50.0f; // Cubes spawned per second
const float FALLING_CUBE_SPAWN_AREA_SIZE = 80.0f; // Size of the spawn area
const float FALLING_CUBE_INITIAL_HEIGHT = 100.0f; // Initial height
const float FALLING_GRAVITY = -30.0f; // Gravity effect
const float FALLING_CUBE_INITIAL_SCALE = 2.0f; // Initial uniform scale for falling cubes
const float FALLING_CUBE_DESPAWN_HEIGHT = -50.0f; // Height at which cubes despawn

// Base cube create info (can be customized per factory)
auto baseCubeCreateInfo = entities::CubeFactory("Base Cube", {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});

// Function declarations for scene factories
SceneCreateInfo SphereFactory();
SceneCreateInfo CubeGridFactory();
SceneCreateInfo RandomCubesFactory();
SceneCreateInfo SpiralFactory();
SceneCreateInfo WaveformGridFactory();
SceneCreateInfo RingFactory();
SceneCreateInfo FountainFactory();
SceneCreateInfo DNAHelixFactory();
SceneCreateInfo SwarmFactory();
SceneCreateInfo TorusFactory();
SceneCreateInfo FallingCubesFactory();


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
    Registry registry;
	ShaderFactory shader_factory;
	register_zg_shader_hooks();
    // Window creation info
    WindowCreateInfo windowCreateInfo{.title = "Beautiful 3D Scene Factories Test", .borderless = true, .vsync = false, .framerate = 144};
    auto window_tuple = zg::Registry::GetSingleton().addWindow(windowCreateInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);

    size_t current_scene_id = 0; // Initialize with 0
    size_t scene_factory_index = 0;

    // Vector of scene factory functions
    std::vector<std::function<SceneCreateInfo()>> scene_factories = {
        SphereFactory,
        CubeGridFactory,
        RandomCubesFactory,
        SpiralFactory,
        WaveformGridFactory,
        RingFactory,
        FountainFactory,
        DNAHelixFactory,
        SwarmFactory,
        TorusFactory,
        FallingCubesFactory
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
    
    window.addKeyPressHandler('q',
        [&](auto pressed)
        {
            if (pressed)
            {
                window.close();
            }
        }
    );

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
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

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

                // usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Attach scene components for camera control and effects
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f); // Counter for animation time
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

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
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
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
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

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

                        //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                        //     return { glm::vec4(color, 1.0f), 0 };
                        // };

                        // Add the cube and store its ID
                        cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
                    }
                }
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

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
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
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
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

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

                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialRotations = scene.template getData<std::vector<glm::vec3>>("InitialRotations");
            auto cubesSize = cubes.size();

            // Animation parameters
            float rotation_speed = 0.5f; // Speed of rotation

            // Update cube rotations
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
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
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

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

                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

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
                auto& cube = Registry::GetSingleton().getEntity(cubeID);

                // Calculate scale factor based on index and time
                float t = (float)i / (NUM_SPIRAL_CUBES - 1);
                float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + t * glm::two_pi<float>()) * 0.5f + 0.5f));

                cube.scale = glm::vec3(SPIRAL_CUBE_INITIAL_SCALE * scale_factor);
            }
        }
    };
    return info;
}

// Waveform Grid Scene Factory: Creates a grid of cubes with a wave propagating across the grid, affecting height and color.
SceneCreateInfo WaveformGridFactory()
{
    SceneCreateInfo info{
        .name = "WaveformGridScene",
        .cameraPosition = glm::vec3(WAVE_GRID_SIZE * WAVE_GRID_SPACING * 0.5f, WAVE_GRID_SIZE * WAVE_GRID_SPACING * 0.8f, WAVE_GRID_SIZE * WAVE_GRID_SPACING * 1.2f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -1.0f)), // Look towards the center
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.1f, 0.1f, 0.2f, 1.0f};

            // Store cube IDs and initial positions
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(WAVE_GRID_SIZE * WAVE_GRID_SIZE);
            auto& initialPositions = scene.template make<std::vector<glm::vec2>>("InitialGridPositions"); // Store XZ
            initialPositions.reserve(WAVE_GRID_SIZE * WAVE_GRID_SIZE);

            // Create cubes in a grid on the XZ plane
            for (int x = 0; x < WAVE_GRID_SIZE; ++x) {
                for (int z = 0; z < WAVE_GRID_SIZE; ++z) {
                    auto usingCubeCreateInfo = baseCubeCreateInfo;
                    usingCubeCreateInfo.name = "Waveform Cube";
                    usingCubeCreateInfo.scale = {WAVE_CUBE_INITIAL_SCALE, WAVE_CUBE_INITIAL_SCALE, WAVE_CUBE_INITIAL_SCALE};

                    // Calculate initial position (XZ plane)
                    glm::vec2 initialPosXZ = glm::vec2(
                        (x - WAVE_GRID_SIZE * 0.5f) * WAVE_GRID_SPACING,
                        (z - WAVE_GRID_SIZE * 0.5f) * WAVE_GRID_SPACING
                    );
                    usingCubeCreateInfo.position = glm::vec3(initialPosXZ.x, 0.0f, initialPosXZ.y); // Y is initially 0
                    initialPositions.push_back(initialPosXZ);

                    // Initial color (will be animated)
                    //  usingCubeCreateInfo.meshInfos[0].material = [](auto&) -> Material {
                    //     return { glm::vec4(1.0f), 0 }; // Start with white
                    // };

                    // Add the cube and store its ID
                    cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
                }
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialPositions = scene.template getData<std::vector<glm::vec2>>("InitialGridPositions");
            auto cubesSize = cubes.size();

            // Animation parameters
            float wave_speed = 4.0f;
            float wave_frequency = 0.05f;
            float wave_amplitude = 15.0f;
            float color_speed = 1.0f;

            // Update cube positions (Y-axis) and colors based on a wave
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
                const auto& initialPosXZ = initialPositions[i];

                // Calculate wave offset based on initial position and time
                float wave_offset = wave_amplitude * glm::sin((initialPosXZ.x + initialPosXZ.y) * wave_frequency + deltaTimeCounter * wave_speed);

                // Apply the wave offset along the Y-axis
                cube.position.y = wave_offset;

                // Calculate color based on the wave offset and time
                float color_hue = glm::mod((wave_offset / wave_amplitude) * 0.5f + deltaTimeCounter * color_speed, 1.0f);
                glm::vec3 color = hslToRgb(color_hue, 1.0f, 0.5f);

                // Update material color
                // cube.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };
            }
        }
    };
    return info;
}

// Expanding/Contracting Ring Scene Factory: Cubes in a ring that expands and contracts, with individual cubes rotating.
SceneCreateInfo RingFactory()
{
    SceneCreateInfo info{
        .name = "RingScene",
        .cameraPosition = glm::vec3(0, RING_INITIAL_RADIUS * 1.5f, RING_INITIAL_RADIUS * 2.5f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(0, -0.5f, -1.0f)), // Look towards the center
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.2f, 0.1f, 0.1f, 1.0f};

            // Store cube IDs and initial angles
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_RING_CUBES);
            auto& initialAngles = scene.template make<std::vector<float>>("InitialAngles");
            initialAngles.reserve(NUM_RING_CUBES);
             auto& initialRotations = scene.template make<std::vector<glm::vec3>>("InitialRotations");
            initialRotations.reserve(NUM_RING_CUBES);


            // Create cubes in a ring
            for (int i = 0; i < NUM_RING_CUBES; ++i)
            {
                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Ring Cube";
                usingCubeCreateInfo.scale = {RING_CUBE_INITIAL_SCALE, RING_CUBE_INITIAL_SCALE, RING_CUBE_INITIAL_SCALE};

                // Calculate initial angle
                float angle = (float)i / NUM_RING_CUBES * glm::two_pi<float>();
                initialAngles.push_back(angle);

                // Calculate initial position on the ring
                glm::vec3 position = glm::vec3(
                    RING_INITIAL_RADIUS * glm::cos(angle),
                    0.0f, // Flat ring on XZ plane
                    RING_INITIAL_RADIUS * glm::sin(angle)
                );
                usingCubeCreateInfo.position = position;

                 // Generate random initial rotation
                glm::vec3 initialRot = glm::vec3(
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>()),
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>()),
                    zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>())
                );
                initialRotations.push_back(initialRot);

                // Calculate color based on angle
                float hue = glm::mod(angle / glm::two_pi<float>(), 1.0f);
                glm::vec3 color = hslToRgb(hue, 1.0f, 0.6f);

                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialAngles = scene.template getData<std::vector<float>>("InitialAngles");
             auto& initialRotations = scene.template getData<std::vector<glm::vec3>>("InitialRotations");
            auto cubesSize = cubes.size();

            // Calculate current radius based on pulsation
            float current_radius = RING_INITIAL_RADIUS + RING_EXPANSION_AMPLITUDE * glm::sin(deltaTimeCounter * RING_EXPANSION_SPEED);

            // Update cube positions and rotations
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
                const auto& initialAngle = initialAngles[i];
                 const auto& initialRot = initialRotations[i];

                // Update position on the expanding/contracting ring
                cube.position.x = current_radius * glm::cos(initialAngle);
                cube.position.z = current_radius * glm::sin(initialAngle);

                 // Apply a continuous rotation
                cube.rotation = glm::quat(glm::vec3(
                    initialRot.x + deltaTimeCounter * 1.0f,
                    initialRot.y + deltaTimeCounter * 0.8f,
                    initialRot.z + deltaTimeCounter * 1.2f
                ));
            }
        }
    };
    return info;
}

// Particle Fountain Scene Factory: Cubes originating from a point and moving upwards, spreading out, and fading.
SceneCreateInfo FountainFactory()
{
    struct FountainParticle {
        size_t entityID;
        glm::vec3 velocity;
        float lifetime;
    };

    SceneCreateInfo info{
        .name = "FountainScene",
        .cameraPosition = glm::vec3(0, FOUNTAIN_INITIAL_VELOCITY_Y * 2.0f, FOUNTAIN_INITIAL_VELOCITY_Y * 3.0f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(0, -0.5f, -1.0f)), // Look towards the fountain source
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.08f, 0.1f, 0.08f, 1.0f};

            // Store particle data
            scene.template make<std::vector<FountainParticle>>("Particles");
            scene.template make<float>("emissionTimer", 0.f); // Timer for emitting new particles

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& particles = scene.template getData<std::vector<FountainParticle>>("Particles");
            auto& emissionTimer = scene.template getData<float>("emissionTimer");

            // Emit new particles based on emission rate
            emissionTimer += *window.deltaTime;
            while (emissionTimer >= 1.0f / FOUNTAIN_EMISSION_RATE && particles.size() < NUM_FOUNTAIN_PARTICLES) {
                emissionTimer -= 1.0f / FOUNTAIN_EMISSION_RATE;

                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Fountain Particle";
                usingCubeCreateInfo.scale = {FOUNTAIN_CUBE_INITIAL_SCALE, FOUNTAIN_CUBE_INITIAL_SCALE, FOUNTAIN_CUBE_INITIAL_SCALE};
                usingCubeCreateInfo.position = {0.0f, 0.0f, 0.0f}; // Start at the origin

                // Generate random velocity within the spread angle
                float horizontalAngle = zg::crypto::Random::value<float>(0.0f, glm::two_pi<float>());
                float verticalAngle = zg::crypto::Random::value<float>(0.0f, FOUNTAIN_SPREAD_ANGLE);

                glm::vec3 velocity = glm::vec3(
                    FOUNTAIN_INITIAL_VELOCITY_Y * glm::sin(verticalAngle) * glm::cos(horizontalAngle),
                    FOUNTAIN_INITIAL_VELOCITY_Y * glm::cos(verticalAngle),
                    FOUNTAIN_INITIAL_VELOCITY_Y * glm::sin(verticalAngle) * glm::sin(horizontalAngle)
                );

                // Set a random color
                glm::vec3 color = glm::vec3(
                    zg::crypto::Random::value<float>(0.5f, 1.0f),
                    zg::crypto::Random::value<float>(0.5f, 1.0f),
                    zg::crypto::Random::value<float>(0.5f, 1.0f)
                );
                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store particle data
                size_t entityID = std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo));
                particles.push_back({entityID, velocity, 0.0f});
            }

            // Update particle positions and lifetimes
            for (auto it = particles.begin(); it != particles.end(); ) {
                auto& particle = *it;
                auto& cube = Registry::GetSingleton().getEntity(particle.entityID);

                // Apply gravity to velocity
                particle.velocity.y += FOUNTAIN_GRAVITY * *window.deltaTime;

                // Update position
                cube.position += particle.velocity * (float)*window.deltaTime;

                // Update lifetime
                particle.lifetime += *window.deltaTime;

                // Remove particle if lifetime exceeded
                if (particle.lifetime >= FOUNTAIN_PARTICLE_LIFETIME) {
                    scene.removeEntity(particle.entityID);
                    it = particles.erase(it);
                } else {
                    // Update color based on lifetime (fade out)
                    float alpha = 1.0f - (particle.lifetime / FOUNTAIN_PARTICLE_LIFETIME);
                    //  glm::vec4 color = cube.meshInfos[0].material(cube).albedo; // Get current color
                    //  color.a = alpha; // Update alpha

                    //  cube.meshInfos[0].material = [color](auto&) -> Material {
                    //     return { color, 0 };
                    // };
                    ++it;
                }
            }
        }
    };
    return info;
}

// DNA Helix Scene Factory: Cubes arranged along a double helix structure, with rotation and pulsing scale.
SceneCreateInfo DNAHelixFactory()
{
    SceneCreateInfo info{
        .name = "DNAHelixScene",
        .cameraPosition = glm::vec3(DNA_RADIUS * 4.0f, DNA_HEIGHT * 0.5f, DNA_RADIUS * 4.0f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-1.0f, -0.2f, -1.0f)), // Look towards the center of the helix
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.12f, 0.08f, 0.12f, 1.0f};

            // Store cube IDs and initial helix parameters (angle, height)
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_DNA_PAIRS * 2);
            auto& initialHelixParams = scene.template make<std::vector<glm::vec2>>("InitialHelixParams"); // x: angle, y: height
            initialHelixParams.reserve(NUM_DNA_PAIRS * 2);

            // Create cubes for the double helix
            for (int i = 0; i < NUM_DNA_PAIRS; ++i)
            {
                // Base pair position along the height
                float t = (float)i / (NUM_DNA_PAIRS - 1); // Parameter from 0 to 1
                float height = t * DNA_HEIGHT - DNA_HEIGHT * 0.5f; // Center the helix vertically

                // Angle for the first strand
                float angle1 = t * SPIRAL_TURNS * glm::two_pi<float>();

                // Angle for the second strand (offset by pi)
                float angle2 = angle1 + glm::pi<float>();

                // Create cube for the first strand
                auto usingCubeCreateInfo1 = baseCubeCreateInfo;
                usingCubeCreateInfo1.name = "DNA Cube 1";
                usingCubeCreateInfo1.scale = {DNA_CUBE_INITIAL_SCALE, DNA_CUBE_INITIAL_SCALE, DNA_CUBE_INITIAL_SCALE};
                usingCubeCreateInfo1.position = glm::vec3(
                    DNA_RADIUS * glm::cos(angle1),
                    height,
                    DNA_RADIUS * glm::sin(angle1)
                );
                initialHelixParams.push_back(glm::vec2(angle1, height));

                // Set color for the first strand (e.g., based on height)
                float color_hue1 = glm::mod(t, 1.0f);
                glm::vec3 color1 = hslToRgb(color_hue1, 1.0f, 0.6f);
                //  usingCubeCreateInfo1.meshInfos[0].material = [color1](auto&) -> Material {
                //     return { glm::vec4(color1, 1.0f), 0 };
                // };
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo1)));

                // Create cube for the second strand
                auto usingCubeCreateInfo2 = baseCubeCreateInfo;
                usingCubeCreateInfo2.name = "DNA Cube 2";
                usingCubeCreateInfo2.scale = {DNA_CUBE_INITIAL_SCALE, DNA_CUBE_INITIAL_SCALE, DNA_CUBE_INITIAL_SCALE};
                usingCubeCreateInfo2.position = glm::vec3(
                    DNA_RADIUS * glm::cos(angle2),
                    height,
                    DNA_RADIUS * glm::sin(angle2)
                );
                 initialHelixParams.push_back(glm::vec2(angle2, height));

                // Set color for the second strand (e.g., complementary color or different scheme)
                 float color_hue2 = glm::mod(t + 0.5f, 1.0f); // Offset hue
                 glm::vec3 color2 = hslToRgb(color_hue2, 1.0f, 0.6f);
                //  usingCubeCreateInfo2.meshInfos[0].material = [color2](auto&) -> Material {
                //     return { glm::vec4(color2, 1.0f), 0 };
                // };
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo2)));
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialHelixParams = scene.template getData<std::vector<glm::vec2>>("InitialHelixParams");
            auto cubesSize = cubes.size();

            // Animation parameters
            float rotation_speed = 0.5f; // Speed of helix rotation
            float scale_pulse_speed = 3.0f; // Speed of scale pulsation
            float scale_min = 0.8f; // Minimum scale factor
            float scale_max = 1.2f; // Maximum scale factor

            // Update cube positions (rotation around the helix axis) and scales
            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
                const auto& initialParams = initialHelixParams[i]; // x: initial angle, y: initial height

                float current_angle = initialParams.x + deltaTimeCounter * rotation_speed;
                float height = initialParams.y;

                // Update position based on current angle
                cube.position.x = DNA_RADIUS * glm::cos(current_angle);
                cube.position.y = height;
                cube.position.z = DNA_RADIUS * glm::sin(current_angle);

                // Calculate scale factor based on height and time
                float t = (height + DNA_HEIGHT * 0.5f) / DNA_HEIGHT; // Parameter from 0 to 1 based on height
                 float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + t * glm::two_pi<float>()) * 0.5f + 0.5f));
                 cube.scale = glm::vec3(DNA_CUBE_INITIAL_SCALE * scale_factor);

                 // Optional: Orient cubes to face outwards from the helix axis
                 glm::vec3 direction_from_axis = glm::normalize(glm::vec3(cube.position.x, 0, cube.position.z));
                 glm::vec3 up_vector = glm::vec3(0, 1, 0);
                 cube.rotation = glm::quatLookAt(direction_from_axis, up_vector);
            }
        }
    };
    return info;
}

// Swarming Cubes Scene Factory: Cubes moving somewhat randomly but staying within a defined volume, with subtle color changes.
SceneCreateInfo SwarmFactory()
{
    struct SwarmCube {
        size_t entityID;
        glm::vec3 velocity;
    };

    SceneCreateInfo info{
        .name = "SwarmScene",
        .cameraPosition = glm::vec3(SWARM_BOUNDING_BOX_SIZE * 1.5f, SWARM_BOUNDING_BOX_SIZE * 1.0f, SWARM_BOUNDING_BOX_SIZE * 1.5f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-1.0f, -0.5f, -1.0f)), // Look towards the center of the swarm
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.05f, 0.08f, 0.08f, 1.0f};

            // Store swarm cube data
            auto& swarmCubes = scene.template make<std::vector<SwarmCube>>("SwarmCubes");
            swarmCubes.reserve(NUM_SWARM_CUBES);

            // Create swarm cubes at random initial positions and velocities
            for (int i = 0; i < NUM_SWARM_CUBES; ++i)
            {
                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Swarm Cube";
                usingCubeCreateInfo.scale = {SWARM_CUBE_INITIAL_SCALE, SWARM_CUBE_INITIAL_SCALE, SWARM_CUBE_INITIAL_SCALE};

                // Random initial position within the bounding box
                usingCubeCreateInfo.position = glm::vec3(
                    zg::crypto::Random::value<float>(-SWARM_BOUNDING_BOX_SIZE * 0.5f, SWARM_BOUNDING_BOX_SIZE * 0.5f),
                    zg::crypto::Random::value<float>(-SWARM_BOUNDING_BOX_SIZE * 0.5f, SWARM_BOUNDING_BOX_SIZE * 0.5f),
                    zg::crypto::Random::value<float>(-SWARM_BOUNDING_BOX_SIZE * 0.5f, SWARM_BOUNDING_BOX_SIZE * 0.5f)
                );

                // Random initial velocity
                glm::vec3 velocity = glm::vec3(
                    zg::crypto::Random::value<float>(-1.0f, 1.0f),
                    zg::crypto::Random::value<float>(-1.0f, 1.0f),
                    zg::crypto::Random::value<float>(-1.0f, 1.0f)
                ) * SWARM_MOVEMENT_SPEED * 0.5f; // Start with slightly lower speed

                 // Set a random color
                glm::vec3 color = glm::vec3(
                    zg::crypto::Random::value<float>(0.6f, 1.0f),
                    zg::crypto::Random::value<float>(0.6f, 1.0f),
                    zg::crypto::Random::value<float>(0.6f, 1.0f)
                );
                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };


                // Add the cube and store swarm data
                size_t entityID = std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo));
                swarmCubes.push_back({entityID, velocity});
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& swarmCubes = scene.template getData<std::vector<SwarmCube>>("SwarmCubes");
            float halfBoxSize = SWARM_BOUNDING_BOX_SIZE * 0.5f;

            // Update swarm cube positions and velocities
            for (auto& swarmCube : swarmCubes)
            {
                auto& cube = Registry::GetSingleton().getEntity(swarmCube.entityID);

                // Add random influence to velocity
                swarmCube.velocity += glm::vec3(
                    zg::crypto::Random::value<float>(-1.0f, 1.0f),
                    zg::crypto::Random::value<float>(-1.0f, 1.0f),
                    zg::crypto::Random::value<float>(-1.0f, 1.0f)
                ) * SWARM_RANDOMNESS * (float)*window.deltaTime;

                // Normalize velocity and apply speed
                if (glm::length(swarmCube.velocity) > 0.0f) {
                    swarmCube.velocity = glm::normalize(swarmCube.velocity) * SWARM_MOVEMENT_SPEED;
                } else {
                     swarmCube.velocity = glm::vec3(
                        zg::crypto::Random::value<float>(-1.0f, 1.0f),
                        zg::crypto::Random::value<float>(-1.0f, 1.0f),
                        zg::crypto::Random::value<float>(-1.0f, 1.0f)
                    ) * SWARM_MOVEMENT_SPEED * 0.1f; // Give a small push if velocity is zero
                }


                // Keep cubes within the bounding box by reversing velocity if they hit a boundary
                if (cube.position.x > halfBoxSize || cube.position.x < -halfBoxSize) swarmCube.velocity.x *= -1.0f;
                if (cube.position.y > halfBoxSize || cube.position.y < -halfBoxSize) swarmCube.velocity.y *= -1.0f;
                if (cube.position.z > halfBoxSize || cube.position.z < -halfBoxSize) swarmCube.velocity.z *= -1.0f;

                // Update position
                cube.position += swarmCube.velocity * (float)*window.deltaTime;

                 // Subtle color change based on time or position
                 float color_factor = glm::sin(deltaTimeCounter * 0.5f + glm::length(cube.position) * 0.05f) * 0.1f + 0.9f;
                //  glm::vec4 currentColor = cube.meshInfos[0].material(cube).albedo;
                //  glm::vec3 baseColor = glm::vec3(currentColor);
                //  glm::vec3 newColor = baseColor * color_factor;
                //  cube.meshInfos[0].material = [newColor](auto&) -> Material {
                //     return { glm::vec4(newColor, 1.0f), 0 };
                // };
            }
        }
    };
    return info;
}

// Rotating Torus Scene Factory: Cubes arranged on the surface of a torus, with the torus rotating and cubes pulsing.
SceneCreateInfo TorusFactory()
{
    SceneCreateInfo info{
        .name = "TorusScene",
        .cameraPosition = glm::vec3(TORUS_MAJOR_RADIUS * 3.0f, TORUS_MAJOR_RADIUS * 1.5f, TORUS_MAJOR_RADIUS * 3.0f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-1.0f, -0.5f, -1.0f)), // Look towards the center of the torus
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.08f, 0.08f, 0.12f, 1.0f};

            // Store cube IDs and initial torus parameters (phi, theta)
            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_TORUS_SEGMENTS * NUM_TORUS_TUBES);
            auto& initialTorusParams = scene.template make<std::vector<glm::vec2>>("InitialTorusParams"); // x: phi, y: theta
            initialTorusParams.reserve(NUM_TORUS_SEGMENTS * NUM_TORUS_TUBES);

            // Create cubes on the surface of a torus
            for (int i = 0; i < NUM_TORUS_SEGMENTS; ++i) {
                float phi = (float)i / NUM_TORUS_SEGMENTS * glm::two_pi<float>(); // Angle around the main ring

                for (int j = 0; j < NUM_TORUS_TUBES; ++j) {
                    float theta = (float)j / NUM_TORUS_TUBES * glm::two_pi<float>(); // Angle around the tube

                    auto usingCubeCreateInfo = baseCubeCreateInfo;
                    usingCubeCreateInfo.name = "Torus Cube";
                    usingCubeCreateInfo.scale = {TORUS_CUBE_INITIAL_SCALE, TORUS_CUBE_INITIAL_SCALE, TORUS_CUBE_INITIAL_SCALE};

                    // Calculate initial position on the torus
                    glm::vec3 position = glm::vec3(
                        (TORUS_MAJOR_RADIUS + TORUS_MINOR_RADIUS * glm::cos(theta)) * glm::cos(phi),
                        TORUS_MINOR_RADIUS * glm::sin(theta),
                        (TORUS_MAJOR_RADIUS + TORUS_MINOR_RADIUS * glm::cos(theta)) * glm::sin(phi)
                    );
                    usingCubeCreateInfo.position = position;
                    initialTorusParams.push_back(glm::vec2(phi, theta));

                    // Calculate color based on position on the torus
                    float hue = glm::mod(phi / glm::two_pi<float>() + theta / glm::two_pi<float>(), 1.0f);
                    glm::vec3 color = hslToRgb(hue, 1.0f, 0.7f);
                    //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                    //     return { glm::vec4(color, 1.0f), 0 };
                    // };

                    // Add the cube and store its ID
                    cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
                }
            }

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
            auto& initialTorusParams = scene.template getData<std::vector<glm::vec2>>("InitialTorusParams");
            auto cubesSize = cubes.size();

            // Animation parameters
            float scale_pulse_speed = 5.0f;
            float scale_min = 0.6f;
            float scale_max = 1.4f;

            // Update cube positions (rotate the torus) and scales
            glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), deltaTimeCounter * TORUS_ROTATION_SPEED, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis

            for (size_t i = 0; i < cubesSize; ++i)
            {
                auto& cubeID = cubes[i];
                auto& cube = Registry::GetSingleton().getEntity(cubeID);
                const auto& initialParams = initialTorusParams[i]; // x: initial phi, y: initial theta

                // Calculate the initial position in the torus's local space
                 glm::vec3 local_position = glm::vec3(
                    (TORUS_MAJOR_RADIUS + TORUS_MINOR_RADIUS * glm::cos(initialParams.y)) * glm::cos(initialParams.x),
                    TORUS_MINOR_RADIUS * glm::sin(initialParams.y),
                    (TORUS_MAJOR_RADIUS + TORUS_MINOR_RADIUS * glm::cos(initialParams.y)) * glm::sin(initialParams.x)
                );

                // Apply the torus rotation
                cube.position = glm::vec3(rotation_matrix * glm::vec4(local_position, 1.0f));

                // Calculate scale factor based on initial theta and time
                float t = initialParams.y / glm::two_pi<float>(); // Parameter from 0 to 1 based on theta
                 float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + t * glm::two_pi<float>()) * 0.5f + 0.5f));
                 cube.scale = glm::vec3(TORUS_CUBE_INITIAL_SCALE * scale_factor);
            }
        }
    };
    return info;
}

// Falling Cubes Scene Factory: Cubes generated at the top of the scene and falling downwards, disappearing at the bottom.
SceneCreateInfo FallingCubesFactory()
{
    struct FallingCube {
        size_t entityID;
        glm::vec3 velocity;
    };

    SceneCreateInfo info{
        .name = "FallingCubesScene",
        .cameraPosition = glm::vec3(FALLING_CUBE_SPAWN_AREA_SIZE * 1.0f, FALLING_CUBE_INITIAL_HEIGHT * 0.8f, FALLING_CUBE_SPAWN_AREA_SIZE * 1.0f), // Position camera
        .cameraDirection = glm::normalize(glm::vec3(-0.5f, -0.8f, -0.5f)), // Look downwards
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);

            // Set a clear color
            scene.clearColor = {0.05f, 0.05f, 0.05f, 1.0f}; // Dark background

            // Store falling cube data
            scene.template make<std::vector<FallingCube>>("FallingCubes");
            scene.template make<float>("spawnTimer", 0.f); // Timer for spawning new cubes

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
            scene.attachComponent(zg::components::scenes::BloomFactory());
        },
        .onDetachedFunction = [](auto& scene)
        {
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& window = Registry::GetSingleton().getWindow(scene.INDEX_STACK);
            auto& deltaTimeCounter = scene.template getData<float>("deltaTimeCounter");
            deltaTimeCounter += *window.deltaTime;

            auto& fallingCubes = scene.template getData<std::vector<FallingCube>>("FallingCubes");
            auto& spawnTimer = scene.template getData<float>("spawnTimer");

            // Spawn new cubes based on spawn rate
            spawnTimer += *window.deltaTime;
            while (spawnTimer >= 1.0f / FALLING_CUBE_SPAWN_RATE && fallingCubes.size() < NUM_FALLING_CUBES) {
                spawnTimer -= 1.0f / FALLING_CUBE_SPAWN_RATE;

                auto usingCubeCreateInfo = baseCubeCreateInfo;
                usingCubeCreateInfo.name = "Falling Cube";
                usingCubeCreateInfo.scale = {FALLING_CUBE_INITIAL_SCALE, FALLING_CUBE_INITIAL_SCALE, FALLING_CUBE_INITIAL_SCALE};

                // Random initial position within the spawn area at the initial height
                usingCubeCreateInfo.position = glm::vec3(
                    zg::crypto::Random::value<float>(-FALLING_CUBE_SPAWN_AREA_SIZE * 0.5f, FALLING_CUBE_SPAWN_AREA_SIZE * 0.5f),
                    FALLING_CUBE_INITIAL_HEIGHT,
                    zg::crypto::Random::value<float>(-FALLING_CUBE_SPAWN_AREA_SIZE * 0.5f, FALLING_CUBE_SPAWN_AREA_SIZE * 0.5f)
                );

                // Initial downward velocity (can add some randomness)
                glm::vec3 velocity = glm::vec3(
                    zg::crypto::Random::value<float>(-5.0f, 5.0f),
                    zg::crypto::Random::value<float>(-10.0f, -5.0f),
                    zg::crypto::Random::value<float>(-5.0f, 5.0f)
                );

                 // Set a random color
                glm::vec3 color = glm::vec3(
                    zg::crypto::Random::value<float>(0.7f, 1.0f),
                    zg::crypto::Random::value<float>(0.7f, 1.0f),
                    zg::crypto::Random::value<float>(0.7f, 1.0f)
                );
                //  usingCubeCreateInfo.meshInfos[0].material = [color](auto&) -> Material {
                //     return { glm::vec4(color, 1.0f), 0 };
                // };

                // Add the cube and store falling cube data
                size_t entityID = std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo));
                fallingCubes.push_back({entityID, velocity});
            }

            // Update falling cube positions and remove those below the despawn height
            for (auto it = fallingCubes.begin(); it != fallingCubes.end(); ) {
                auto& fallingCube = *it;
                auto& cube = Registry::GetSingleton().getEntity(fallingCube.entityID);

                // Apply gravity to velocity
                fallingCube.velocity.y += FALLING_GRAVITY * *window.deltaTime;

                // Update position
                cube.position += fallingCube.velocity * (float)*window.deltaTime;

                // Remove cube if below despawn height
                if (cube.position.y < FALLING_CUBE_DESPAWN_HEIGHT) {
                    scene.removeEntity(fallingCube.entityID);
                    it = fallingCubes.erase(it);
                } else {
                     // Optional: Subtle rotation
                     cube.rotation = glm::quat(glm::vec3(
                         deltaTimeCounter * 0.2f + cube.position.x * 0.01f,
                         deltaTimeCounter * 0.3f + cube.position.y * 0.01f,
                         deltaTimeCounter * 0.1f + cube.position.z * 0.01f
                     ));
                    ++it;
                }
            }
        }
    };
    return info;
}

