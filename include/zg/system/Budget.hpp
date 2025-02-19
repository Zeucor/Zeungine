/**
 * @file Budget.hpp
 * @author ZeunO8 (mr.steven.french@gmail.com)
 * @brief  Declares a program budget, which you can update, keep track of your reals, and sleep
 * [zeungine attempt at pacing algorithms while getting the correct amount of loop sleep]
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
using namespace std;
#define DECLARE_NZBUDGET(NAME, BUDGET_INSTANT_SECONDS_DURATION)                                             \
	inline static zg::budget::ZBudget NAME = {BUDGET_INSTANT_SECONDS_DURATION}
namespace zg::budget
{
	/**
	 * @brief Interface for a zg::budget::IBudget
	 *
	 * @tparam SecondsDuration your seconds duration to the chrono::duration<TimeReal>
	 */
	template <typename SecondsDuration>
	struct IBudget
	{
		SecondsDuration m_IsZgBudgetReal;
	};
// some bbudget considerations (^options+)
#define REAL long double
#define CLOCK chrono::system_clock
#define HISTORY_SIZE 38
#define SECONDS_DURATION chrono::duration<REAL, nano>
#define DBudget zg::budget::IBudget<SECONDS_DURATION>
	struct FBudget;
/*|phase|*/
#define BUDGET_SLEPT_FOR_PRINT
#define BUDGET_CLOCKED_FOR_PRINT
#define BUDGET_CALCAVG_FOR_PRINT
	/**
	 *
	 * @brief Zeungines' evolving history implementation of DBudget
	 *
	 */
	template <size_t HistorySize = HISTORY_SIZE, typename Clock = CLOCK, typename Real = REAL,
						typename TimePoint = CLOCK::time_point, typename ChronoSeconds = chrono::seconds,
						typename ChronoNanoSeconds = chrono::nanoseconds, typename SecondsDuration = SECONDS_DURATION,
						intmax_t Denominator = 1>
	struct ZBudget : DBudget
	{
		friend FBudget;

	protected:
		inline static SecondsDuration ZeroSecondsDuration = SecondsDuration(0.0);
		constexpr static bool m_AutoCalculateAverage = true;
		inline static SecondsDuration OneSecondsDuration = SecondsDuration(1.0);

	public:
		ZBudget(const SecondsDuration BudgetTime) : m_BudgetTime(BudgetTime) {};
		SecondsDuration update(const TimePoint& tp = CLOCK::now())
		{
			if (!m_AutoCalculateAverage)
				goto _g_unjb;
			if (m_HistoryIndex < HistorySize - 1)
			{
			_record_history:
				auto& history_tuple = m_History[m_HistoryIndex];
				auto& begin = get<0>(history_tuple);
				auto& end = get<1>(history_tuple);
				auto& seconds = get<2>(history_tuple);
				Real beginTSE = begin.time_since_epoch().count();
				Real endTSE = end.time_since_epoch().count();
				// weve been here before clean up
				if (beginTSE && endTSE)
				{
					memset(&begin, 0, sizeof(begin));
					memset(&end, 0, sizeof(end));
					memset(&seconds, 0, sizeof(seconds));
					beginTSE = begin.time_since_epoch().count();
					endTSE = end.time_since_epoch().count();
				}
				if (beginTSE == 0)
				{
					begin = CLOCK::now();
#ifdef BUDGET_CLOCKED_FOR_PRINT
					cerr << "zupdate() is at start of 2pass @ " << begin << endl;
#endif
					return m_IsZgBudgetRealBegin;
				}
				else if (m_IsZSle_p && endTSE == 0)
				{
					end = CLOCK::now();
					auto nsduration = end - begin;
					seconds = end - begin;
					auto avgBud = CalculateAverageBudget();
					m_HistoryIndex++;
#ifdef BUDGET_CLOCKED_FOR_PRINT
					cerr << "zupdate() reached end of 2pass @ " << end << ", nanoseconds(t): " << nsduration << ", zbudget is now: " << avgBud << endl;
#endif
					m_IsZSle_p = false;
					return avgBud;
				}
				else
				{
					auto somewhereidekknowinthemiddle = CLOCK::now();
					auto nsduration = somewhereidekknowinthemiddle - begin;
					seconds = somewhereidekknowinthemiddle - begin;
#ifdef BUDGET_CLOCKED_FOR_PRINT
					cerr << "zupdate() went update in 2pass @ " << somewhereidekknowinthemiddle << ", nanoseconds(@): " << nsduration << endl;
#endif
				}
			}
			else
			{
				m_HistoryIndex = ComputeHistoryEvolvedIndex();
				goto _record_history;
			}
		_g_unjb:
			return m_IsZgBudgetRealBegin;
		}
		SecondsDuration operator/(SecondsDuration a)
		{
			auto b = m_IsZgBudgetReal / a;
			SecondsDuration c = SecondsDuration(b);
			m_IsZgBudgetReal = SecondsDuration(m_IsZgBudgetReal - c);
			return c;
		}
		void zsleep()
		{
			m_IsZSle_p = true;
			update();
#ifdef BUDGET_SLEPT_FOR_PRINT
			cerr << "...    ?  zsle_p :&+    Cojpint@ " << m_IsZgBudgetReal << endl;
#endif
			this_thread::sleep_for(m_IsZgBudgetReal);
		}

	private:
		bool m_IsZSle_p = false;
		size_t m_HistoryIndex = 0;
		size_t m_HistoryTotalLength = 0;
		SecondsDuration m_BudgetTime;
		SecondsDuration m_IsZgBudgetReal;
		SecondsDuration m_IsZgBudgetRealBegin;
		array<tuple<TimePoint, TimePoint, SecondsDuration>, HistorySize> m_History;
		array<size_t, 1> historyEvolvedIndexMap;
		SecondsDuration CalculateAverageBudget()
		{
			chrono::duration<REAL, nano> total_avg_budget = chrono::duration<REAL, nano>((REAL)0.0);
			auto& history_tuple = m_History[m_HistoryIndex];
			auto& secondsTakenByCook = get<2>(history_tuple);
#ifdef BUDGET_CALCAVG_FOR_PRINT
			cerr << "zCalcAvg(): seconds tkn in loop@ " << secondsTakenByCook << endl;
#endif
			auto remainingBudget = m_BudgetTime - secondsTakenByCook;
			auto remainingBudgetCount = remainingBudget.count();
#ifdef BUDGET_CALCAVG_FOR_PRINT
			cerr << "zCalcAvg(): remaining budget .:@ " << remainingBudget << endl;
#endif
			m_IsZgBudgetReal = m_IsZgBudgetRealBegin = ((remainingBudgetCount >= 0) ? remainingBudget : ZeroSecondsDuration);
			return m_IsZgBudgetRealBegin;
		}
		size_t ComputeHistoryEvolvedIndex()
		{
			if (m_HistoryIndex < HistorySize)
			{
				auto indexHash = getHistoryIndexHash();
				auto resolvedHistoryIndex = calculateResolvedHistoryIndex(indexHash);
				return historyEvolvedIndexMap[indexHash] = resolvedHistoryIndex;
			}
		}
		size_t getHistoryIndexHash() { return 0; }
		size_t calculateResolvedHistoryIndex(size_t indexHash) { return 0; }
	};
} // namespace zg::budget
