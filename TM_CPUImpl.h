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

#include <vector>

#include "tm_baseclass.h"

class TM_CPURefImpl :
	public TM_BaseClass
{
public:
	TM_CPURefImpl(int _NumSamples, int _BufferSize);
	~TM_CPURefImpl(void);
	virtual void AddSample(void *NewBuffer);
	virtual void *GetMedianImage();
protected:
	unsigned char *Buffers;
	unsigned char *MedianBuffer;
	std::vector<unsigned char> SortBuffer;
	unsigned char *MedianValuePtr;
	void BuildMedianImage(unsigned char *NewBuffer);
};

class TM_CPUImpl :
	public TM_CPURefImpl
{
public:
	TM_CPUImpl(int _NumSamples, int _BufferSize);
	virtual void AddSample(void *NewBuffer);
protected:
	void BuildMedianImageOpt3(void *NewBuffer);
	void BuildMedianImageOpt4(void *NewBuffer);
	void BuildMedianImageOpt5(void *NewBuffer);
	void BuildMedianImageOpt6(void *NewBuffer);
	void BuildMedianImageOpt7(void *NewBuffer);
	void BuildMedianImageOpt8(void *NewBuffer);
	void BuildMedianImageOpt9(void *NewBuffer);
};

