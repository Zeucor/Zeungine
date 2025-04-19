#pragma once
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <zg/DataStorage.hpp>
#include <zg/Events.hpp>
namespace zg::interfaces
{
	struct IComponent : DataStorage<IComponent>
	{
		void* host = 0;
		size_t ID = 0;
		std::string NAME;
		IComponent(const std::string& name, const DataStorage<IComponent>::GetDataFunctionMap& getDataFunctionMap = {},
							 const DataStorage<IComponent>::SetDataFunctionMap& setDataFunctionMap = {},
							 const DataStorage<IComponent>::DataMap& dataMap = {}) :
				DataStorage<IComponent>(getDataFunctionMap, setDataFunctionMap, dataMap), NAME(name) {};
		virtual ~IComponent() = default;
	};
} // namespace zg::interfaces
