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

#include "boost/function.hpp"

#include <amp.h>
using namespace concurrency;

#include "tm_baseclass.h"

//#define USE_STAGING

class TM_GPUImpl :
	public TM_BaseClass
{
public:
	TM_GPUImpl(int _NumSamples, int _BufferSize);
	~TM_GPUImpl(void);
	virtual void AddSample(void *NewBuffer);
	virtual void *GetMedianImage();
	void Wait();
protected:
#ifdef USE_STAGING
	array<unsigned int, 1> *StagingData;  //< hosts MedianBuffer staging buffer
#else
	unsigned char *MedianBuffer;
#endif
	int GPUBufferSize;
	bool DirtyBuffers;
	array<unsigned int, 1> *Samples;
	array_view<unsigned int, 1> *MedianBufferView;
	boost::function<void (array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)> OptimalStrategy;
};

class TM_GPUAsyncImpl : 
	public TM_GPUImpl
{
public:
	TM_GPUAsyncImpl(int _NumSamples, int _BufferSize) : TM_GPUImpl(_NumSamples, _BufferSize) {};
	~TM_GPUAsyncImpl(void);
	virtual void AddSample(void *NewBuffer);
	virtual void *GetMedianImage();
private:
	completion_future MedianBufferUpdated;
};

