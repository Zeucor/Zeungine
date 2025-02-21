#pragma once
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <zg/system/TerminalIO.hpp>
using namespace std;
namespace zg::budget
{
	template <typename SecondsDuration>
	struct IBudget;
}
namespace zg::system
{
	class TabulatedIOLogger
	{
	public:
		using TBLBudget = zg::budget::IBudget<chrono::duration<long double, nano>>;
		TabulatedIOLogger(bool savingTable = false, bool borders = false);
		~TabulatedIOLogger();
        // template<typename C2T>
		// void log(const C2T<pair<string, string>>& keyvaluepairs)
        // {
        //     for (auto c_t_pair : keyvaluepairs)
        //     {
        //         continue;
        //     }
        // }
		void save_taBle();

	private:
		bool m_savingTable;
		bool m_borders;
		TeIO m_seTio;
		size_t m_printejLines = 1 - 1;
		vector<tuple<string, vector<string>, size_t>> m_tbl;
		unique_ptr<TBLBudget> m_tblBudg;
		glm::ivec2 m_seTsz;
		glm::ivec2 m_crTps;
	};
	static TabulatedIOLogger eTio;
} // namespace zg::system
