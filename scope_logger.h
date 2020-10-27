#pragma once

#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <iostream>

class scope_logger final {
public:
  scope_logger(const scope_logger&) = delete;
  scope_logger(scope_logger&&) = delete;
  scope_logger& operator=(const scope_logger&) = delete;
  scope_logger& operator=(scope_logger&&) = delete;

  explicit scope_logger(std::string&& scope_id);
  scope_logger(const void* const object_id, std::string&& scope_id);
  ~scope_logger();

  void log_timestamp();
  void log_message(std::string&& message);
private:
  using duration_type = std::chrono::milliseconds;
  static duration_type now();

  void write_message(const duration_type& current_timestamp, std::string&& string);
  void write_header(std::stringstream& string_stream, const duration_type& current_timestamp);
  void write_footer(std::stringstream& string_stream, const duration_type& current_timestamp);
  void write_text(std::string&& message);
private:
  static inline std::mutex m_mutex;

  const duration_type m_start_timestamp;
  const void* const m_object_id;
  const std::string m_scope_id;

  static inline const auto m_begin_message = "Begin";
  static inline const auto m_end_message = "End";
  static inline const auto m_timestamp_message = "Timestamp";

  static inline const auto m_thread_tag = "Thread: ";
  static inline const auto m_timestamp_tag = "Timestamp: ";
  static inline const auto m_object_tag = "Object: ";
  static inline const auto m_scope_tag = "Scope: ";
  static inline const auto m_message_tag = "Message: ";
  static inline const auto m_duration_tag = "Duration: ";
  static inline const auto m_tag_separator = ". ";
};

inline scope_logger::scope_logger(std::string&& scope_id)
  : m_start_timestamp{now()},
    m_object_id{nullptr},
    m_scope_id{std::move(scope_id)} {
  std::unique_lock unique_lock{m_mutex};
  write_message(m_start_timestamp, m_begin_message);
}

inline scope_logger::scope_logger(const void* const object_id, std::string&& scope_id)
    : m_start_timestamp{now()},
      m_object_id{object_id},
      m_scope_id{std::move(scope_id)} {
  std::unique_lock unique_lock{m_mutex};
  write_message(m_start_timestamp, m_begin_message);
}

inline scope_logger::~scope_logger() {
  const auto current_timestamp{now()};
  std::unique_lock unique_lock{m_mutex};
  write_message(current_timestamp, m_end_message);
}

inline void scope_logger::log_timestamp() {
  const auto current_timestamp{now()};
  std::unique_lock unique_lock{m_mutex};
  write_message(current_timestamp, m_timestamp_message);
}

void scope_logger::log_message(std::string&& message) {
  const auto current_timestamp{now()};
  std::unique_lock unique_lock{m_mutex};
  write_message(current_timestamp, std::move(message));
}

inline scope_logger::duration_type scope_logger::now() {
  return std::chrono::duration_cast<duration_type>(std::chrono::steady_clock::now().time_since_epoch());
}

inline void scope_logger::write_message(const duration_type& current_timestamp, std::string&& string) {
  std::stringstream string_stream;
  write_header(string_stream, current_timestamp);
  string_stream << m_tag_separator << m_message_tag << string;
  write_footer(string_stream, current_timestamp);
  write_text(string_stream.str());
}

inline void scope_logger::write_header(std::stringstream& string_stream, const duration_type& current_timestamp) {
  string_stream << m_thread_tag << std::this_thread::get_id();
  string_stream << m_tag_separator << m_timestamp_tag << current_timestamp.count();

  if (m_object_id) {
    string_stream << m_tag_separator << m_object_tag << m_object_id;
  }

  string_stream << m_tag_separator << m_scope_tag << m_scope_id;
}

inline void scope_logger::write_footer(std::stringstream& string_stream, const duration_type& current_timestamp) {
  string_stream << m_tag_separator << m_duration_tag << (current_timestamp - m_start_timestamp).count();
  string_stream << std::endl;
}

inline void scope_logger::write_text(std::string&& message) {
  std::cout << message;
}
