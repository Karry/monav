#pragma once

#include "SimpleString.h"


class Failure
{
public:
	Failure (const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& theCondition);
	Failure (const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& expected, const SimpleString& actual);
	Failure (const SimpleString& theTestName, const SimpleString& theFileName, long theLineNumber, const SimpleString& theCondition, const SimpleString& expected, const SimpleString& actual);
	~Failure();

	SimpleString mMessage;
	SimpleString mTestName;
	SimpleString mFileName;
	long mLineNumber;
};
