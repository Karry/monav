#include "SimpleString.h"


#include <cstring>
#include <cstdio>

static const unsigned int DEFAULT_SIZE = 20;


SimpleString::SimpleString()
	: mBuffer(0)
{
	mBuffer = CreateEmptyCString(DEFAULT_SIZE);
}


SimpleString::SimpleString(const char* otherBuffer)
:	mBuffer(0)
{
	unsigned int newBufferSize = strlen (otherBuffer) + 1;

	mBuffer = CreateEmptyCString(newBufferSize);
	strcat(mBuffer, otherBuffer);
}



SimpleString::SimpleString(const SimpleString& other)
:	mBuffer(0)
{
	unsigned int newBufferSize = other.GetLength() + 1;

	mBuffer = CreateEmptyCString(newBufferSize);
	strcat(mBuffer, other.mBuffer);
}


SimpleString::~SimpleString ()
{
	delete [] mBuffer;
}


SimpleString&
SimpleString::operator= (const SimpleString& other)
{
	unsigned int newBufferSize = other.GetLength() + 1;

	delete [] mBuffer;
	mBuffer = CreateEmptyCString(newBufferSize);
	strcat(mBuffer, other.mBuffer);

	return *this;
}


void
SimpleString::operator +=(const char* rhs)
{
	unsigned int newBufferSize = GetLength() + strlen(rhs) + 1;

	char* tmpBuf = CreateEmptyCString(newBufferSize);
	strcat(tmpBuf, mBuffer);
	strcat(tmpBuf, rhs);

	delete [] mBuffer;
	mBuffer = tmpBuf;
}


void
SimpleString::operator +=(const SimpleString& rhs)
{
	unsigned int newBufferSize = GetLength() + rhs.GetLength() + 1;

	char* tmpBuf = CreateEmptyCString(newBufferSize);
	strcat(tmpBuf, mBuffer);
	strcat(tmpBuf, rhs.GetCString());

	delete [] mBuffer;
	mBuffer = tmpBuf;
}


char*
SimpleString::GetCString () const
{
	return mBuffer;
}


unsigned int
SimpleString::GetLength() const
{
	return strlen(mBuffer);
}


inline char*
SimpleString::CreateEmptyCString(unsigned int size)
{
	char* newCString = new char[size];
	newCString[0] = 0;

	return newCString;
}





bool operator== (const SimpleString& left, const SimpleString& right)
{
	return strcmp(left.GetCString(), right.GetCString()) == 0;
}


SimpleString StringFrom (bool value)
{
	char trueString[5] = "true";
	char falseString[6] = "false";

	const unsigned int tmpBufferSize = 6;
	char buffer[tmpBufferSize];
	buffer[0] = 0;
	strcat(buffer, value ? trueString : falseString);

	return SimpleString(buffer);
}


SimpleString StringFrom (const char *value)
{
	return SimpleString(value);
}


SimpleString StringFrom (short value)
{
	return StringFrom(static_cast<long>(value));
}


SimpleString StringFrom (unsigned short value)
{
	return StringFrom(static_cast<unsigned long>(value));
}


SimpleString StringFrom (int value)
{
	return StringFrom(static_cast<long>(value));
}


SimpleString StringFrom (unsigned int value)
{
	return StringFrom(static_cast<unsigned long>(value));
}


SimpleString StringFrom (long value)
{
	char buffer[DEFAULT_SIZE];
	sprintf(buffer, "%ld", value);

	return SimpleString(buffer);
}


SimpleString StringFrom (unsigned long value)
{
	char buffer[DEFAULT_SIZE];
	sprintf(buffer, "%ld", value);

	return SimpleString(buffer);
}


SimpleString StringFrom (double value)
{
	char buffer[DEFAULT_SIZE];
	sprintf(buffer, "%f", value);

	return SimpleString(buffer);
}


SimpleString StringFrom (const SimpleString& value)
{
	return SimpleString(value);
}
