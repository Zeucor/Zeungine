#include <zg/Standard.hpp>
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg::components::scenes
{
	struct P2P : zg::interfaces::ISceneComponent
	{
		friend Scene;

	private:
		Scene& scene;
		std::string_view key;
        std::string castaddr;

	public:
		P2P(Scene& scene, std::string_view key, const std::string &castaddr);

	protected:
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
	};
} // namespace zg::components::scenes
