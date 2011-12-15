#pragma once

#include "TestHarness.h"
#include "Test.h"
#include "Failure.h"


#define TEST_F(testName, fixture)									\
	struct fixture##testName : public fixture						\
	{																\
		fixture##testName(const char* name_) : mTestName(name_)	{}	\
		~fixture##testName();										\
																	\
		void testName(TestResult& result_);							\
		SimpleString mTestName;										\
	};																\
																	\
	fixture##testName::~fixture##testName()							\
	{																\
	}																\
																	\
																	\
	class fixture##testName##Test : public Test						\
	{																\
	public:															\
		fixture##testName##Test() : Test (#testName "Test") {}		\
		~fixture##testName##Test(); 								\
	protected:														\
		virtual void run (TestResult& result_);						\
	} fixture##testName##Instance;									\
																	\
	fixture##testName##Test::~fixture##testName##Test()				\
	{																\
	}																\
																	\
																	\
	void fixture##testName##Test::run (TestResult& result_)			\
	{																\
		fixture##testName mt(#testName);							\
		mt.testName(result_);										\
	}																\
																	\
	void fixture##testName::testName(TestResult& result_)


