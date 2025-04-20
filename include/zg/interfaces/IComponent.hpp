#pragma once
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <zg/DataStorage.hpp>
#include <zg/Events.hpp>
namespace zg::interfaces
{
	template <typename ComponentT>
	struct IComponent : DataStorage<ComponentT>
	{
		std::vector<size_t*> hostIndexStack;
		size_t ID = 0;
		std::string NAME;
		IComponent(const std::string& name,
							 const DataStorage<ComponentT>::GetDataFunctionMap& getDataFunctionMap = {},
							 const DataStorage<ComponentT>::SetDataFunctionMap& setDataFunctionMap = {},
							 const DataStorage<ComponentT>::DataMap& dataMap = {}) :
				DataStorage<ComponentT>(getDataFunctionMap, setDataFunctionMap, dataMap), NAME(name) {};
		virtual ~IComponent() = default;
	};
} // namespace zg::interfaces
