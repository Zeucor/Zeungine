#include <zg/Standard.hpp>
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg::components::scenes
{
	struct P2P : zg::interfaces::ISceneComponent
	{
		friend Scene;

	private:
		std::string_view key;
        std::string castaddr;

	public:
		P2P(std::string_view key, const std::string castaddr);

	protected:
		void onUpdate(Scene& scene) override;
		void onAdded(Scene& scene) override;
		void onRemoved(Scene& scene) override;
	};
} // namespace zg::components::scenes
