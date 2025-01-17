#pragma once

#include <chrono>
#include <string>
#include <cstdio>
#include <nano/stats.h>
#include <nano/numeric.h>

namespace nano
{
    using picoseconds_t = std::chrono::duration<long long, std::pico>;
    using nanoseconds_t = std::chrono::duration<long long, std::nano>;
    using microseconds_t = std::chrono::duration<long long, std::micro>;
    using milliseconds_t = std::chrono::duration<long long, std::milli>;
    using seconds_t = std::chrono::duration<long long>;

    using timepoint_t = std::chrono::high_resolution_clock::time_point;

    ///
    /// \brief utility to measure duration.
    ///
    class timer_t
    {
    public:
        ///
        /// \brief constructor
        ///
        timer_t() : m_start(now()) {}

        ///
        /// \brief reset the current time point
        ///
        void reset() { m_start = now(); }

        ///
        /// \brief retrieve the elapsed time as a string
        ///
        std::string elapsed() const { return elapsed(static_cast<int>(this->milliseconds().count())); }

        ///
        /// \brief retrieve the elapsed time in seconds
        ///
        seconds_t seconds() const { return duration<seconds_t>(); }

        ///
        /// \brief retrieve the elapsed time in milliseconds
        ///
        milliseconds_t milliseconds() const { return duration<milliseconds_t>(); }

        ///
        /// \brief retrieve the elapsed time in microseconds
        ///
        microseconds_t microseconds() const { return duration<microseconds_t>(); }

        ///
        /// \brief retrieve the elapsed time in nanoseconds
        ///
        nanoseconds_t nanoseconds() const { return duration<nanoseconds_t>(); }

    private:

        static timepoint_t now()
        {
            return std::chrono::high_resolution_clock::now();
        }

        static void append(std::string& str, const char* format, const int value)
        {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), format, value);
            str.append(buffer);
        }

        template <typename tduration>
        tduration duration() const
        {
            return std::chrono::duration_cast<tduration>(now() - m_start);
        }

        static std::string elapsed(int milliseconds)
        {
            static constexpr int size_second = 1000;
            static constexpr int size_minute = 60 * size_second;
            static constexpr int size_hour = 60 * size_minute;
            static constexpr int size_day = 24 * size_hour;

            const auto days = milliseconds / size_day; milliseconds -= days * size_day;
            const auto hours = milliseconds / size_hour; milliseconds -= hours * size_hour;
            const auto minutes = milliseconds / size_minute; milliseconds -= minutes * size_minute;
            const auto seconds = milliseconds / size_second; milliseconds -= seconds * size_second;

            std::string str;
            if (days > 0)
            {
                append(str, "%id:", days);
            }
            if (days > 0 || hours > 0)
            {
                append(str, "%.2ih:", hours);
            }
            if (days > 0 || hours > 0 || minutes > 0)
            {
                append(str, "%.2im:", minutes);
            }
            if (days > 0 || hours > 0 || minutes > 0 || seconds > 0)
            {
                append(str, "%.2is:", seconds);
            }
            append(str, "%.3ims", milliseconds);

            return str;
        }

        // attributes
        timepoint_t     m_start;        ///< starting time point
    };

    ///
    /// \brief robustly measure a function call (in the given time units).
    ///
    template <typename tunits, typename toperator>
    tunits measure(const toperator& op, const size_t trials,
        const size_t min_trial_iterations = 1,
        const microseconds_t min_trial_duration = microseconds_t(1000))
    {
        const auto run_opx = [&] (const size_t times)
        {
            const timer_t timer;
            for (size_t i = 0; i < times; ++ i)
            {
                op();
            }
            return timer.microseconds();
        };

        const auto run_trial = [&] (const size_t times)
        {
            return picoseconds_t(nano::idiv(run_opx(times).count() * 1000 * 1000, times));
        };

        // calibrate the number of function calls to achieve the minimum time resolution
        size_t trial_iterations = std::max(size_t(1), min_trial_iterations);
        for (microseconds_t usecs(0); usecs < min_trial_duration; trial_iterations *= 2)
        {
            usecs = run_opx(trial_iterations);
        }

        // measure multiple times for robustness
        picoseconds_t duration = run_trial(trial_iterations);
        for (size_t t = 1; t < trials; ++ t)
        {
            duration = std::min(duration, run_trial(trial_iterations));
        }

        return std::chrono::duration_cast<tunits>(duration);
    }

    ///
    /// \brief compute GFLOPS (giga floating point operations per seconds)
    ///     given the number of FLOPs run in the given duration.
    ///
    template <typename tinteger, typename tduration>
    int64_t gflops(const tinteger flops, const tduration& duration)
    {
        const auto div = static_cast<int64_t>(std::chrono::duration_cast<picoseconds_t>(duration).count());
        return nano::idiv(static_cast<int64_t>(flops) * 1000, std::max(div, int64_t(1)));
    }

    ///
    /// \brief accumulate time measurements for a given operation of given complexity (aka flops).
    ///
    class probe_t
    {
    public:

        probe_t(std::string basename = std::string(),
                std::string fullname = std::string(),
                const int64_t flops = 1) :
                m_basename(std::move(basename)),
                m_fullname(std::move(fullname)),
                m_flops(flops)
        {
        }

        template <typename toperator>
        void measure(const toperator& op, const int64_t count = 1)
        {
            assert(count > 0);
            const timer_t timer;
            op();
            m_timings(timer.nanoseconds().count() / count);
        }

        operator bool() const { return m_timings; }
        const auto& timings() const { return m_timings; }

        const auto& basename() const { return m_basename; }
        const auto& fullname() const { return m_fullname; }

        auto flops() const { return m_flops; }
        auto kflops() const { return m_flops / 1024; }
        auto gflops() const { return nano::gflops(flops(), nanoseconds_t(static_cast<int64_t>(timings().min()))); }

    private:

        // attributes
        std::string     m_basename;             ///<
        std::string     m_fullname;             ///<
        int64_t         m_flops;                ///< number of floating point operations per call
        stats_t         m_timings;              ///< time measurements
    };

    using probes_t = std::vector<probe_t>;
}
