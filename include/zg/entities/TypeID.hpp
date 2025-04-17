#include <cstdint>
template <typename T>
struct EntityTypeID;
#define ZG_BASE_OFFSET_ZGCORE 1
#define ZG_BASE_OFFSET_DEVELOPER 1'111'111
#define ZG_BASE_OFFSET_MOD 2'222'222
namespace zg::entities
{
    struct Asset;
    struct Folder;
    struct Breadcrumbs;
    struct AssetGrid;
    struct AssetBrowser;
    struct Button;
    struct Console;
    struct Cube;
    template <size_t N, typename T>
    struct NDParametricCurve;
    template <size_t N, typename T>
    struct NUVVolume;
    struct Dialog;
    struct DropdownMenu;
    struct DropdownItem;
    struct Input;
    struct PanelMenu;
    struct PanelItem;
    struct Plane;
    struct SkyBox;
    struct StatusText;
    struct TabsBar;
    struct Tab;
    struct TextView;
    struct Toolbar;
} // namespace zg::entities
namespace zg::media::entities
{
    struct Audio;
    struct Image;
    struct Video;
}
#define ZG_DEFINE_ENTITY_ID(TYPE, ID)                                                                                  \
template <>                                                                                                          \
struct EntityTypeID<TYPE>                                                                                            \
{                                                                                                                    \
    static constexpr size_t id = ID;                                                                                   \
}
ZG_DEFINE_ENTITY_ID(zg::entities::Asset, ZG_BASE_OFFSET_ZGCORE + 1);
ZG_DEFINE_ENTITY_ID(zg::entities::Folder, ZG_BASE_OFFSET_ZGCORE + 2);
ZG_DEFINE_ENTITY_ID(zg::entities::Breadcrumbs, ZG_BASE_OFFSET_ZGCORE + 3);
ZG_DEFINE_ENTITY_ID(zg::entities::AssetGrid, ZG_BASE_OFFSET_ZGCORE + 4);
ZG_DEFINE_ENTITY_ID(zg::entities::AssetBrowser, ZG_BASE_OFFSET_ZGCORE + 5);
ZG_DEFINE_ENTITY_ID(zg::entities::Button, ZG_BASE_OFFSET_ZGCORE + 6);
ZG_DEFINE_ENTITY_ID(zg::entities::Console, ZG_BASE_OFFSET_ZGCORE + 7);
ZG_DEFINE_ENTITY_ID(zg::entities::Cube, ZG_BASE_OFFSET_ZGCORE + 8);
#define NDParametricCurve_2_float zg::entities::NDParametricCurve<2, float>
#define NDParametricCurve_3_float zg::entities::NDParametricCurve<3, float>
ZG_DEFINE_ENTITY_ID(NDParametricCurve_2_float, ZG_BASE_OFFSET_ZGCORE + 9);
ZG_DEFINE_ENTITY_ID(NDParametricCurve_3_float, ZG_BASE_OFFSET_ZGCORE + 10);
ZG_DEFINE_ENTITY_ID(zg::entities::Dialog, ZG_BASE_OFFSET_ZGCORE + 11);
ZG_DEFINE_ENTITY_ID(zg::entities::DropdownMenu, ZG_BASE_OFFSET_ZGCORE + 12);
ZG_DEFINE_ENTITY_ID(zg::entities::DropdownItem, ZG_BASE_OFFSET_ZGCORE + 13);
ZG_DEFINE_ENTITY_ID(zg::entities::Input, ZG_BASE_OFFSET_ZGCORE + 14);
ZG_DEFINE_ENTITY_ID(zg::entities::PanelMenu, ZG_BASE_OFFSET_ZGCORE + 15);
ZG_DEFINE_ENTITY_ID(zg::entities::PanelItem, ZG_BASE_OFFSET_ZGCORE + 16);
ZG_DEFINE_ENTITY_ID(zg::entities::Plane, ZG_BASE_OFFSET_ZGCORE + 17);
ZG_DEFINE_ENTITY_ID(zg::entities::SkyBox, ZG_BASE_OFFSET_ZGCORE + 18);
ZG_DEFINE_ENTITY_ID(zg::entities::StatusText, ZG_BASE_OFFSET_ZGCORE + 19);
ZG_DEFINE_ENTITY_ID(zg::entities::TabsBar, ZG_BASE_OFFSET_ZGCORE + 20);
ZG_DEFINE_ENTITY_ID(zg::entities::Tab, ZG_BASE_OFFSET_ZGCORE + 21);
ZG_DEFINE_ENTITY_ID(zg::entities::TextView, ZG_BASE_OFFSET_ZGCORE + 22);
ZG_DEFINE_ENTITY_ID(zg::entities::Toolbar, ZG_BASE_OFFSET_ZGCORE + 23);
#define NUVVolume_2_float zg::entities::NUVVolume<2, float>
#define NUVVolume_3_float zg::entities::NUVVolume<3, float>
ZG_DEFINE_ENTITY_ID(NUVVolume_2_float, ZG_BASE_OFFSET_ZGCORE + 24);
ZG_DEFINE_ENTITY_ID(NUVVolume_3_float, ZG_BASE_OFFSET_ZGCORE + 25);

ZG_DEFINE_ENTITY_ID(zg::media::entities::Audio, ZG_BASE_OFFSET_ZGCORE + 300);
ZG_DEFINE_ENTITY_ID(zg::media::entities::Video, ZG_BASE_OFFSET_ZGCORE + 301);
ZG_DEFINE_ENTITY_ID(zg::media::entities::Image, ZG_BASE_OFFSET_ZGCORE + 302);