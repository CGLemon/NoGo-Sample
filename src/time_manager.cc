#include "time_manager.h"

Time::Time() {
    m_time = std::chrono::steady_clock::now();
}

int Time::timediff_centis(const Time start, const Time end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (end.m_time - start.m_time).count() / 10;
}

double Time::timediff_seconds(const Time start, const Time end) {
    return std::chrono::duration<double>(end.m_time - start.m_time).count();
}

TimeManager::TimeManager() {
    time_setting(7 * 24 * 60 * 60); // one week
    reset();
}

void TimeManager::reset() {
    m_remining_times[0] = m_main_time;
    m_remining_times[1] = m_main_time;
}

void TimeManager::time_setting(int main_time, int lag_buffer) {
    m_lag_buffer = 100 * lag_buffer; // second -> centisecond
    m_main_time =  100 * main_time; // second -> centisecond
    reset();
}

void TimeManager::clock(int color, GameState &state) {
    m_times[color] = Time();
    int board_size = state.get_board_size();
    int remining_centis = m_remining_times[color];

    int c_max_ply = 0.5 * (board_size * board_size);
    int c_base_ply = 0.25 * (board_size * board_size);
    int thinking_centis =
        remining_centis / (
            c_base_ply + std::max(c_max_ply - state.get_movenum(), 0));
    m_thinking_times[color] = thinking_centis;
}

float TimeManager::get_thinking_time(int color) const {
    return (float)m_thinking_times[color]/100.f;
}

bool TimeManager::should_stop(int color) const {
    Time start = m_times[color];
    Time now;

    int thinking_centis = m_thinking_times[color];
    int elapsed_centis = Time::timediff_centis(start, now);

    if (elapsed_centis > thinking_centis) {
        return true;
    }
    return false;
}

void TimeManager::stop(int color) {
    Time start = m_times[color];
    Time end;
    int elapsed_centis = Time::timediff_centis(start, end);
    m_remining_times[color] -= elapsed_centis;
}

float TimeManager::get_time_left(int color) const {
    return (float)m_remining_times[color]/100.f;
}
