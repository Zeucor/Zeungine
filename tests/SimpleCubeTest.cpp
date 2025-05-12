#include <iostream>
#include <vector>
#include <cmath> // For spherical calculations
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
#include <glm/gtc/constants.hpp> // For glm::pi
#include <glm/gtc/matrix_transform.hpp> // For rotation if needed

using namespace zg;

// Define sphere parameters
const int NUM_CUBES = 1600; // Total number of cubes
const float SPHERE_RADIUS = 80.0f; // Radius of the initial sphere
const float CUBE_INITIAL_SCALE = 4.0f; // Initial uniform scale for cubes

auto cubeCreateInfo = entities::CubeFactory("Sphere Cube", {0, 0, 0}, {1, 0, 0, 0}, {CUBE_INITIAL_SCALE, CUBE_INITIAL_SCALE, CUBE_INITIAL_SCALE});
// auto planeCreateInfo = entities::PlaneFactory({0.3, 0.25, 0.35, 0.75}, "Basic Grey Plane", {0, -5.5, 0}, {1, 0, 0, 0}, {1000, 1, 1000});

SceneCreateInfo ExampleSceneFactory();

int main()
{
    WindowCreateInfo windowCreateInfo{.title = "Beautiful 3D Cube Sphere Test", .borderless = true, .vsync = false, .framerate = 144};
    auto window_tuple = zg::Registry::addWindow(windowCreateInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);

    window.runOnThread([](auto& window) {
        auto sceneCreateInfo = ExampleSceneFactory();
        window.addScene(sceneCreateInfo);
    });

    window.run();
}

SceneCreateInfo ExampleSceneFactory()
{
    SceneCreateInfo info{
        .name = "ExampleScene",
        .cameraPosition = glm::vec3(0, 0, SPHERE_RADIUS * 2.0f), // Position camera to view the sphere
        .cameraDirection = glm::normalize(glm::vec3(0, 0, -1)), // Look towards the center
        .onAttachedFunction = [](auto& scene)
        {
            auto& window = Registry::getWindow(scene.INDEX_STACK);

            // Set a dark clear color for contrast
            scene.clearColor = {0.05f, 0.05f, 0.1f, 1.0f};

            auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
            cubes.reserve(NUM_CUBES); // Reserve space for efficiency

            // Store initial spherical coordinates for updates
            auto& initialSphericalCoords = scene.template make<std::vector<glm::vec2>>("InitialSphericalCoords");
            initialSphericalCoords.reserve(NUM_CUBES);

            // Create cubes arranged on a sphere
            for (int i = 0; i < NUM_CUBES; ++i)
            {
                auto usingCubeCreateInfo = cubeCreateInfo;

                // Calculate spherical coordinates (using Fibonacci sphere or similar distribution for even spacing)
                // A simple mapping from 1D index to spherical coordinates for demonstration:
                float phi = glm::acos(-1.0f + (2.0f * i) / (NUM_CUBES - 1)); // Latitude (0 to pi)
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
                float value = 1.0f; // Keep value high for bright colors

                // Convert HSL to RGB (simplified) - You might need a proper HSL to RGB conversion function
                // This is a placeholder, a full HSL to RGB conversion would be more accurate
                glm::vec3 color;
                float c = value * saturation;
                float x_hsl = c * (1.0f - glm::abs(std::fmod(hue * 6.0f, 2.0f) - 1.0f));
                float m = value - c;

                if (hue < 1.0f/6.0f) color = glm::vec3(c, x_hsl, 0);
                else if (hue < 2.0f/6.0f) color = glm::vec3(x_hsl, c, 0);
                else if (hue < 3.0f/6.0f) color = glm::vec3(0, c, x_hsl);
                else if (hue < 4.0f/6.0f) color = glm::vec3(0, x_hsl, c);
                else if (hue < 5.0f/6.0f) color = glm::vec3(x_hsl, 0, c);
                else color = glm::vec3(c, 0, x_hsl);

                color += glm::vec3(m); // Add the lightness component

                usingCubeCreateInfo.meshInfos[0].material.albedo = glm::vec4(color, 1.0f);

                // Add the cube and store its ID
                cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
            }

            // Add the plane (optional, commented out)
            // scene.template setData<size_t>("PlaneID", std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(planeCreateInfo)));

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

            // Attach scene components
            scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
            scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 25));
            scene.template make<float>("deltaTimeCounter", 0.f);
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
            auto deltaTimeCounter = (scene.template getData<float>("deltaTimeCounter") += window.deltaTime);

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
                auto& cubeID = cubes[i]; // Access by reference
                auto& cube = Registry::getEntity(cubeID);
                const auto& initialCoords = initialSphericalCoords[i]; // Initial (phi, theta)

                float phi = initialCoords.x;
                float theta = initialCoords.y;

                // Calculate perturbation for the surface wave
                // Using initial theta (longitude) and time for a simple wave propagating around the sphere
                float surface_perturbation = wave_amplitude * glm::sin(theta * wave_frequency + deltaTimeCounter * wave_speed);

                // Calculate the new position on the pulsating sphere with surface waves
                float current_sphere_radius = current_radius + surface_perturbation;

                float x = current_sphere_radius * glm::sin(phi) * glm::cos(theta);
                float y = current_sphere_radius * glm::cos(phi);
                float z = current_sphere_radius * glm::sin(phi) * glm::sin(theta);

                cube.position = glm::vec3(x, y, z);

                // Calculate scale factor based on a different time/position function
                // This creates a separate pulsating effect on the scale of individual cubes
                float scale_factor = glm::mix(scale_min, scale_max, (glm::sin(deltaTimeCounter * scale_pulse_speed + phi) * 0.5f + 0.5f));
                cube.scale = glm::vec3(CUBE_INITIAL_SCALE * scale_factor);
            }
        }
    };
    return info;
}
