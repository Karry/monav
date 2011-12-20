#pragma once

#include "SimpleString.h"

class TestResult;


class Test
{
public:
	Test(const SimpleString& testName);
	virtual ~Test();

	virtual void run(TestResult& result) = 0;

	void	setNext(Test* test);
	Test*	getNext () const;

protected:
	bool DoublesNearlyEqual(double lhs, double rhs, double epsilon);

	SimpleString	mTestName;
	Test*			mNextTest;

private:
	Test(const Test&);
	Test& operator= (const Test&);
};



#define TEST(testName, testGroup)												\
	class testGroup##_##testName##Test : public Test							\
	{																			\
		public:																	\
			testGroup##_##testName##Test() : Test(#testName "Test")		{};		\
			virtual ~testGroup##_##testName##Test()						{};		\
																				\
            virtual void run (TestResult& result_);								\
	}																			\
																				\
	testGroup##_##testName##Instance;											\
	void testGroup##_##testName##Test::run(TestResult& result_)



#define CHECK(condition)															\
{																					\
	if (!(condition))																\
		result_.addFailure(Failure(mTestName, __FILE__, __LINE__, #condition));	\
}




#define CHECK_EQUAL(actual, expected)																						\
{																															\
	SimpleString condition(#actual);																						\
	condition += " == ";																									\
	condition += #expected;																									\
	condition += "\n";																										\
	if (!((actual) == (expected)))																						\
		result_.addFailure(Failure(mTestName, __FILE__, __LINE__, condition, StringFrom(expected), StringFrom(actual)));										\
}


#define LONGS_EQUAL(actual, expected)																					\
{																														\
	long actualTemp = actual;																							\
	long expectedTemp = expected;																						\
	if ((actualTemp) != (expectedTemp))																					\
		result_.addFailure (Failure(mTestName, __FILE__, __LINE__, StringFrom(expectedTemp), StringFrom(actualTemp)));	\
}


#define DOUBLES_EQUAL(actual, expected, epsilon)																		\
{																														\
	double actualTemp = actual;																							\
	double expectedTemp = expected;																						\
	if (! DoublesNearlyEqual(actualTemp, expectedTemp, epsilon))														\
		result_.addFailure(Failure(mTestName, __FILE__, __LINE__, StringFrom(expectedTemp), StringFrom(actualTemp)));	\
}


#define FAIL(text)														\
{																		\
	result_.addFailure(Failure(mTestName, __FILE__, __LINE__, (text)));	\
}
