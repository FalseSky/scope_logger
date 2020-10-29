from enum import Enum

scope_logger_output_path = 'scope_logger_output.txt'

begin_message = "Begin"
end_message = "End"
timestamp_message = "Timestamp"
variable_message = "Variable"

thread_tag = 'Thread: '
timestamp_tag = 'Timestamp: '
object_tag = 'Object: '
scope_tag = 'Scope: '
message_tag = 'Message: '
name_tag = "Name: "
value_tag = "Value: "
duration_tag = 'Duration: '
tag_separator = '. '

message_format = 'Thread: {}. Timestamp: {}. Scope: {}. Message: {}. Duration: {}'
message_with_object_format = 'Thread: {}. Timestamp: {}. Object: {}. Scope: {}. Message: {}. Duration: {}'
variable_message_format = 'Thread: {}. Timestamp: {}. Scope: {}. Message: {}. Name: {}. Value: {}. Duration: {}'
variable_message_with_object_format = 'Thread: {}. Timestamp: {}. Object: {}. Scope: {}. Message: {}. Name: {}. ' \
                                      'Value: {}. Duration: {} '


class MessageType(Enum):
    BEGIN = 0
    END = 1
    TIMESTAMP = 2
    VARIABLE = 3
    CUSTOM = 4


class Message:
    def __init__(self, raw_message):
        self.thread_id = 0
        self.timestamp = 0
        self.object_id = 0
        self.scope_id = ''
        self.message_type = MessageType.CUSTOM
        self.message = ''
        self.name = ''
        self.value = ''
        self.duration = 0

        raw_message_values = raw_message.split(tag_separator)

        for raw_message_value in raw_message_values:
            if raw_message_value.startswith(thread_tag):
                self.thread_id = int(raw_message_value[len(thread_tag):], 16)
            elif raw_message_value.startswith(timestamp_tag):
                self.timestamp = int(raw_message_value[len(timestamp_tag):])
            elif raw_message_value.startswith(object_tag):
                self.object_id = int(raw_message_value[len(object_tag):], 16)
            elif raw_message_value.startswith(scope_tag):
                self.scope_id = raw_message_value[len(scope_tag):]
            elif raw_message_value.startswith(message_tag):
                self.message = raw_message_value[len(message_tag):]
            elif raw_message_value.startswith(name_tag):
                self.name = raw_message_value[len(name_tag):]
            elif raw_message_value.startswith(value_tag):
                self.value = raw_message_value[len(value_tag):]
            elif raw_message_value.startswith(duration_tag):
                self.duration = int(raw_message_value[len(duration_tag):])

        if self.message == begin_message:
            self.message_type = MessageType.BEGIN
        elif self.message == end_message:
            self.message_type = MessageType.END
        elif self.message == timestamp_message:
            self.message_type = MessageType.TIMESTAMP
        elif self.message == variable_message:
            self.message_type = MessageType.VARIABLE
        else:
            self.message_type = MessageType.CUSTOM

    def __str__(self):
        if self.object_id == 0:
            if self.message_type == MessageType.VARIABLE:
                return variable_message_format.format(hex(self.thread_id), self.timestamp, self.scope_id, self.message,
                                                      self.name, self.value, self.duration)
            else:
                return message_format.format(hex(self.thread_id), self.timestamp, self.scope_id, self.message,
                                             self.duration)
        else:
            if self.message_type == MessageType.VARIABLE:
                return variable_message_with_object_format.format(hex(self.thread_id), self.timestamp,
                                                                  hex(self.object_id), self.scope_id, self.message,
                                                                  self.name, self.value, self.duration)
            else:
                return message_with_object_format.format(hex(self.thread_id), self.timestamp, hex(self.object_id),
                                                         self.scope_id, self.message, self.duration)


messages = []

file = open(scope_logger_output_path, 'r')
file_messages = file.readlines()

for file_message in file_messages:
    messages.append(Message(file_message))

for message in messages:
    print(message)
