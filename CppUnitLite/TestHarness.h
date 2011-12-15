#pragma once


#include "TestResult.h"



class TestHarness
{
public:
	TestHarness() : mTR()	{}
	const char* RunAllTests();
	void Destroy();

	unsigned long GetNumTestsRun();

private:
	TestResult mTR;
};


extern TestHarness testHarness;
