
#include "ProtocolParser.h"
#include <exception>
#include <Winsock2.h>
#include "AsyncFileLogger.h"

#define THROW(s)	throw std::runtime_error(s);


namespace PgSqlParse
{

Message GetMsg(InputBuffer& in, int maxLen /* = - 1 */)
{
	Message msg;
	msg.code = in.GetChar();
	msg.strInfo = GetStringInfo(in, maxLen);
	return msg;
}

std::string GetStringInfo(InputBuffer& in, int maxLen /* = -1 */)
{
	uint32_t	len;
	/* Read message length word */
	in.GetBytes(&len, 4);

	len = ntohl(len);

	if (len < 4 || (maxLen > 0 && (int)(len) > maxLen))
	{
		THROW("invalid message length");
	}

	len -= 4;					/* discount length itself */
	return in.GetString(len);
}


void InputBuffer::Append(const void * data, size_t size)
{
	const uint8_t* beg = static_cast<const uint8_t*>(data);
	buf.insert(buf.end(), beg, beg + size);
}

uint8_t InputBuffer::GetByte()
{
	uint8_t b;
	GetBytes(&b, sizeof(b));
	return b;
}

char PgSqlParse::InputBuffer::GetChar()
{
	return (char)GetByte();
}

InputBuffer::Bytes InputBuffer::GetBytes(size_t size)
{
	Bytes bs;
	bs.resize(size);
	GetBytes(bs.data(), bs.size());
	return bs;
}

void InputBuffer::GetBytes(void * data, size_t size)
{
	if (size > buf.size())
	{
		THROW("unexpected EOF when get bytes");
	}
	memcpy(data, buf.data(), size);
	buf.erase(buf.begin(), buf.begin() + size);
}

std::string InputBuffer::GetString(size_t size)
{
	Bytes bs = GetBytes(size);
	return std::string(bs.begin(), bs.end());
}

void InputBuffer::DiscardBytes(size_t size)
{
	if (size > buf.size())
	{
		THROW("unexpected EOF when discard bytes");
	}
	buf.erase(buf.begin(), buf.begin() + size);
}

bool InputBuffer::IsNotEmpty() const
{
	return !buf.empty();
}



}

int64_t ConvertStrToUlong(const std::string& str, int64_t defVal)
{
	char * pEnd;
	int64_t val = strtol(str.c_str(), &pEnd, 10);
	return str.c_str() != pEnd ? val : defVal;
}
