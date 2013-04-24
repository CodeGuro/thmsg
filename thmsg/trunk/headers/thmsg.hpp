#pragma once
#define MSGTYPE_DIALOGUE 0x10
#include <vector>
#include <string>
#include <stdlib.h>
#include <iostream>
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

// Endianness swapping
inline void endian_swap(unsigned short& x)
{
    x = (x>>8) |
        (x<<8);
}

inline void endian_swap(unsigned int& x)
{
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

//decode
void xorz(std::vector<byte> * buf, int xorval, int xoradd, int addadd)
{
	for(uint32_t i=0; i<buf->size(); i++) {
		buf->at(i) = buf->at(i) ^ xorval;
		xorval = (xorval + xoradd) % 256;
		xoradd = (xoradd + addadd) % 256;
	}
}

//Ayayaya! report on the hex values!
void aya(std::vector<byte> buf)
{
	uint32_t len = buf.size();

	std::cout << std::hex;
	
	xorz(&buf, 0x77, 7, 16);
	for(uint32_t i=0; i<len; i++) {
		std::cout << (int)buf.at(i) << " ";
	}
	std::cout << std::endl << std::endl;
}

std::vector<byte> hex2bytes(std::string const & str)
{
	std::vector<byte> buf;

	// First pass: erase whitespace
	for(uint32_t i=0; i<str.length(); i++)
		if( str.at(i) == ' ' || str.at(i) == '-' )
			str.erase(i, 1);

	// Second pass: process 2 bytes at a time, converting them to numeral format
	for(uint32_t i=0; i<str.length(); i+=2)
		buf.push_back( (byte)( strtoul(str.substr(i, 2).c_str(), 0, 16) ) );

	return buf;
}

inline uint32_t hex2long(std::string const & str)
{
	return (uint32_t)( strtoul(str.c_str(), 0, 16)  );
}

inline uint16_t hex2short(std::string const & str)
{
	return (uint16_t)( strtoul(str.c_str(), 0, 16)  );
}

inline uint8_t hex2byte(std::string const & str)
{
	return (uint8_t)( strtoul(str.c_str(), 0, 16)  );
}

std::vector<byte> str2bytes(std::string const & str)
{
	uint32_t len = str.length();
	std::vector<byte> buf;

	for(uint32_t i=0; i<len; i++) 
		buf.push_back( (uint8_t)str[i] );
	

	return buf;
}

std::string byte2str(std::vector<byte> const & buf)
{
	std::string tmp;

	for(uint32_t i=0; i<buf.size(); i++)
		tmp.push_back( (char)(buf.at(i)) );

	return tmp;
}

std::string byte2hex(std::vector<byte> const & buf)
{
	std::string tmp;

	for(uint32_t i=0; i<buf.size(); i++) 
	{
		char chrtmp[2];
		sprintf( chrtmp, "%X", (uint16_t)(buf.at(i)) );
		if(strlen(chrtmp) == 1) { chrtmp[1] = chrtmp[0]; chrtmp[0] = '0'; }
		else if(strlen(chrtmp) == 0) { chrtmp[0] = '0'; chrtmp[1] = '0'; }
		chrtmp[2] = 0;

		tmp += chrtmp;
	}

	return tmp;
}

std::string int2hex(uint8_t i)
{
	char chrtmp[32];
	sprintf( chrtmp, "%X", i );

	std::string tmp( 2-strlen(chrtmp), '0');
	tmp += chrtmp;

	return tmp;
}

std::string int2hex(uint16_t i) 
{
	char chrtmp[32];
	sprintf( chrtmp, "%X", i );

	std::string tmp( 4-strlen(chrtmp), '0');
	tmp += chrtmp;

	return tmp;
}

std::string int2hex(uint32_t i) 
{
	char chrtmp[32];
	sprintf( chrtmp, "%X", i );

	std::string tmp( 8-strlen(chrtmp), '0');
	tmp += chrtmp;

	return tmp;
}

void cleanbytes(std::vector<byte> * data) 
{
	std::vector<byte>::iterator i = data->begin();
	while( i != data->end() )
	{
		if (*i == 0){ i = data->erase( i ); }
		else{ ++i; }
	}
}