/**
 * @file Budget.hpp
 * @author ZeunO8 (mr.steven.french@gmail.com)
 * @brief  Declare a program budget, use (begin(), tick(), end(), sleep()) to keep track of your timepoints
 * [zeungine attempt at tracking & pacing algorithms all while getting the correct amount of loop sleep]
 * @version 0.421
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
// include <flat_set>
//  all creator ids playing back eachother in a serial Universe peace moje enabled.
using namespace zg::system;
using namespace std;
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
		virtual ~IBudget() = default;
		virtual void begin() = 0;
		virtual void tick() = 0;
		virtual SecondsDuration getCurrentBudget() = 0;
		virtual SecondsDuration getBeginningBudget() = 0;
		virtual SecondsDuration durationToWaitTilNextBudget() = 0;
		virtual SecondsDuration operator/(SecondsDuration a) = 0;
		virtual void end() = 0;
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

	private:
		void zsleep()
		{
			auto& history_tuple = m_History.front();
			auto& zslept = get<4>(history_tuple);
			if (zslept)
				return;
			unique_lock lock(mTx);
			cv.wait_until(lock, m_IsNextBudgetWakeAtTimePoint, [&] { return wakezwakez; });
			wakezwakez = false;
			zslept = true; // this only gets set once forever(y) HistoryItem
		}

	public:
		ZBudget(const size_t historySize, const SecondsDuration BudgetTime, bool instantStart = false,
						bool sleepAtSleep = false, string_view budgetName = "", bool serializeHistory = false,
						filesystem::path serializeDirectory = {}) :
				m_BudgetTime(BudgetTime), m_serializeHistory(serializeHistory), m_serializeDirectory(serializeDirectory),
				m_chunkID(calculateChunkID()), m_historySize(historySize), m_instantStart(instantStart),
				m_sleeponsleep(sleepAtSleep), m_budgetCountNs(m_BudgetTime.count())
		{
			m_IsBeginningZgBudget = m_BudgetTime;
			// if (m_serializeHistory && m_chunkID)
			// {
			// 	// loadChunk();
			// }
			pushFrontHistory();
		};
		~ZBudget()
		{
			if (m_serializeHistory)
			{
				// saveChunk();
			}
		}
		void begin() override
		{
			if (!m_instantStart && !m_sleeponsleep)
			{
				zsleep();
			}
			unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& begin = get<0>(history_tuple);
			begin = CLOCK::now();
		}
		void tick() override
		{
			unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& seconds = get<2>(history_tuple);
			auto& begin = get<0>(history_tuple);
			auto mid = CLOCK::now();
			auto nsduration = mid - begin;
			seconds = mid - begin;
			if (m_serializeHistory)
			{
				get<3>(history_tuple).push({mid, seconds});
			}
			return;
		}
		SecondsDuration getCurrentBudget() override
		{
			unique_lock lock(mTx);
			return m_IsZgBudget;
		}
		SecondsDuration getBeginningBudget() override
		{
			unique_lock lock(mTx);
			return m_IsBeginningZgBudget;
		}
		SecondsDuration durationToWaitTilNextBudget() override
		{
			unique_lock lock(mTx);
			auto now = chrono::duration_cast<CHRONO_SECONDS>(CLOCK::now().time_since_epoch());
			auto m_budgetCountNs = chrono::duration_cast<CHRONO_SECONDS>(m_BudgetTime).count();
			auto nsQuantized = SecondsDuration(m_BudgetTime.count() - (now.count() % m_budgetCountNs));
			return nsQuantized;
		}
		SecondsDuration operator/(SecondsDuration a) override
		{
			unique_lock lock(mTx);
			auto b = m_IsZgBudget / a;
			SecondsDuration c = SecondsDuration(b);
			m_IsZgBudget = SecondsDuration(m_IsZgBudget - c);
			return c;
		}
		void end() override
		{
			unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& end = get<1>(history_tuple);
			end = CLOCK::now();
			auto& seconds = get<2>(history_tuple);
			auto& begin = get<0>(history_tuple);
			seconds = end - begin;
			auto nsQuantized = SecondsDuration(m_budgetCountNs - (end.time_since_epoch().count() % m_budgetCountNs));
			m_IsZgBudget = nsQuantized;
			m_IsNextBudgetWakeAtTimePoint = chrono::time_point_cast<chrono::nanoseconds>(end + m_IsZgBudget);
		}
		void sleep() override
		{
			{
				unique_lock lock(mTx);
				pushFrontHistory();
				if (!m_serializeHistory)
				{
					m_History.pop_back();
				}
			}
			if (m_sleeponsleep)
			{
				zsleep();
			}
		}
		void wake() override
		{
			{
				unique_lock lock(mTx);
				wakezwakez = true;
			}
			cv.notify_one();
		}

	private:
		SecondsDuration m_BudgetTime;
		bool m_serializeHistory = false;
		filesystem::path m_serializeDirectory;
		size_t m_chunkID = 0;
		size_t m_historySize = HISTORY_SIZE;
		// std::map<CreatorID, std::unique_ptr<deque<std::pair<CreatorID, glm::vec4>>>> creatorSpaceTimeDoubleEndejQueue;
		// flat8set<CreatorID> realCreatorIDSet
		SecondsDuration m_IsZgBudget;
		SecondsDuration m_IsBeginningZgBudget;
		TimePoint m_IsNextBudgetWakeAtTimePoint;
		size_t m_budgetCountNs;
		bool m_sleeponsleep;
		using HistoryItem = tuple<TimePoint, TimePoint, SecondsDuration, queue<pair<TimePoint, SecondsDuration>>, bool>;
		deque<HistoryItem> m_History;
		condition_variable cv;
		mutex mTx;
		bool m_instantStart;
		bool wakezwakez = false;

	private:
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
		void pushFrontHistory()
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
	};
} // namespace zg::budget
