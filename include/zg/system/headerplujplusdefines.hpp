#pragma once
#define LD_REAL long double
#define L_LREAL long long
#define DBudget zg::budget::IBudget<NANOSECONDS_DURATION>
#define SYS_CLOCK std::chrono::system_clock
#define CHRONO_SECONDS std::chrono::seconds
#define NANOSECONDS std::nano
#define CHRONO_NANOSECONDS std::chrono::nanoseconds
#define ATTOSECONDS std::atto
#define CHRONO_ATTOSECONDS std::chrono::attoseconds
#define NANOSECONDS_DURATION std::chrono::duration<LD_REAL, NANOSECONDS>
#define ATTOSECONDS_DURATION std::chrono::duration<L_LREAL, ATTOSECONDS>
#define SECONDS_DURATION_CAST std::chrono::duration_cast<CHRONO_SECONDS, LD_REAL>
#define NANOSECONDS_DURATION_CAST std::chrono::duration_cast<CHRONO_NANOSECONDS, LD_REAL>
#define ATTOSECONDS_DURATION_CAST std::chrono::duration_cast<CHRONO_ATTOSECONDS, L_LREAL>
#define NANO_TIMEPOINT std::chrono::time_point<SYS_CLOCK, CHRONO_NANOSECONDS>
#define ATTO_TIMEPOINT std::chrono::time_point<SYS_CLOCK, CHRONO_ATTOSECONDS>
#define NANO_TIMEPOINT_CAST std::chrono::time_point_cast<CHRONO_NANOSECONDS>
#define ATTO_TIMEPOINT_CAST std::chrono::time_point_cast<CHRONO_ATTOSECONDS>
#define QUEUE_PAIR std::pair<NANO_TIMEPOINT, NANOSECONDS_DURATION>
#define QUEUE std::queue<QUEUE_PAIR>