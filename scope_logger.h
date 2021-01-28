#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <thread>

/**********************************************************************************************************************/

class ScopeLogger;
inline static thread_local std::stack<ScopeLogger*> internal_scope_logger_stack;

/**********************************************************************************************************************/

class ScopeLogger {
 public:
  ScopeLogger(const ScopeLogger&) = delete;
  ScopeLogger(ScopeLogger&&) = delete;
  ScopeLogger& operator=(const ScopeLogger&) = delete;
  ScopeLogger& operator=(ScopeLogger&&) = delete;

  ScopeLogger(std::ostream& stream, std::string&& scope_name);
  ScopeLogger(std::ostream& stream, const void* const object, std::string&& scope_name);
  virtual ~ScopeLogger();

  void LogTimestamp() const;
  void LogMessage(std::string&& message) const;
  template <typename VariableType>
  void LogVariable(std::string&& name, VariableType&& value) const;

 private:
  using UnixTimestamp = std::chrono::milliseconds;
  static UnixTimestamp TimeSinceEpoch();

  void WriteMessage(std::string&& message, const UnixTimestamp& current_timestamp) const;
  void WriteHeader(std::stringstream& string_stream, const UnixTimestamp& current_timestamp) const;
  void WriteFooter(std::stringstream& string_stream, const UnixTimestamp& current_timestamp) const;
  void WriteText(std::string&& message) const;

 private:
  std::ostream& stream_;
  const void* const object_;
  const UnixTimestamp begin_timestamp_;
  const std::string scope_name_;

  static constexpr auto kBeginMessage = "Begin";
  static constexpr auto kEndMessage = "End";

  static constexpr auto kThreadTag = "Thread: ";
  static constexpr auto kTimestampTag = "Timestamp: ";
  static constexpr auto kObjectTag = "Object: ";
  static constexpr auto kScopeTag = "Scope: ";
  static constexpr auto kMessageTag = "Message: ";
  static constexpr auto kVariableNameTag = "Variable name: ";
  static constexpr auto kValueTag = "Value: ";
  static constexpr auto kDurationTag = "Duration: ";
  static constexpr auto kUncaughtExceptionsTag = "Uncaught exceptions: ";
  static constexpr auto kTagSeparator = ". ";
};

inline ScopeLogger::ScopeLogger(std::ostream& stream, std::string&& scope_name)
    : ScopeLogger{stream, nullptr, std::forward<std::string>(scope_name)} {}

inline ScopeLogger::ScopeLogger(std::ostream& stream, const void* const object, std::string&& scope_name)
    : stream_{stream},
      object_{object},
      begin_timestamp_{TimeSinceEpoch()},
      scope_name_{std::forward<std::string>(scope_name)} {
  WriteMessage(kBeginMessage, begin_timestamp_);

  internal_scope_logger_stack.emplace(this);
}

inline ScopeLogger::~ScopeLogger() {
  const auto current_timestamp{TimeSinceEpoch()};
  WriteMessage(kEndMessage, current_timestamp);

  internal_scope_logger_stack.pop();
}

inline void ScopeLogger::LogTimestamp() const {
  const auto current_timestamp{TimeSinceEpoch()};

  std::stringstream string_stream;
  WriteHeader(string_stream, current_timestamp);
  WriteFooter(string_stream, current_timestamp);

  auto text = string_stream.str();
  WriteText(std::move(text));
}

inline void ScopeLogger::LogMessage(std::string&& message) const {
  const auto current_timestamp{TimeSinceEpoch()};
  WriteMessage(std::forward<std::string>(message), current_timestamp);
}

template <typename VariableType>
void ScopeLogger::LogVariable(std::string&& name, VariableType&& value) const {
  const auto current_timestamp{TimeSinceEpoch()};

  std::stringstream string_stream;
  WriteHeader(string_stream, current_timestamp);
  string_stream << kTagSeparator << kVariableNameTag << name;
  string_stream << kTagSeparator << kValueTag << value;
  WriteFooter(string_stream, current_timestamp);

  auto text = string_stream.str();
  WriteText(std::move(text));
}

inline ScopeLogger::UnixTimestamp ScopeLogger::TimeSinceEpoch() {
  const auto time_point{std::chrono::system_clock::now()};
  const auto time_since_epoch{time_point.time_since_epoch()};
  return std::chrono::duration_cast<UnixTimestamp>(time_since_epoch);
}

inline void ScopeLogger::WriteMessage(std::string&& message, const UnixTimestamp& current_timestamp) const {
  std::stringstream string_stream;
  WriteHeader(string_stream, current_timestamp);
  string_stream << kTagSeparator << kMessageTag << message;
  WriteFooter(string_stream, current_timestamp);

  auto text = string_stream.str();
  WriteText(std::move(text));
}

inline void ScopeLogger::WriteHeader(std::stringstream& string_stream, const UnixTimestamp& current_timestamp) const {
  string_stream << kThreadTag << std::this_thread::get_id();
  string_stream << kTagSeparator << kTimestampTag << current_timestamp.count();

  if (object_ != nullptr) {
    string_stream << kTagSeparator << kObjectTag << object_;
  }

  string_stream << kTagSeparator << kScopeTag << scope_name_;
}

inline void ScopeLogger::WriteFooter(std::stringstream& string_stream, const UnixTimestamp& current_timestamp) const {
  const auto uncaught_exceptions = std::uncaught_exceptions();
  if (uncaught_exceptions != 0) {
    string_stream << kTagSeparator << kUncaughtExceptionsTag << uncaught_exceptions;
  }

  const auto duration = current_timestamp - begin_timestamp_;
  const auto duration_count = duration.count();
  string_stream << kTagSeparator << kDurationTag << duration_count;

  string_stream << std::endl;
}

inline void ScopeLogger::WriteText(std::string&& message) const {
  stream_ << message;
  stream_.flush();
}

/**********************************************************************************************************************/

class ConsoleScopeLogger final : public ScopeLogger {
 public:
  explicit ConsoleScopeLogger(std::string&& scope_name);
  ConsoleScopeLogger(const void* const object, std::string&& scope_name);
};

inline ConsoleScopeLogger::ConsoleScopeLogger(std::string&& scope_name)
    : ConsoleScopeLogger{nullptr, std::forward<std::string>(scope_name)} {}

inline ConsoleScopeLogger::ConsoleScopeLogger(const void* const object, std::string&& scope_name)
    : ScopeLogger(std::cout, object, std::forward<std::string>(scope_name)) {}

/**********************************************************************************************************************/

class FileScopeLogger final : public ScopeLogger {
 public:
  explicit FileScopeLogger(std::string&& scope_name);
  FileScopeLogger(const void* const object, std::string&& scope_name);

 private:
  inline static std::ofstream file_stream_{"ScopeLogger.log", std::ios::app};
};

inline FileScopeLogger::FileScopeLogger(std::string&& scope_name)
    : FileScopeLogger{nullptr, std::forward<std::string>(scope_name)} {}

inline FileScopeLogger::FileScopeLogger(const void* const object, std::string&& scope_name)
    : ScopeLogger{file_stream_, object, std::forward<std::string>(scope_name)} {}

/**********************************************************************************************************************/

#define INTERNAL_JOIN(a, b) a##b
#define INTERNAL_EXPAND_AND_JOIN(a, b) INTERNAL_JOIN(a, b)
#define INTERNAL_UNIQUE_NAME(prefix) INTERNAL_EXPAND_AND_JOIN(prefix, __LINE__)

#define LOG_MEMBER \
  const ConsoleScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { this, __func__ }
#define LOG_FUNCTION \
  const ConsoleScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { __func__ }
#define LOG_SCOPE(scope_name) \
  const ConsoleScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { scope_name }

#define FILE_LOG_MEMBER \
  const FileScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { this, __func__ }
#define FILE_LOG_FUNCTION \
  const FileScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { __func__ }
#define FILE_LOG_SCOPE(scope_name) \
  const ConsoleScopeLogger INTERNAL_UNIQUE_NAME(scope_logger) { scope_name }

#define LOG_TIMESTAMP internal_scope_logger_stack.top()->LogTimestamp()
#define LOG_MESSAGE(message) internal_scope_logger_stack.top()->LogMessage(#message)
#define LOG_VARIABLE(variable) internal_scope_logger_stack.top()->LogVariable(#variable, variable)
