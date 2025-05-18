#pragma once
#ifdef ENABLE_TRACY
#include <tracy/tracy/Tracy.hpp>
#define ZGZoneScoped ZoneScoped
#define ZGZoneScopedN(STR) ZoneScopedN(STR)
#define ZGFrameMark FrameMark
#else
#define ZGZoneScoped
#define ZGZoneScopedN(STR)
#define ZGFrameMark
#endif