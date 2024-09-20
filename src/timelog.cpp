#include "../include/state_machine.hpp"
#include <bits/chrono.h>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Aliases
using Clock = std::chrono::system_clock;
using Time = std::chrono::time_point<Clock>;
using CalendarDay = std::chrono::year_month_day;
template <> struct std::hash<CalendarDay> {};

// Timelog: A log containing entries of written content for a specific day.
class Timelog {
public:
  /*** Data Structures ***/
  // Entry: Written content that covers a time range.
  struct Entry {
    // Metadata: Extra info on the entry.
    struct Metadata {
      Time entry_time;
      Time last_viewed_time;
      Time last_modified_time;

      Metadata()
          : entry_time(Clock::now()), last_viewed_time(Clock::now()),
            last_modified_time(Clock::now()) {}
    };
    Time from;
    Time to;
    std::string content;
    Metadata metadata;

    Entry(Time from) : from(from), to(), content(), metadata() {}

    Entry(Time from, std::string content)
        : from(from), content(content), metadata() {}

    std::string to_string() { return std::string(); }
  };

  // EntryNotPresent: When an entry do not exist within the timelog.
  class EntryNotPresent : public std::out_of_range {
  public:
    EntryNotPresent() : std::out_of_range("Entry does not exist.") {}
    EntryNotPresent(std::string message) : std::out_of_range(message) {}
  };

  // EntryInvalidTime: When an entry isn't within the timelog's day.
  class EntryInvalidTime : public std::out_of_range {
  public:
    EntryInvalidTime()
        : std::out_of_range(
              "Entry's start time is outside of the timelog's specified day.") {
    }
  };

protected:
  /*** Members ***/
  CalendarDay calendar_day;
  std::vector<Entry> entries;

public:
  Timelog() {}
  Timelog(CalendarDay calendar_day) : calendar_day(calendar_day) {}

  // Add an entry to the timelog with the current time.
  // If current time is not within the timelog's day, throw exception.
  void add_entry(std::string content) {
    Clock::time_point time = Clock::now();
    if (!time_valid(time)) {
      throw EntryInvalidTime();
    }
    entries.push_back(Entry(time, content));
  }

  // Add an entry to the timelog with a specified time.
  // Throw if current time is not within the timelog's day.
  void add_entry(Clock::time_point time, std::string content) {
    if (!time_valid(time)) {
      throw EntryInvalidTime();
    }
    entries.push_back(Entry(time, content));
  }

  // Delete an entry from the timelog.
  // Throw if entry index is invalid.
  void delete_entry(size_t index) {
    if (!index_valid(index)) {
      throw EntryNotPresent();
    }
    entries.erase(entries.begin() + index);
  }

  // Delete a range of entries from the timelog.
  // Throw if range is invalid.
  void delete_entries(size_t start, size_t end) {
    if (start == end) {
      delete_entry(start);
    } else if (start > end) {
      delete_entries(end, start);
    } else if (!index_valid(start) || !index_valid(end)) {
      throw EntryNotPresent();
    } else {
      entries.erase(entries.begin() + start, entries.begin() + end);
    }
  }

  // Add to an entry's content.
  // Throw if entry index is invalid.
  void append_entry_content(size_t index, std::string content) {
    try {
      Entry *entry = get_entry(index);
      entry->content += content;
    } catch (EntryNotPresent &err) {
      throw err;
    }
  }

  // Replace an entry's content.
  // Throw if entry index is invalid.
  void replace_entry_content(size_t index, std::string content) {
    try {
      Entry *entry = get_entry(index);
      entry->content = content;
    } catch (EntryNotPresent &err) {
      throw err;
    }
  }

protected:
  // Get an entry based on it's index.
  // Throw if entry index is invalid.
  Entry *get_entry(size_t index) {
    try {
      return &entries.at(index);
    } catch (std::out_of_range &err) {
      throw EntryNotPresent();
    }
  }

  // True if time is within the timelog's day.
  bool time_valid(Clock::time_point time) {
    Clock::time_point current_day_time = std::chrono::sys_days(calendar_day);
    Clock::time_point next_day_time = current_day_time + std::chrono::days(1);
    return time >= current_day_time && time < next_day_time;
  }

  // True if index is a valid index within entries.
  bool index_valid(size_t index) { return index < entries.size(); }
};

// A journal of timelogs.
class TimeLogJournal {
  std::unordered_map<CalendarDay, Timelog> pages;

public:
  TimeLogJournal() {}
};

class SummaryState {};
class ViewState {};
class ModifyState {};

class TimelogFSM {};

int main(int argc, const char **argv) {
  std::cout << "Arg Count: " << argc << std::endl;
  for (auto i = 0; i < argc; ++i) {
    std::cout << *argv << std::endl;
    ++argv;
  }
}