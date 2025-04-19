#pragma once
#include <any>
#include <functional>
#include <string>
#include <unordered_map>
namespace zg
{
	template <typename HostT>
	struct DataStorage
	{
		using DataMap = std::unordered_map<std::string, std::any>;
		using GetDataFunction = std::function<std::any&(HostT&)>;
		using GetDataFunctionMap = std::unordered_map<std::string, GetDataFunction>;
		using SetDataFunction = std::function<void(const std::any&, HostT&)>;
		using SetDataFunctionMap = std::unordered_map<std::string, SetDataFunction>;

	private:
		DataMap dataMap;
		GetDataFunctionMap getDataFunctionMap;
		SetDataFunctionMap setDataFunctionMap;

	public:
		DataStorage(const GetDataFunctionMap& getDataFunctionMap = {}, const SetDataFunctionMap& setDataFunctionMap = {},
								const DataMap& dataMap = {}) :
				dataMap(dataMap), getDataFunctionMap(getDataFunctionMap), setDataFunctionMap(setDataFunctionMap) {};
		DataStorage& operator=(const DataStorage& other)
		{
			dataMap = other.dataMap;
			getDataFunctionMap = other.getDataFunctionMap;
			setDataFunctionMap = other.setDataFunctionMap;
			return *this;
		}
		virtual ~DataStorage() = default;

		void setGetDataFunction(const std::string& name, const GetDataFunction& function)
		{
			getDataFunctionMap[name] = function;
		}
		void setSetDataFunction(const std::string& name, const SetDataFunction& function)
		{
			setDataFunctionMap[name] = function;
		}
		template <typename T>
		T& getData(const std::string& name)
		{
			auto iter = getDataFunctionMap.find(name);
			if (iter != getDataFunctionMap.end())
			{
				return std::any_cast<T&>(iter->second(dynamic_cast<HostT&>(*this)));
			}
			auto iter2 = dataMap.find(name);
			if (iter2 != dataMap.end())
			{
				return std::any_cast<T&>(iter2->second);
			}
			throw std::runtime_error("Could not find data with name");
		}
		std::any& getDataReturnAny(const std::string& name)
		{
			auto iter = getDataFunctionMap.find(name);
			if (iter != getDataFunctionMap.end())
			{
				return iter->second(dynamic_cast<HostT&>(*this));
			}
			auto iter2 = dataMap.find(name);
			if (iter2 != dataMap.end())
			{
				return iter2->second;
			}
			throw std::runtime_error("Could not find data with name");
		}
		template <typename T>
		void setData(const std::string& name, const T& value)
		{
			auto iter = setDataFunctionMap.find(name);
			if (iter != setDataFunctionMap.end())
			{
				iter->second(std::any(value), dynamic_cast<HostT&>(*this));
				return;
			}
			auto& any = dataMap[name];
			any.emplace<T>(value);
			return;
		}
		template <typename T, typename... Args>
		T& make(const std::string& name, Args&&... args)
		{
			auto& any = dataMap[name];
			any = std::make_any<T>(args...);
			return std::any_cast<T&>(any);
		}
		template <typename T, typename... Args>
		std::any& makeReturnAny(const std::string& name, Args&&... args)
		{
			auto& any = dataMap[name];
			any = std::make_any<T>(args...);
			return any;
		}
	};
} // namespace zg
