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
#include <zg/zgfilesystem/File.hpp>
#include <zg/zgfilesystem/Serial.hpp>
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
#define SECONDS nano
#define HISTORY_SIZE 38
#define SECONDS_DURATION chrono::duration<REAL, SECONDS>
#define DBudget zg::budget::IBudget<SECONDS_DURATION>
#define QUEUE_VALUE pair<CLOCK::time_point, SECONDS_DURATION>
#define QUEUE queue<QUEUE_VALUE>
	struct FBudget;
}
namespace std
{
	ostream& operator<<(ostream& os, const SECONDS_DURATION& value)
	{
		os.write((const char *)&value, sizeof(value));
		return os;
	}
	ostream& operator<<(ostream& os, const CLOCK::time_point& value)
	{
		os.write((const char *)&value, sizeof(value));
		return os;
	}
	ostream& operator<<(ostream& os, const QUEUE_VALUE& value)
	{
		os.write((const char *)&(value.first), sizeof(value.first));
		os.write((const char *)&(value.second), sizeof(value.second));;
		return os;
	}
	ostream& operator<<(ostream& os, const QUEUE& que)
	{
		os << que.size() << endl;
		QUEUE st = que;
		while (!st.empty())
		{
			auto marker = st.front();
			os << marker;
			st.pop();
		}
		return os;
	}
	istream& operator>>(istream& is, SECONDS_DURATION& value)
	{
		is.read((char *)&value, sizeof(value));
		return is;
	}
	istream& operator>>(istream& is, CLOCK::time_point& value)
	{
		is.read((char *)&value, sizeof(value));
		return is;
	}
	istream& operator>>(istream& is, QUEUE_VALUE& value)
	{
		is.read((char *)&(value.first), sizeof(value.first));
		is.read((char *)&(value.second), sizeof(value.second));	
		return is;
	}
	istream& operator>>(istream& is, QUEUE& que)
	{
		auto size_ = que.size();
		is >> size_;
		for (int count = 1; count <= size_; ++count)
		{
			QUEUE_VALUE value;
			is >> value;
			que.push(value);
		}
		return is;
	}
}
namespace zg::budget
{
/*|phase|*/
	/**
	 *
	 * @brief Zeungines' evolving history implementation of DBudget
	 *
	 */
	template <size_t HistorySize = HISTORY_SIZE, typename Clock = CLOCK, typename Real = REAL,
						typename TimePoint = CLOCK::time_point, typename SecondsDuration = SECONDS_DURATION>
	struct ZBudget : DBudget
	{
		friend FBudget;

	protected:
		inline static SecondsDuration ZeroSecondsDuration = SecondsDuration(0.0);
		constexpr static bool m_AutoCalculateAverage = true;
		inline static SecondsDuration OneSecondsDuration = SecondsDuration(1.0);

	public:
		ZBudget(const SecondsDuration BudgetTime, string_view budgetName = "", bool serializeHistory = false, filesystem::path serializeDirectory = {}) :
			m_BudgetTime(BudgetTime),
			m_serializeHistory(serializeHistory),
			m_serializeDirectory(serializeDirectory),
			m_chunkID(calculateChunkID())
		{
			if (m_serializeHistory && m_chunkID)
			{
				loadChunk();
			}
		};
		~ZBudget()
		{
			if (m_serializeHistory)
			{
				saveChunk();
			}
		}
		SecondsDuration update(bool print = false)
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
					if (print)
						cerr << "zupdate() is at start of 2pass @ " << begin << endl;
					return m_IsZgBudgetRealBegin;
				}
				else if (m_IsZSle_p && endTSE == 0)
				{
					end = CLOCK::now();
					auto nsduration = end - begin;
					seconds = end - begin;
					auto avgBud = CalculateAverageBudget(print);
					m_HistoryIndex++;
					if (m_HistoryTotalLength < HistorySize)
						m_HistoryTotalLength++;
					if (print)
						cerr << "zupdate() reached end of 2pass @ " << end << ", nanoseconds(t): " << nsduration << ", zbudget is now: " << avgBud << endl;
					m_IsZSle_p = false;
					return avgBud;
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
					if (print)
						cerr << "zupdate() went update in 2pass @ " << mid << ", nanoseconds(@): " << nsduration << endl;
				}
			}
			else
			{
				m_HistoryIndex = ComputeHistoryEvolvedIndex();
				if (m_serializeHistory)
				{
					saveChunk();
					for (auto i = m_HistoryIndex; i < HistorySize; ++i)
					{
						auto& history_record = m_History[i];
						auto& _0 = get<0>(history_record);
						auto& _1 = get<1>(history_record);
						auto& _2 = get<2>(history_record);
						auto& _3 = get<3>(history_record);
						memset(&_0, 0, sizeof(_0));
						memset(&_1, 0, sizeof(_1));
						memset(&_2, 0, sizeof(_2));
						while (!_3.empty())
							_3.pop();
					}
				}
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
		void zsleep(bool print = false)
		{
			m_IsZSle_p = true;
			update();
			if (print)
				cerr << "...    ?  zsle_p :&+    Cojpint@ " << m_IsZgBudgetReal << endl;
			this_thread::sleep_for(m_IsZgBudgetReal);
		}

	private:
		bool m_serializeHistory = false;
		filesystem::path m_serializeDirectory;
		size_t m_chunkID = 0;
		bool m_IsZSle_p = false;
		size_t m_HistoryIndex = 0;
		size_t m_HistoryTotalLength = 0;
		SecondsDuration m_BudgetTime;
		SecondsDuration m_IsZgBudgetReal;
		SecondsDuration m_IsZgBudgetRealBegin;
		array<tuple<TimePoint, TimePoint, SecondsDuration, queue<pair<TimePoint, SecondsDuration>>>, HistorySize> m_History;
		array<size_t, 1> historyEvolvedIndexMap;
		SecondsDuration CalculateAverageBudget(bool print = false)
		{
			auto total_avg_budget = ZeroSecondsDuration;
			auto& history_tuple = m_History[m_HistoryIndex];
			auto& secondsTakenByCook = get<2>(history_tuple);
			if (print)
				cerr << "zCalcAvg(): seconds tkn in loop@ " << secondsTakenByCook << endl;
			auto remainingBudget = m_BudgetTime - secondsTakenByCook;
			auto remainingBudgetCount = remainingBudget.count();
			if (print)
				cerr << "zCalcAvg(): remaining budget .:@ " << remainingBudget << endl;
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
			return 0;
		}
		size_t getHistoryIndexHash() { return 0; }
		size_t calculateResolvedHistoryIndex(size_t indexHash) { return 0; }
		bool saveChunk()
		{
			zgfilesystem::File chunkFile(m_serializeDirectory / ("zgb_chunk_" + to_string(++m_chunkID)), enums::EFileLocation::Absolute, "w");
			zgfilesystem::Serial serial(chunkFile.fileStream, chunkFile.fileStream);
			serial << m_HistoryIndex << m_HistoryTotalLength << m_History.size();
			for (auto& historyRecord : m_History)
			{
				auto& que = get<3>(historyRecord);
				serial << get<0>(historyRecord) << get<1>(historyRecord) << get<2>(historyRecord)
					<< que;
				while (!que.empty())
					que.pop();
			}
			return true;
		}
		bool loadChunk()
		{
			auto historySize = m_History.size();
			zgfilesystem::File chunkFile(m_serializeDirectory / ("zgb_chunk_" + to_string(m_chunkID)), enums::EFileLocation::Absolute, "r");
			zgfilesystem::Serial serial(chunkFile.fileStream, chunkFile.fileStream);
			serial >> m_HistoryIndex >> m_HistoryTotalLength >> historySize;
			assert(historySize <= HistorySize);
			for (size_t count = 1; count <= historySize; count++)
			{
				auto& historyRecord = m_History[count - 1];
				serial >> get<0>(historyRecord) >> get<1>(historyRecord) >> get<2>(historyRecord)
					>> get<3>(historyRecord);
			}
			return true;
		}
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
	};
} // namespace zg::budget
