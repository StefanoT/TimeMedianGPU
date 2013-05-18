// Time Median GPU
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include "TM_BaseClass.h"

TM_BaseClass::TM_BaseClass(int _NumSamples, int _BufferSize)
{
	_ASSERT(_NumSamples >= 3);

	NumSamples = _NumSamples;
	BufferSize = _BufferSize;
	BufferIndex = 0;
}

TM_BaseClass::~TM_BaseClass(void)
{
}

int TM_BaseClass::GetIndexOfMedian()
{
	// [3]->1  [4]->1  [5]->2  [6]->2  [7]->3
	return (NumSamples - 1) >> 1;
}
