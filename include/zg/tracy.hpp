#pragma once
#include <tracy/tracy/Tracy.hpp>
#ifdef ENABLE_TRACY
#define ZGZoneScoped ZoneScoped
#define ZGZoneScopedN(STR) ZoneScopedN(STR)
#define ZGFrameMark FrameMark
#else
#define ZGZoneScoped
#define ZGZoneScopedN(STR)
#define ZGFrameMark
#endif