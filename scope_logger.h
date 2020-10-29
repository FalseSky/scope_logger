#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

class ScopeLogger final {
 public:
  ScopeLogger(const ScopeLogger&) = delete;
  ScopeLogger(ScopeLogger&&) = delete;
  ScopeLogger& operator=(const ScopeLogger&) = delete;
  ScopeLogger& operator=(ScopeLogger&&) = delete;

  explicit ScopeLogger(std::string&& scope_id);
  ScopeLogger(const void* object_id, std::string&& scope_id);
  ~ScopeLogger();

  void LogTimestamp() const;
  void LogMessage(std::string&& message) const;
  template <typename VariableType>
  void LogVariable(std::string&& variable_name, VariableType&& variable) const;

 private:
  using Duration = std::chrono::milliseconds;
  static Duration Now();

  void WriteMessage(const Duration& current_timestamp,
                    std::string&& string) const;
  void WriteHeader(std::stringstream& string_stream,
                   const Duration& current_timestamp) const;
  void WriteFooter(std::stringstream& string_stream,
                   const Duration& current_timestamp) const;
  static void WriteText(std::string&& message);

 private:
  const Duration start_timestamp_;
  const void* const object_id_;
  const std::string scope_id_;

  static constexpr auto kBeginMessage = "Begin";
  static constexpr auto kEndMessage = "End";
  static constexpr auto kTimestampMessage = "Timestamp";
  static constexpr auto kVariableMessage = "Variable";

  static constexpr auto kThreadTag = "Thread: ";
  static constexpr auto kTimestampTag = "Timestamp: ";
  static constexpr auto kObjectTag = "Object: ";
  static constexpr auto kScopeTag = "Scope: ";
  static constexpr auto kMessageTag = "Message: ";
  static constexpr auto kNameTag = "Name: ";
  static constexpr auto kValueTag = "Value: ";
  static constexpr auto kDurationTag = "Duration: ";
  static constexpr auto kTagSeparator = ". ";
};

inline ScopeLogger::ScopeLogger(std::string&& scope_id)
    : start_timestamp_{Now()},
      object_id_{nullptr},
      scope_id_{std::move(scope_id)} {
  WriteMessage(start_timestamp_, kBeginMessage);
}

inline ScopeLogger::ScopeLogger(const void* const object_id,
                                std::string&& scope_id)
    : start_timestamp_{Now()},
      object_id_{object_id},
      scope_id_{std::move(scope_id)} {
  WriteMessage(start_timestamp_, kBeginMessage);
}

inline ScopeLogger::~ScopeLogger() {
  const auto current_timestamp{Now()};
  WriteMessage(current_timestamp, kEndMessage);
}

inline void ScopeLogger::LogTimestamp() const {
  const auto current_timestamp{Now()};
  WriteMessage(current_timestamp, kTimestampMessage);
}

inline void ScopeLogger::LogMessage(std::string&& message) const {
  const auto current_timestamp{Now()};
  WriteMessage(current_timestamp, std::move(message));
}

template <typename VariableType>
inline void ScopeLogger::LogVariable(std::string&& variable_name,
                                     VariableType&& variable) const {
  const auto current_timestamp{Now()};

  std::stringstream string_stream;
  WriteHeader(string_stream, current_timestamp);
  string_stream << kTagSeparator << kMessageTag << kVariableMessage;
  string_stream << kTagSeparator << kNameTag << variable_name;
  string_stream << kTagSeparator << kValueTag << variable;
  WriteFooter(string_stream, current_timestamp);
  WriteText(string_stream.str());
}

inline ScopeLogger::Duration ScopeLogger::Now() {
  return std::chrono::duration_cast<Duration>(
      std::chrono::steady_clock::now().time_since_epoch());
}

inline void ScopeLogger::WriteMessage(const Duration& current_timestamp,
                                      std::string&& string) const {
  std::stringstream string_stream;
  WriteHeader(string_stream, current_timestamp);
  string_stream << kTagSeparator << kMessageTag << string;
  WriteFooter(string_stream, current_timestamp);
  WriteText(string_stream.str());
}

inline void ScopeLogger::WriteHeader(std::stringstream& string_stream,
                                     const Duration& current_timestamp) const {
  string_stream << kThreadTag << std::this_thread::get_id();
  string_stream << kTagSeparator << kTimestampTag << current_timestamp.count();

  if (object_id_) {
    string_stream << kTagSeparator << kObjectTag << object_id_;
  }

  string_stream << kTagSeparator << kScopeTag << scope_id_;
}

inline void ScopeLogger::WriteFooter(std::stringstream& string_stream,
                                     const Duration& current_timestamp) const {
  string_stream << kTagSeparator << kDurationTag
                << (current_timestamp - start_timestamp_).count();
  string_stream << std::endl;
}

inline void ScopeLogger::WriteText(std::string&& message) {
  std::cout << message;
}
