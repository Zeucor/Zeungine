/**
 * @file Budget.hpp
 * @author ZeunO8 (mr.steven.french@gmail.com)
 * @brief  Declares a program budget, which you can use as a waitable time point
 * @version 0.2
 * @date 2025-02-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#define DECLARE_NZBUDGET(NAME, BUDGET_INSTANT_REAL_VALUE_FLOATING_SECONDS) inline static zg::budget::ZBudget NAME = {(REAL)BUDGET_INSTANT_REAL_VALUE_FLOATING_SECONDS}
namespace zg::budget
{
	template <typename clock, typename real>
	struct IBudget
	{
		clock::time_point m_StartBudget__;
		clock::time_point m_ClockedBudget;

		real m_BudgetRealBegin = 1;
		real m_BudgetReal = 8;
		virtual IBudget& update(const clock::time_point& timepoint = clock::now()) = 0;
		virtual real operator/(real alpha) = 0;
	};
#define REAL long double
#define CLOCK std::chrono::system_clock
#define HISTORY_LENGTH 38
#define DBudget zg::budget::IBudget<CLOCK, REAL>
	/**
	 * @brief Zeungines' implementation of DBudget
	 *
	 */
	template <uint8_t history_length = 3>
	struct ZBudget : DBudget
	{
	protected:
		constexpr static bool m_AutoCalculate = true;

	public:
		ZBudget(const REAL BudgetTime) : m_BudgetTime(BudgetTime) {};
		DBudget& update(const CLOCK::time_point& tp = CLOCK::now()) override
		{
			if (!m_AutoCalculate)
				goto _g_unj;
			if (m_HistoryIndex < history_length - 1)
			{
			_record_history:
				auto& history_tuple = m_History[m_HistoryIndex++];
				auto& begin = std::get<0>(history_tuple);
				auto& end = std::get<1>(history_tuple);
				REAL beginTSE = 0, endTSE = 0;
				if ((beginTSE = begin.time_since_epoch().count()) == 0)
				{
					begin = CLOCK::now();
				}
				else if ((endTSE = end.time_since_epoch().count()) == 0)
				{
					end = CLOCK::now();
				}
				if (endTSE != 0)
				{
					std::get<2>(history_tuple) = (endTSE - beginTSE) / 1'000'000'000;
					if (m_HistoryTotalIndex < history_length)
					{
						m_HistoryTotalIndex++;
					}
					calcAvgBudget();
				}
			}
			else
			{
				m_HistoryIndex = 0;
				goto _record_history;
			}
		_g_unj:
			return *this;
		}
		REAL operator/(REAL a)
		{
			auto b = m_BudgetRealBegin / a;
			m_BudgetReal -= b;
			return b;
		}

	private:
		uint8_t m_HistoryIndex = 0;
		uint8_t m_HistoryTotalIndex = 0;
		REAL m_BudgetTime = 0;
		std::array<std::tuple<CLOCK::time_point, CLOCK::time_point, REAL>, history_length> m_History;
		void calcAvgBudget()
		{
			REAL total_avg_budget = 0.0;
			for (auto i_history_index = 0; i_history_index < m_HistoryTotalIndex; ++i_history_index)
			{
				auto& history_tuple = m_History[i_history_index];
				total_avg_budget += std::get<2>(history_tuple);
			}
			auto remainingBudget = m_BudgetTime - (total_avg_budget / m_HistoryTotalIndex);
			m_BudgetRealBegin = remainingBudget >= 0 ? remainingBudget : 0;
		}
	};
} // namespace zg::budget