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
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <zg/system/TerminalIO.hpp>
#include <zg/zgfilesystem/Directory.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/zgfilesystem/Serial.hpp>
using namespace zg::system;
using namespace std;
#define DECLARE_NZBUDGET(NAME, BUDGET_INSTANT_SECONDS_DURATION)                                                        \
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
		virtual ~IBudget() = default;
		virtual void update() = 0;
		virtual void sleep() = 0;
		virtual void wake() = 0;
	};
// some bbudget considerations (^options+)
#define REAL long double
#define CLOCK chrono::system_clock
#define SECONDS nano
#define CHRONO_SECONDS chrono::nanoseconds
#define HISTORY_SIZE 38
#define SECONDS_DURATION chrono::duration<REAL, SECONDS>
#define DBudget zg::budget::IBudget<SECONDS_DURATION>
#define QUEUE_VALUE pair<CLOCK::time_point, SECONDS_DURATION>
#define QUEUE queue<QUEUE_VALUE>
	struct FBudget;
} // namespace zg::budget
namespace zg::budget
{
	/*|phase|*/
	/**
	 *
	 * @brief Zeungines' evolving history implementation of DBudget
	 *
	 */
	template <typename Clock = CLOCK, typename Real = REAL,
						typename TimePoint = chrono::time_point<CLOCK, CHRONO_SECONDS>, typename SecondsDuration = SECONDS_DURATION>
	struct ZBudget : DBudget
	{
		friend FBudget;

	protected:
		inline static SecondsDuration ZeroSecondsDuration = SecondsDuration(0.0);
		constexpr static bool m_AutoCalculateAverage = true;
		inline static SecondsDuration OneSecondsDuration = SecondsDuration(1.0);

	public:
		ZBudget(const size_t historySize, const SecondsDuration BudgetTime, bool instantStart = false,
						string_view budgetName = "", bool serializeHistory = false, filesystem::path serializeDirectory = {}) :
				m_BudgetTime(BudgetTime), m_serializeHistory(serializeHistory), m_serializeDirectory(serializeDirectory),
				m_chunkID(calculateChunkID()), m_historySize(historySize), m_instantStart(instantStart)
		{
			m_IsZgBudgetRealBegin = m_BudgetTime;
			// if (m_serializeHistory && m_chunkID)
			// {
			// 	// loadChunk();
			// }
			pushHistory();
		};
		~ZBudget()
		{
			if (m_serializeHistory)
			{
				// saveChunk();
			}
		}
		void update() override
		{
			if (!m_AutoCalculateAverage)
				return;
			auto& history_tuple = m_History.front();
			auto& zslept = get<4>(history_tuple);
			if (!zslept && !m_instantStart)
			{
				unique_lock lock(mTx);
				cv.wait_for(lock, nextBudgetTime(), [&] { return wakezwakez; });
				wakezwakez = false;
				zslept = true;
			}
			auto& begin = get<0>(history_tuple);
			auto& end = get<1>(history_tuple);
			auto& seconds = get<2>(history_tuple);
			Real beginTSE = begin.time_since_epoch().count();
			Real endTSE = end.time_since_epoch().count();
			if (beginTSE == 0)
			{
				begin = CLOCK::now();
			}
			else if (m_IsZSle_p && endTSE == 0)
			{
				end = CLOCK::now();
				auto nsduration = end - begin;
				seconds = end - begin;
				CalculateAverageBudget();
				auto wake = end + m_IsZgBudgetReal;
				m_IsZgBudgetWake = chrono::time_point_cast<chrono::nanoseconds>(wake);
				m_IsZSle_p = false;
				if (m_serializeHistory && m_History.size() == m_historySize)
				{
					// saveChunk();
				}
				pushHistory();
				return;
			}
			else
			{
				auto mid = CLOCK::now();
				auto nsduration = mid - begin;
				seconds = mid - begin;
				if (m_serializeHistory)
				{
					get<3>(history_tuple).push({mid, seconds});
				}
			}
			return;
		}
		SecondsDuration operator/(SecondsDuration a)
		{
			auto b = m_IsZgBudgetReal / a;
			SecondsDuration c = SecondsDuration(b);
			m_IsZgBudgetReal = SecondsDuration(m_IsZgBudgetReal - c);
			return c;
		}
		void wake() override
		{
			{
				unique_lock lock(mTx);
				wakezwakez = true;
			}
			cv.notify_one();
		}
		void sleep() override
		{
			m_IsZSle_p = true;
			update();
		}

	private:
		SecondsDuration m_BudgetTime;
		bool m_serializeHistory = false;
		filesystem::path m_serializeDirectory;
		size_t m_chunkID = 0;
		bool m_IsZSle_p = false;
		size_t m_historySize = HISTORY_SIZE;
		// size_t m_HistoryIndex = 0;
		// size_t m_HistoryTotalLength = 0;
		SecondsDuration m_IsZgBudgetReal;
		SecondsDuration m_IsZgBudgetRealBegin;
		TimePoint m_IsZgBudgetWake;
		using HistoryItem = tuple<TimePoint, TimePoint, SecondsDuration, queue<pair<TimePoint, SecondsDuration>>, bool>;
		deque<HistoryItem> m_History;
		condition_variable cv;
		mutex mTx;
		bool m_instantStart;
		bool wakezwakez = false;
		REAL m_sleepDiff = 50'000.0;
		void CalculateAverageBudget()
		{
			m_IsZgBudgetReal = nextBudgetTime();
		}
		// size_t ComputeHistoryEvolvedIndex()
		// {
		// 	if (m_HistoryIndex < HistorySize)
		// 	{
		// 		auto indexHash = getHistoryIndexHash();
		// 		auto resolvedHistoryIndex = calculateResolvedHistoryIndex(indexHash);
		// 		return historyEvolvedIndexMap[indexHash] = resolvedHistoryIndex;
		// 	}
		// 	return 0;
		// }
		// size_t getHistoryIndexHash() { return 0; }
		// size_t calculateResolvedHistoryIndex(size_t indexHash) { return 0; }
		// bool saveChunk()
		// {
		// 	zgfilesystem::File chunkFile(m_serializeDirectory / ("zgb_chunk_" + to_string(++m_chunkID)),
		// 															 enums::EFileLocation::Absolute, "w");
		// 	zgfilesystem::Serial serial(chunkFile.fileStream, chunkFile.fileStream);
		// 	serial << m_HistoryIndex << m_HistoryTotalLength << m_History.size();
		// 	for (auto& historyRecord : m_History)
		// 	{
		// 		auto& que = get<3>(historyRecord);
		// 		serial << get<0>(historyRecord) << get<1>(historyRecord) << get<2>(historyRecord) << que;
		// 		while (!que.empty())
		// 			que.pop();
		// 	}
		// 	return true;
		// }
		// bool loadChunk()
		// {
		// 	auto historySize = m_History.size();
		// 	zgfilesystem::File chunkFile(m_serializeDirectory / ("zgb_chunk_" + to_string(m_chunkID)),
		// 															 enums::EFileLocation::Absolute, "r");
		// 	zgfilesystem::Serial serial(chunkFile.fileStream, chunkFile.fileStream);
		// 	serial >> m_HistoryIndex >> m_HistoryTotalLength >> historySize;
		// 	assert(historySize <= HistorySize);
		// 	for (size_t count = 1; count <= historySize; count++)
		// 	{
		// 		auto& historyRecord = m_History[count - 1];
		// 		serial >> get<0>(historyRecord) >> get<1>(historyRecord) >> get<2>(historyRecord) >> get<3>(historyRecord);
		// 	}
		// 	if (m_HistoryIndex < HistorySize)
		// 	{
		// 		--m_chunkID;
		// 	}
		// 	return true;
		// }
		size_t calculateChunkID()
		{
			if (!m_serializeHistory)
				return 0;
			zgfilesystem::Directory::ensureExists(m_serializeDirectory);
			zgfilesystem::Directory chunkDirectory(m_serializeDirectory);
			size_t chunks = 0;
			for (auto& entry : chunkDirectory.entries)
			{
				auto entryString = entry.second.string();
				if (entryString.find("zgb_chunk_"))
				{
					chunks++;
				}
			}
			return chunks;
		}

		SecondsDuration nextBudgetTime()
		{
			auto now = chrono::duration_cast<CHRONO_SECONDS>(CLOCK::now().time_since_epoch());
			auto budgetCountNs = chrono::duration_cast<CHRONO_SECONDS>(m_BudgetTime).count();
			auto nsQuantized = SecondsDuration(m_BudgetTime.count() - (now.count() % budgetCountNs) - (m_sleepDiff / 2));
			return nsQuantized;
		}

		void pushHistory()
		{
			HistoryItem historyItem{{}, {}, {}, {}, false};
			auto& _1 = get<0>(historyItem);
			auto& _2 = get<1>(historyItem);
			auto& _3 = get<2>(historyItem);
			memset(&_1, 0, sizeof(_1));
			memset(&_2, 0, sizeof(_2));
			memset(&_3, 0, sizeof(_3));
			m_History.push_front(historyItem);
		}
	};
} // namespace zg::budget
