from enum import Enum

scope_logger_output_path = 'scope_logger_output.txt'

begin_message = "Begin"
end_message = "End"
timestamp_message = "Timestamp"

thread_tag = 'Thread: '
timestamp_tag = 'Timestamp: '
object_tag = 'Object: '
scope_tag = 'Scope: '
message_tag = 'Message: '
duration_tag = 'Duration: '
tag_separator = '. '

message_format = 'Thread: {0}. Timestamp: {1}. Object: {2}. Scope: {3}. Message: {4}. Duration: {5}'


class MessageType(Enum):
    BEGIN = 0
    END = 1
    TIMESTAMP = 2
    CUSTOM = 3


class Message:
    def __init__(self):
        self.thread_id = 0
        self.timestamp = 0
        self.object_id = 0
        self.scope_id = ''
        self.message_type = MessageType.CUSTOM
        self.message = ''
        self.duration = 0

    def __str__(self):
        return message_format.format(hex(self.thread_id), self.timestamp, hex(self.object_id), self.scope_id,
                                     self.message, self.duration)


file = open(scope_logger_output_path, 'r')
raw_messages = file.readlines()

messages = []

for raw_message in raw_messages:
    values = raw_message.split(tag_separator)

    new_message = Message()

    for value in values:
        if value.startswith(thread_tag):
            new_message.thread_id = int(value[len(thread_tag):], 16)
        elif value.startswith(timestamp_tag):
            new_message.timestamp = int(value[len(timestamp_tag):])
        elif value.startswith(object_tag):
            new_message.object_id = int(value[len(object_tag):], 16)
        elif value.startswith(scope_tag):
            new_message.scope_id = value[len(scope_tag):]
        elif value.startswith(message_tag):
            new_message.message = value[len(message_tag):]
        elif value.startswith(duration_tag):
            new_message.duration = int(value[len(duration_tag):])

    if new_message.message == begin_message:
        new_message.message_type = MessageType.BEGIN
    elif new_message.message == end_message:
        new_message.message_type = MessageType.END
    elif new_message.message == timestamp_message:
        new_message.message_type = MessageType.TIMESTAMP
    else:
        new_message.message_type = MessageType.CUSTOM

    messages.append(new_message)

for message in messages:
    print(message)
