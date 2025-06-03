#include <zg/raytracing/zgBVH.hpp>
#include <zg/raytracing/BVH.hpp>
#include <zg/glm.hpp>
#include <iostream>
#include <zg/system/headerplujplusdefines.hpp>
#include <zg/system/ThreadPool.hpp>
#include <chrono>
#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/entities/SDF.hpp>
#include <zg/crypto/Random.hpp>
using namespace zg;
using namespace zg::shaders;
using namespace zg::crypto;
template<typename TriangleT, typename UserDataT>
using zgBVH_t = zg::exp::raytracing::BVH<TriangleT, UserDataT>;
using BVH = zg::raytracing::BVH;
struct Triangle
{
private:
    glm::vec3 vertices[3];
public:
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
    {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
    }
    inline const glm::vec3& operator[](size_t index) const
    {
        return vertices[index];
    }
    inline glm::vec3& operator[](size_t index)
    {
        return vertices[index];
    }
};
using zgBVH = zgBVH_t<Triangle, std::pair<size_t, size_t>>;
void testZGBVH();
void testLBBVH();
#define TRACE_AVX
#define WINDOW_WIDTH (1920.f / 2.f)
#define WINDOW_HEIGHT (1080.f / 2.f)
SceneCreateInfo BVHSceneFactory(Window& window);
int main()
{
    testZGBVH();
    testLBBVH();
    Registry rgy;
	ShaderFactory shader_factory;
    register_zg_shader_hooks();
    SDFRegistry sdf_rgy;
    register_zg_sdfs();
    WindowCreateInfo windowInfo{
        .title = "BVH Test",
        .windowWidth = WINDOW_WIDTH,
        .windowHeight = WINDOW_HEIGHT,
        .borderless = true,
        .vsync = false
    };
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(rgy.addWindow(windowInfo));
    window.runOnThread([](auto& window) {
        window.addScene(BVHSceneFactory(window));
    });
    window.registerHandler(EVENT_KEY_PRESS, [&](auto& event) {
        if (event.getValue() == 'q' && event.template castData<bool>())
            window.close();
    });
    window.run();
    return 0;
}
void traceTexture(Scene& scene, std::shared_ptr<textures::Texture>& texture)
{
    auto& rgy = Registry::GetSingleton();
    auto& window = rgy.getWindow(scene.INDEX_STACK);
    auto& windowWidth = *window.windowWidth;
    auto& windowHeight = *window.windowHeight;
    auto rgba = new uint8_t[windowWidth * windowHeight * 4];
    memset(rgba, 0, windowWidth * windowHeight * 4);
    auto& bvh = scene.template getData<zgBVH>("BVH");
    auto getRPointer = [&](size_t x, size_t y)
    {
        return &rgba[(y * (size_t)windowWidth + x) * 4];
    };
    uvec ranvec(
        Random::value<uint16_t>(0, 255),
        Random::value<uint16_t>(0, 255),
        Random::value<uint16_t>(0, 255),
        255
    );
    float div = 4.f;
    auto xDiv = windowWidth / div;
    auto yDiv = windowHeight / div;
    auto& threadPool = *scene.template getData<std::shared_ptr<system::ThreadPool>>("ThreadPool");
    std::vector<system::ThreadPool::Promise> promises;
    promises.reserve(div * div);
    for (size_t x = 0; x < windowWidth; x += xDiv)
    {
        for (size_t y = 0; y < windowHeight; y += yDiv)
        {
            promises.push_back(threadPool.emplace_task([&, x, y]() {
                float outT;
                int outIndex;
                size_t outData;
                size_t ray_index = 0;
                zgBVH::RayPacket packet;
                packet.validMask = 0xFFFF;
                zgBVH::PacketHitInfo out_hit;
                for (auto x_a = x; x_a < x + xDiv; x_a++)
                {
                    for (auto y_a = y; y_a < y + yDiv; y_a++)
                    {
#ifdef TRACE_AVX
                        packet.dirX.m512_f32[ray_index] = 0;
                        packet.dirY.m512_f32[ray_index] = 0;
                        packet.dirZ.m512_f32[ray_index] = -1;
                        packet.invDirX.m512_f32[ray_index] = (std::numeric_limits<float>::max)();
                        packet.invDirY.m512_f32[ray_index] = (std::numeric_limits<float>::max)();
                        packet.invDirZ.m512_f32[ray_index] = 1.f / -1;
                        packet.originX.m512_f32[ray_index] = x_a;
                        packet.originY.m512_f32[ray_index] = y_a;
                        packet.originZ.m512_f32[ray_index] = 5;
                        ray_index++;
                        if (ray_index == RAY_PACKET_WIDTH)
                        {
                            if (bvh.intersect_packet(packet, out_hit))
                            {
                                for (auto i = 0; i < ray_index; i++)
                                {
                                    if (out_hit.triangleIndex.m512i_i32[i] >= 0)
                                    {
                                        auto o_x = packet.originX.m512_f32[i];
                                        auto o_y = packet.originY.m512_f32[i];
                                        auto& vec = *(uvec*)getRPointer(o_x, o_y);
                                        auto& uv = out_hit.uv[i];
                                        auto& data = out_hit.data[i];
                                        auto& entity = rgy.getEntity(data.first);
                                        auto& mesh = rgy.getMesh(entity.meshIDs[data.second]);
                                        if (mesh.info.material.type == 0)
                                        {
                                            vec.r = uv.x * 255;
                                            vec.g = uv.y * 255;
                                            vec.b = 0;
                                            vec.a = 255;
                                            // vec = {0, 0, 255, 255};// mesh.info.material.albedo;
                                        }
                                        else
                                        {
                                            // auto& triangle = bvh.getTriangle(out_hit.triangleIndex.m512i_i32[i]);
                                            // auto& indice = *(glm::ivec3*)&mesh.indices[triangle.mesh_index * 3];
                                            // auto 
                                            // triangle.get_texture_uv(uv);
                                            // vec = {uv.x * 255, uv.y * 255, 0, 255};
                                        }
                                    }
                                }
                            }
                            ray_index = 0;
                        }
#else
                            if (bvh.intersect({x_a, y_a, 5}, {0, 0, -1}, outT, outIndex, outData))
                            {
                                auto& vec = *(uvec*)getRPointer(x_a, y_a);
                                vec = ranvec;
                            }
#endif
                    }
                }
            }));
        }
    }
    for (auto& pp : promises)
    {
        auto& p = *pp;
        auto f = p.get_future();
        f.get();
    }
    if (!texture)
    {
        texture = std::make_shared<textures::Texture>(
            window.iRenderer,
            glm::ivec4(windowWidth, windowHeight, 1, 0),
            (const void*)rgba,
            DEFAULT_TEXTURE_FORMAT,
            DEFAULT_TEXTURE_TYPE,
            DEFAULT_TEXTURE_FILTERTYPE,
            false,
            DEFAULT_TEXTURE_MULTISAMPLING,
            TEXTURE_CLAMP_EDGE
        );
        texture->isTransparent = false;
    }
    else
    {
        texture->update((const void*)rgba);
    }
    delete[] rgba;
}
SceneCreateInfo BVHSceneFactory(Window& window)
{
    SceneCreateInfo sceneInfo{
        .name = "BVH Scene",
        .cameraPosition = {(*window.windowWidth) / 2.f, (*window.windowHeight) / 2.f, 5},
        .cameraDirection = {0, 0, -1},
        .projectionType = vp::Projection::TYPE::Orthographic,
        .orthoSize = {*window.windowWidth, *window.windowHeight},
        .onAttachedFunction = [](auto& scene) {
            scene.clearColor = {0, 1, 0, 1};
            auto& bvh = scene.template make<zgBVH>("BVH");
            scene.template make<std::shared_ptr<system::ThreadPool>>("ThreadPool", std::make_shared<system::ThreadPool>());
            auto& rgy = Registry::GetSingleton();
            auto& window = rgy.getWindow(scene.INDEX_STACK);
            auto& windowWidth = *window.windowWidth;
            auto& windowHeight = *window.windowHeight;

            auto cubeInfo = entities::CubeFactory(
                "Test Cube",
                {windowWidth / 2.f, windowHeight / 2.f, -5},
                rotate_identity, {windowWidth / 6.f, windowHeight / 6.f, ((windowWidth + windowHeight) / 4.f)},
                {1, 0, 0, 1}
            );
            auto cube_tuple = scene.addEntity(cubeInfo);
            auto cube_ID = std::get<KEY_ID_VECTOR_ID_INDEX>(cube_tuple);
            scene.template make<size_t>("OutputPlaneID", cube_ID);
            auto& cube = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(cube_tuple);
            bvh.addEntity(cube);
            cube.skipRender = true;
            window.registerHandler(EVENT_KEY_PRESS, [cube_ID, scene_ID = scene.ID](auto& event) {
                auto& pressed = event.template castData<bool>();
                if (!pressed)
                    return;
                auto& rgy = Registry::GetSingleton();
                auto& scene = rgy.getScene(scene_ID);
                auto& bvh = scene.template getData<zgBVH>("BVH");
                auto& cube = rgy.getEntity(cube_ID);
                glm::quat angle;
                switch (event.getValue())
                {
                    case KEYCODE_LEFT:
                    {
                        angle = glm::angleAxis(glm::radians(360.f / 24.f), glm::vec3(0, 1, 0));
                        break;
                    }
                    case KEYCODE_RIGHT:
                    {
                        angle = glm::angleAxis(-glm::radians(360.f / 24.f), glm::vec3(0, 1, 0));
                        break;
                    }
                    case KEYCODE_UP:
                    {
                        angle = glm::angleAxis(glm::radians(360.f / 24.f), glm::vec3(0, 0, 1));
                        break;
                    }
                    case KEYCODE_DOWN:
                    {
                        angle = glm::angleAxis(-glm::radians(360.f / 24.f), glm::vec3(0, 0, 1));
                        break;
                    }
                    default: return;
                }
                cube.rotation *= angle;
                bvh.updateEntity(cube);
            });
            window.registerHandler(EVENT_MOUSE_PRESS, [scene_ID = scene.ID](auto& event) {
                auto& pressed = event.template castData<bool>();
                if (!pressed)
                    return;
                auto& rgy = Registry::GetSingleton();
                auto& scene = rgy.getScene(scene_ID);
                auto& bvh = scene.template getData<zgBVH>("BVH");
                // add cubes
            });
            std::shared_ptr<textures::Texture> texture;
            traceTexture(scene, texture);
            auto planeInfo = entities::PlaneFactory(texture, "Output Plane", {windowWidth / 2.f, windowHeight / 2.f, 0}, rotate_identity, {windowWidth, windowHeight, 1});
            scene.template make<size_t>("OutputPlaneID", std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(planeInfo)));
        },
        .preUpdateFunction = [](auto& scene)
        {
            auto& outputPlaneID = scene.template getData<size_t>("OutputPlaneID");
            auto& rgy = Registry::GetSingleton();
            auto& outputPlane = rgy.getEntity(outputPlaneID);
            traceTexture(scene, outputPlane.meshInfos[0].keyedTextures[0].second);
            outputPlane.refreshMeshes();
        }
    };
    return sceneInfo;
}
void testZGBVH()
{
    zgBVH bvh;
    bvh.addTriangle({
        {0, 1, 0},
        {0, 0, 0},
        {1, 0, 0}
    }, {1, 1});
    bvh.addTriangle({
        {1, 1, 0},
        {0, 1, 0},
        {1, 0, 0}
    }, {1, 1});
    auto begin = SYS_CLOCK::now();
    bvh.build();
    auto end = SYS_CLOCK::now();
    auto diff = end - begin;
    std::cout << "Built BVH in: " << ns_to_s_string(diff.count()) << std::endl;
    float outT;
    int outIndex;
    size_t outData;
    begin = SYS_CLOCK::now();
    size_t intersect = 0, no_intersect = 0;
    zgBVH::RayPacket packet;
    packet.validMask = 0xFFFF;
    zgBVH::PacketHitInfo out_hit;
    size_t ray_index = 0;
    for (size_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (size_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            auto x_f = x / WINDOW_WIDTH;
            auto y_f = y / WINDOW_HEIGHT;
#ifdef TRACE_AVX
            packet.dirX.m512_f32[ray_index] = 0;
            packet.dirY.m512_f32[ray_index] = 0;
            packet.dirZ.m512_f32[ray_index] = -1;
            packet.invDirX.m512_f32[ray_index] = (std::numeric_limits<float>::max)();
            packet.invDirY.m512_f32[ray_index] = (std::numeric_limits<float>::max)();
            packet.invDirZ.m512_f32[ray_index] = 1.f / -1;
            packet.originX.m512_f32[ray_index] = x_f;
            packet.originY.m512_f32[ray_index] = y_f;
            packet.originZ.m512_f32[ray_index] = 5;
            ray_index++;
            if (ray_index == RAY_PACKET_WIDTH)
            {
                if (bvh.intersect_packet(packet, out_hit))
                {
                    for (auto i = 0; i < ray_index; i++)
                    {
                        if (out_hit.triangleIndex.m512i_i32[i] >= 0)
                        {
                            intersect++;
                        }
                        else
                        {
                            no_intersect++;
                        }
                    }
                }
                ray_index = 0;
            }
#else
            if (bvh.intersect({x_f, y_f, 5}, {0, 0, -1}, outT, outIndex, outData))
            {
                intersect++;
            }
            else
            {
                no_intersect++;
            }
#endif
        }
    }
    end = SYS_CLOCK::now();
    diff = end - begin;
    std::cout << "Traced BVH in: " << ns_to_s_string(diff.count()) << std::endl;
    std::cout << "Intersect: " << intersect << ", No Intersect: " << no_intersect << std::endl;
}
void testLBBVH()
{
    BVH bvh;
    bvh.addTriangle({
        {0, 0, 0},
        {1, 0, 0},
        {0, 1, 0}, {1, 0}});
    bvh.addTriangle({
        {1, 0, 0},
        {0, 1, 0},
        {1, 1, 0}, {1, 0}});
    auto begin = SYS_CLOCK::now();
    bvh.buildBVH();
    auto end = SYS_CLOCK::now();
    auto diff = end - begin;
    std::cout << "Built BVH in: " << ns_to_s_string(diff.count()) << std::endl;
    begin = SYS_CLOCK::now();
    size_t intersect = 0, no_intersect = 0;
    for (size_t x = 0; x < WINDOW_WIDTH; x++)
    {
        for (size_t y = 0; y < WINDOW_HEIGHT; y++)
        {
            zg::raytracing::Ray ray;
            auto x_f = x / WINDOW_WIDTH;
            auto y_f = y / WINDOW_HEIGHT;
            ray.org = {x_f, y_f, 5};
            ray.dir = {0, 0, -1};
            ray.tmin = 0.1f;
            ray.tmax = 100.f;
            auto primID = bvh.trace(ray);
            if (primID != zg::raytracing::invalidID)
            {
                intersect++;
                // std::cout << "BVH intersected!" << std::endl <<
                //     "\toutT: " << outT << std::endl <<
                //     "\toutIndex: " << outIndex << std::endl <<
                //     "\toutData: " << outData << std::endl;
            }
            else
            {
                no_intersect++;
                // std::cout << "No intersection" << std::endl;
            }
        }
    }
    end = SYS_CLOCK::now();
    diff = end - begin;
    std::cout << "Traced BVH in: " << ns_to_s_string(diff.count()) << std::endl;
    std::cout << "Intersect: " << intersect << ", No Intersect: " << no_intersect << std::endl;
}