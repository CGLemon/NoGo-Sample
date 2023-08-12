#ifndef TIME_MANAGER_H_INCLUDE
#define TIME_MANAGER_H_INCLUDE

#include <array>
#include <chrono>
#include "game_state.h"

class Time {
public:
    Time();

    static int timediff_centis(Time start, Time end);

    static double timediff_seconds(Time start, Time end);

private:
    std::chrono::steady_clock::time_point m_time;
};

class TimeManager {
public:
    TimeManager();

    void reset();
    void time_setting(int main_time, int lag_buffer=1);

    void clock(int color, GameState &state);
    float get_thinking_time(int color) const;
    bool should_stop(int color) const;
    void stop(int color);

    float get_time_left(int color) const;

private:
    std::array<int, 2> m_remining_times;
    std::array<Time, 2> m_times;
    std::array<int, 2> m_thinking_times;

    int m_main_time;
    int m_lag_buffer;
};

#endif
