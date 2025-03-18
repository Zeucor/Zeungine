#pragma once
#include "../Scene.hpp"
#include <functional>
namespace zg::vp
{
    /**
     * @brief View Front Back Left Right
     * 
     */
    struct VFBLR
    {
        enum KeyScheme
        {
            UDLRSH = 1,
            WSADSC = 2
        };
        Scene& scene;
        KeyScheme keyScheme;
        Key f = 0;
        Key b = 0;
        Key l = 0;
        Key r = 0;
        UniqueIdentifier fID = 0;
        UniqueIdentifier bID = 0;
        UniqueIdentifier lID = 0;
        UniqueIdentifier rID = 0;
        float force;
        VFBLR(Scene& scene, KeyScheme keyScheme, float force);
        ~VFBLR();
        void onFrontTick();
        void onBackTick();
        void onLeftTick();
        void onRightTick();
    };
}