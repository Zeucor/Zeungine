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
#include <zg/pureconstcharstreamcode.hpp>
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
	 * @tparam SecondsDuration Type using std::chrono::duration<Real, StdSec>
	 */
	template <typename SecondsDuration>
	struct IBudget
	{
		virtual ~IBudget() = default;
		virtual bool begin() = 0;
		virtual bool tick(bool true_if_false_or___true_if_true_and_now_at_or_after_next_budget__) = 0;
		virtual SecondsDuration getCurrentBudget() = 0;
		virtual SecondsDuration getBeginningBudget() = 0;
		virtual SecondsDuration durationToWaitTilNextBudget() = 0;
		virtual SecondsDuration operator/(SecondsDuration a) = 0;
		virtual void end() = 0;
		virtual void sleep() = 0;
		virtual void wake() = 0;
	};
	// some budget applications (-options+
	struct FBudget;
	/*|phase|*/
	/**
	 *
	 * @brief Zeungines' evolving history implementation of DBudget
	 *
	 */
	template <typename Clock = SYS_CLOCK, typename TimePoint = NANO_TIMEPOINT,
						typename SecondsDuration = NANOSECONDS_DURATION, typename Real = LD_REAL>
	struct ZBudget : DBudget
	{
		friend FBudget;

	private:
		void zsleep()
		{
			auto& history_tuple = m_History.front();
			auto& zslept = std::get<4>(history_tuple);
			if (zslept)
				return;
			std::unique_lock lock(mTx);
			cv.wait_until(lock, m_IsNextBudgetWakeAtTimePoint, [&] { return m_wakezwakez; });
			m_wakezwakez = false;
			zslept = true; // this only gets set once forever(y) HistoryItem
		}

	public:
		ZBudget(const SecondsDuration BudgetTime, const size_t historySize = 1, bool instantStart = false,
						bool sleepAtSleep = false, const std::string& budgetName = "", bool serializeHistory = false,
						std::filesystem::path serializeDirectory = {}) :
				m_BudgetTime(BudgetTime), m_serializeHistory(serializeHistory), m_serializeDirectory(serializeDirectory),
				m_chunkID(calculateChunkID()), m_budgetName(budgetName), m_historySize(historySize),
				m_instantStart(instantStart), m_sleeponsleep(sleepAtSleep), m_budgetCountNs(m_BudgetTime.count())
		{
			m_IsBeginningZgBudget = m_BudgetTime;
			// if (m_serializeHistory && m_chunkID)
			// {
			// 	// loadChunk();
			// }
			setNextBudgetWakeAtTimePoint();
			pushFrontHistory();
		};
		~ZBudget()
		{
			if (m_serializeHistory)
			{
				// saveChunk();
			}
		}
		bool begin() override
		{
			if (!m_instantStart && !m_sleeponsleep)
			{
				zsleep();
			}
			std::unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& begin = std::get<0>(history_tuple);
			auto __now = NANO_TIMEPOINT_CAST(SYS_CLOCK::now());
			if (m_instantStart && !m_sleeponsleep)
			{
				if (__now >= m_IsNextBudgetWakeAtTimePoint)
				{
					begin = __now;
					return true;
				}
			}
			auto& zslept = std::get<4>(history_tuple);
			return zslept;
		}
		bool tick(bool true_if_false_or___true_if_true_and_now_at_or_after_next_budget__ = false) override
		{
			auto t_if = true_if_false_or___true_if_true_and_now_at_or_after_next_budget__;
			std::unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& seconds = std::get<2>(history_tuple);
			auto& begin = std::get<0>(history_tuple);
			auto mid = SYS_CLOCK::now();
			auto nsduration = mid - begin;
			seconds = mid - begin;
			if (m_serializeHistory)
			{
				get<3>(history_tuple).push({mid, seconds});
			}
			if (t_if)
			{
				if (mid >= m_IsNextBudgetWakeAtTimePoint)
				{
					return true;
				}
				GONNA_JO_THIS_THIS_JONE(peaceccsc::now());
				return false; //   |  :.`*
			}
			return true;
		}
		SecondsDuration getCurrentBudget() override
		{
			std::unique_lock lock(mTx);
			return m_IsZgBudget;
		}
		SecondsDuration getBeginningBudget() override
		{
			std::unique_lock lock(mTx);
			return m_IsBeginningZgBudget;
		}
		SecondsDuration durationToWaitTilNextBudget() override
		{
			std::unique_lock lock(mTx);
			auto now = std::chrono::duration_cast<CHRONO_NANOSECONDS>(SYS_CLOCK::now().time_since_epoch());
			auto m_budgetCountNs = std::chrono::duration_cast<CHRONO_NANOSECONDS>(m_BudgetTime).count();
			auto nsQuantized = SecondsDuration(m_BudgetTime.count() - (now.count() % m_budgetCountNs));
			return nsQuantized;
		}
		SecondsDuration operator/(SecondsDuration a) override
		{
			std::unique_lock lock(mTx);
			auto b = m_IsZgBudget / a;
			SecondsDuration c = SecondsDuration(b);
			m_IsZgBudget = SecondsDuration(m_IsZgBudget - c);
			return c;
		}
		void end() override
		{
			unique_lock lock(mTx);
			auto& history_tuple = m_History.front();
			auto& end = std::get<1>(history_tuple);
			end = setNextBudgetWakeAtTimePoint();
			auto& seconds = std::get<2>(history_tuple);
			auto& begin = std::get<0>(history_tuple);
			seconds = end - begin;
		}
		void sleep() override
		{
			if (m_sleeponsleep)
			{
				zsleep();
			}
			{
				std::unique_lock lock(mTx);
				pushFrontHistory();
				if (!m_serializeHistory)
				{
					m_History.pop_back();
				}
			}
		}
		void wake() override
		{
			{
				std::unique_lock lock(mTx);
				m_wakezwakez = true;
			}
			cv.notify_one();
		}

	private:
		SecondsDuration m_BudgetTime;
		bool m_serializeHistory = false;
		std::filesystem::path m_serializeDirectory;
		size_t m_chunkID = 0;
		std::string m_budgetName;
		size_t m_historySize = 38;
		// map<CreatorID, unique_ptr<deque<pair<CreatorID, glm::vec4>>>> creatorSpaceTimeDoubleEndejQueue;
		// unordered_2multiset8<CreatorID> realCreatorIDSet
		SecondsDuration m_IsZgBudget;
		SecondsDuration m_IsBeginningZgBudget;
		TimePoint m_IsNextBudgetWakeAtTimePoint;
		size_t m_budgetCountNs;
		bool m_sleeponsleep;
		using HistoryItem =
			std::tuple<TimePoint, TimePoint, SecondsDuration, std::queue<std::pair<TimePoint, SecondsDuration>>, bool>;
		std::deque<HistoryItem> m_History;
		std::condition_variable cv;
		std::mutex mTx;
		bool m_instantStart;
		bool m_wakezwakez = false;
		bool m_workedOvertime = true;

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
			auto& _1 = std::get<0>(historyItem);
			auto& _2 = std::get<1>(historyItem);
			auto& _3 = std::get<2>(historyItem);
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
		// 		auto& que = std::get<3>(historyRecord);
		// 		serial << std::get<0>(historyRecord) << std::get<1>(historyRecord) << std::get<2>(historyRecord) << que;
		// 		while (!que.empty())
		// 			que.pop();
		// 	}
		// 	return true;
		// }
		bool loadChunk()
		{
			auto historySize = m_History.size();
			zgfilesystem::File chunkFile(m_serializeDirectory / ("zgb_chunk_" + to_string(m_chunkID)),
																	 enums::EFileLocation::Absolute, "r");
			zgfilesystem::Serial serial(chunkFile.fileStream, chunkFile.fileStream);
			serial >> m_historySize;
			// for (size_t count = 1; count <= historySize; count++)
			// {
			// 	auto& historyRecord = m_History[count - 1];
			// 	serial >> std::get<0>(historyRecord) >> std::get<1>(historyRecord) >> std::get<2>(historyRecord) >>
			// std::get<3>(historyRecord) >> std::get<4>(historyRecord);
			// }
			// if (m_HistoryIndex < HistorySize)
			// {
			// 	--m_chunkID;
			// }
			return true;
		}

		TimePoint setNextBudgetWakeAtTimePoint()
		{
			auto __now = NANO_TIMEPOINT_CAST(SYS_CLOCK::now());
			auto nsQuantized = SecondsDuration(m_budgetCountNs - (__now.time_since_epoch().count() % m_budgetCountNs));
			m_IsZgBudget = nsQuantized;
			m_IsNextBudgetWakeAtTimePoint = NANO_TIMEPOINT_CAST(__now + m_IsZgBudget);
			return __now;
		}
	};
} // namespace zg::budget
