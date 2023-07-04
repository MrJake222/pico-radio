#pragma once

#define MSG_TYPE_SHIFT(msg_bits, type_bits) ((msg_bits) - (type_bits))
#define  MSG_DATA_MASK(msg_bits, type_bits) ((1 << MSG_TYPE_SHIFT(msg_bits, type_bits)) - 1)
#define      MSG_TYPE(msg_bits, type_bits, val) ((val) >> ((msg_bits) - (type_bits)))
#define      MSG_DATA(msg_bits, type_bits, val) ((val) & MSG_DATA_MASK(msg_bits, type_bits))
#define      MSG_MAKE(msg_bits, type_bits, type, data) (((type) << MSG_TYPE_SHIFT(msg_bits, type_bits)) | ((data) & MSG_DATA_MASK(msg_bits, type_bits)))