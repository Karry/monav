#pragma once


#include "SimpleString.h"


class Failure;


class TestResult
{
public:
			TestResult();
	virtual	~TestResult();

	virtual void	testsStarted();
	virtual void	addFailure(const Failure& failure);
	virtual void	testsEnded();

	SimpleString	resultStr;

protected:
	int	failureCount;
};
