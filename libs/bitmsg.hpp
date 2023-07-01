#pragma once

#define MSG_BITS      32
#define MSG_TYPE_BITS  8
#define MSG_DATA_MASK ((1 << (MSG_BITS - MSG_TYPE_BITS)) - 1)
#define MSG_TYPE(val) ((FifoMsgType) ((val) >> (MSG_BITS - MSG_TYPE_BITS)))
#define MSG_DATA(val) ((val) & MSG_DATA_MASK)
#define MSG_MAKE(type, data) (((type) << (MSG_BITS - MSG_TYPE_BITS)) | ((data) & MSG_DATA_MASK))