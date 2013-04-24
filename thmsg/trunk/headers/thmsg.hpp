#pragma once
#define MSGTYPE_DIALOGUE 0x10
#include <vector>
typedef unsigned char byte;
typedef byte uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef struct
{
	uint32_t pos;		// offset
	uint16_t indexref;	// which index references this message (if at all)
	uint16_t header;
	uint8_t type;
	uint8_t len;		// length
	std::vector<byte> data;
} msg;