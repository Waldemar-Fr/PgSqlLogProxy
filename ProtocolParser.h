#pragma once

#include <string>
#include <vector>

namespace PgSqlParse
{

	class InputBuffer
	{
		using Bytes = std::vector<uint8_t>;

	public:
		void Append(const void* data, size_t size);
		uint8_t GetByte();
		char GetChar();
		Bytes GetBytes(size_t size);
		void GetBytes(void* data, size_t size);
		std::string GetString(size_t size);
		void DiscardBytes(size_t size);
		bool IsNotEmpty() const;
		void Clear()
		{
			buf.clear();
		}

	private:
		Bytes buf;
	};


	struct Message
	{
		char code;
		std::string strInfo;
	};

	Message GetMsg(InputBuffer& in, int maxLen = -1);
	std::string GetStringInfo(InputBuffer& in, int maxLen = -1);


}

int64_t ConvertStrToUlong(const std::string & str, int64_t defVal);
