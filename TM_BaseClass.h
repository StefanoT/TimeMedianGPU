// Time Median GPU
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#pragma once

#include "boost/utility.hpp"

class TM_BaseClass : boost::noncopyable
{
public:
	TM_BaseClass(int _NumSamples, int _BufferSize);
	~TM_BaseClass(void);
	virtual void AddSample(void *NewBuffer) = 0;
	virtual void *GetMedianImage() = 0;
protected:
	int NumSamples;
	int BufferSize;
	int BufferIndex;
	int GetIndexOfMedian();
};
