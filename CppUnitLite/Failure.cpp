#include "Failure.h"


#include <cstdio>
#include <cstring>


Failure::Failure (const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& theCondition)
	:	mMessage(theCondition), mTestName(theTestName), mFileName(theFileName), mLineNumber(theLineNumber)
{
}


Failure::Failure (const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& expected, const SimpleString& actual)
	:	mMessage(""), mTestName(theTestName), mFileName(theFileName), mLineNumber(theLineNumber)
{
	const char* part1 = "expected ";
	const char* part3 = " but was: ";

	size_t messageSize = strlen(part1) + expected.GetLength() + strlen(part3) + actual.GetLength() + 1;

	char* buf = new char[messageSize];
	sprintf(buf, "%s%s%s%s", part1, expected.GetCString(), part3, actual.GetCString());
	mMessage = SimpleString(buf);
	delete [] buf;
}


Failure::Failure(const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& condition, const SimpleString& expected, const SimpleString& actual)
	:	mMessage(""), mTestName(theTestName), mFileName(theFileName), mLineNumber(theLineNumber)
{
	const char* part1 = "  expected: ";
	const char* part3 = "\n  but was: ";

	size_t messageSize = condition.GetLength() + strlen(part1) + expected.GetLength() + strlen(part3) + actual.GetLength() + 1;

	char* buf = new char[messageSize];
	sprintf(buf, "%s%s%s%s%s", condition.GetCString(), part1, StringFrom(expected).GetCString(), part3, StringFrom(actual).GetCString());
	mMessage = SimpleString(buf);
	delete [] buf;
}


Failure::~Failure()
{

}

