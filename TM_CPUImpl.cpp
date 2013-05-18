// Time Median GPU
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include <algorithm>

#include "TM_CPUImpl.h"

#include <emmintrin.h>

// TM_CPURefImpl

TM_CPURefImpl::TM_CPURefImpl(int _NumSamples, int _BufferSize) : TM_BaseClass(_NumSamples, _BufferSize)
{
	Buffers = new unsigned char[NumSamples * BufferSize];  //< stores NumSamples buffers of the given BufferSize, interleaving data from different buffers
	MedianBuffer = new unsigned char[BufferSize];
	SortBuffer.resize(NumSamples);
	MedianValuePtr = &SortBuffer[GetIndexOfMedian()];
}

TM_CPURefImpl::~TM_CPURefImpl(void)
{
	delete[] Buffers;
	delete[] MedianBuffer;
}

void TM_CPURefImpl::AddSample(void *NewBuffer)
{
	if (NewBuffer == nullptr)
		return;
	
	BuildMedianImage(reinterpret_cast<unsigned char *>(NewBuffer));

	BufferIndex++;
	if (BufferIndex >= NumSamples)
		BufferIndex = 0;
}

void * TM_CPURefImpl::GetMedianImage()
{
	return MedianBuffer;
}

int CompareUnsignedChars(const void * a, const void * b)
{
	return (*(unsigned char*)a - *(unsigned char*)b);
}

void TM_CPURefImpl::BuildMedianImage(unsigned char *NewBuffer)
{
	for (int i = 0; i < BufferSize; i++)
	{
		unsigned char *BuffersSection = &Buffers[i * NumSamples];
		BuffersSection[BufferIndex] = NewBuffer[i];  //< update Buffers 
		std::memcpy(SortBuffer.data(), BuffersSection, NumSamples * sizeof(unsigned char));
		std::sort(SortBuffer.begin(), SortBuffer.end());
		MedianBuffer[i] = *MedianValuePtr;
	}
}

// TM_CPUImpl

TM_CPUImpl::TM_CPUImpl(int _NumSamples, int _BufferSize) : TM_CPURefImpl(_NumSamples, _BufferSize) 
{
	_ASSERT((BufferSize & 16) == 0);  //< SSE2 code processes 16 bytes at a time
};

__forceinline void CompareAndSwap(__m128i& FirstData, __m128i& SecondData)
{
	__m128i FirstResult = _mm_min_epu8(FirstData, SecondData);
	__m128i SecondResult = _mm_max_epu8(FirstData, SecondData);
	FirstData = FirstResult;
	SecondData = SecondResult;
}

void TM_CPUImpl::BuildMedianImageOpt3(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		__m128i SortBuffer0 = BuffersSection[0];
		__m128i SortBuffer1 = BuffersSection[1];
		__m128i SortBuffer2 = BuffersSection[2];
		// sort elements
		CompareAndSwap(SortBuffer0, SortBuffer1);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer0, SortBuffer1);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer1;
	}
}

void TM_CPUImpl::BuildMedianImageOpt4(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		// just like BuildMedianImageOpt5, but SortBuffer0 is filled with zeroes so comparisons are unnecessary
		//__m128i SortBuffer0 = _mm_setzero_si128();
		__m128i SortBuffer1 = BuffersSection[0];
		__m128i SortBuffer2 = BuffersSection[1];
		__m128i SortBuffer3 = BuffersSection[2];
		__m128i SortBuffer4 = BuffersSection[3];
		// sort elements
		CompareAndSwap(SortBuffer3, SortBuffer4);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer2, SortBuffer3);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer2;
	}
}

void TM_CPUImpl::BuildMedianImageOpt5(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		__m128i SortBuffer0 = BuffersSection[0];
		__m128i SortBuffer1 = BuffersSection[1];
		__m128i SortBuffer2 = BuffersSection[2];
		__m128i SortBuffer3 = BuffersSection[3];
		__m128i SortBuffer4 = BuffersSection[4];
		// sort elements
		CompareAndSwap(SortBuffer0, SortBuffer1);
		CompareAndSwap(SortBuffer3, SortBuffer4);
		CompareAndSwap(SortBuffer0, SortBuffer3);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer2, SortBuffer3);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer2;
	}
}

void TM_CPUImpl::BuildMedianImageOpt6(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		// just like BuildMedianImageOpt7, but SortBuffer0 is filled with zeroes so comparisons are unnecessary
		//__m128i SortBuffer0 = _mm_setzero_si128();
		__m128i SortBuffer1 = BuffersSection[0];
		__m128i SortBuffer2 = BuffersSection[1];
		__m128i SortBuffer3 = BuffersSection[2];
		__m128i SortBuffer4 = BuffersSection[3];
		__m128i SortBuffer5 = BuffersSection[4];
		__m128i SortBuffer6 = BuffersSection[5];
		// sort elements
		CompareAndSwap(SortBuffer1, SortBuffer6);
		CompareAndSwap(SortBuffer2, SortBuffer4);
		CompareAndSwap(SortBuffer3, SortBuffer5);
		CompareAndSwap(SortBuffer2, SortBuffer6);
		CompareAndSwap(SortBuffer2, SortBuffer3);
		CompareAndSwap(SortBuffer3, SortBuffer6);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer1, SortBuffer3);
		CompareAndSwap(SortBuffer3, SortBuffer4);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer3;
	}
}

void TM_CPUImpl::BuildMedianImageOpt7(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		__m128i SortBuffer0 = BuffersSection[0];
		__m128i SortBuffer1 = BuffersSection[1];
		__m128i SortBuffer2 = BuffersSection[2];
		__m128i SortBuffer3 = BuffersSection[3];
		__m128i SortBuffer4 = BuffersSection[4];
		__m128i SortBuffer5 = BuffersSection[5];
		__m128i SortBuffer6 = BuffersSection[6];
		// sort elements
		CompareAndSwap(SortBuffer0, SortBuffer5);
		CompareAndSwap(SortBuffer0, SortBuffer3);
		CompareAndSwap(SortBuffer1, SortBuffer6);
		CompareAndSwap(SortBuffer2, SortBuffer4);
		CompareAndSwap(SortBuffer0, SortBuffer1);
		CompareAndSwap(SortBuffer3, SortBuffer5);
		CompareAndSwap(SortBuffer2, SortBuffer6);
		CompareAndSwap(SortBuffer2, SortBuffer3);
		CompareAndSwap(SortBuffer3, SortBuffer6);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer1, SortBuffer3);
		CompareAndSwap(SortBuffer3, SortBuffer4);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer3;
	}
}

void TM_CPUImpl::BuildMedianImageOpt8(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		// just like BuildMedianImageOpt9, but SortBuffer0 is filled with zeroes so comparisons are unnecessary
		//__m128i SortBuffer0 = _mm_setzero_si128();
		__m128i SortBuffer1 = BuffersSection[0];
		__m128i SortBuffer2 = BuffersSection[1];
		__m128i SortBuffer3 = BuffersSection[2];
		__m128i SortBuffer4 = BuffersSection[3];
		__m128i SortBuffer5 = BuffersSection[4];
		__m128i SortBuffer6 = BuffersSection[5];
		__m128i SortBuffer7 = BuffersSection[6];
		__m128i SortBuffer8 = BuffersSection[7];
		// sort elements
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer7, SortBuffer8);
		CompareAndSwap(SortBuffer3, SortBuffer4);
		CompareAndSwap(SortBuffer6, SortBuffer7);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer7, SortBuffer8);
		CompareAndSwap(SortBuffer5, SortBuffer8);
		CompareAndSwap(SortBuffer4, SortBuffer7);
		CompareAndSwap(SortBuffer3, SortBuffer6);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer2, SortBuffer5);
		CompareAndSwap(SortBuffer4, SortBuffer7);
		CompareAndSwap(SortBuffer4, SortBuffer2);
		CompareAndSwap(SortBuffer6, SortBuffer4);
		CompareAndSwap(SortBuffer4, SortBuffer2);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer4;
	}
}

void TM_CPUImpl::BuildMedianImageOpt9(void *NewBuffer)
{
	__m128i *VectorNewBuffer = reinterpret_cast<__m128i *>(NewBuffer);
	__m128i *VectorMedianBuffer = reinterpret_cast<__m128i *>(MedianBuffer);
	__m128i *VectorBuffers = reinterpret_cast<__m128i *>(Buffers);

	int VectorBufferSize = BufferSize / sizeof(__m128i);
	for (int i = 0; i < VectorBufferSize; i++)
	{
		__m128i *BuffersSection = &VectorBuffers[i * NumSamples];
		BuffersSection[BufferIndex] = VectorNewBuffer[i];
		// copy elements
		__m128i SortBuffer0 = BuffersSection[0];
		__m128i SortBuffer1 = BuffersSection[1];
		__m128i SortBuffer2 = BuffersSection[2];
		__m128i SortBuffer3 = BuffersSection[3];
		__m128i SortBuffer4 = BuffersSection[4];
		__m128i SortBuffer5 = BuffersSection[5];
		__m128i SortBuffer6 = BuffersSection[6];
		__m128i SortBuffer7 = BuffersSection[7];
		__m128i SortBuffer8 = BuffersSection[8];
		// sort elements
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer7, SortBuffer8);
		CompareAndSwap(SortBuffer0, SortBuffer1);
		CompareAndSwap(SortBuffer3, SortBuffer4);
		CompareAndSwap(SortBuffer6, SortBuffer7);
		CompareAndSwap(SortBuffer1, SortBuffer2);
		CompareAndSwap(SortBuffer4, SortBuffer5);
		CompareAndSwap(SortBuffer7, SortBuffer8);
		CompareAndSwap(SortBuffer0, SortBuffer3);
		CompareAndSwap(SortBuffer5, SortBuffer8);
		CompareAndSwap(SortBuffer4, SortBuffer7);
		CompareAndSwap(SortBuffer3, SortBuffer6);
		CompareAndSwap(SortBuffer1, SortBuffer4);
		CompareAndSwap(SortBuffer2, SortBuffer5);
		CompareAndSwap(SortBuffer4, SortBuffer7);
		CompareAndSwap(SortBuffer4, SortBuffer2);
		CompareAndSwap(SortBuffer6, SortBuffer4);
		CompareAndSwap(SortBuffer4, SortBuffer2);
		// extract median value
		VectorMedianBuffer[i] = SortBuffer4;
	}
}

void TM_CPUImpl::AddSample(void *NewBuffer)
{
	if (NewBuffer == nullptr)
		return;
	
	switch (NumSamples) {
	case 3 : BuildMedianImageOpt3(NewBuffer);
			break;
	case 4 : BuildMedianImageOpt4(NewBuffer);
			break;
	case 5 : BuildMedianImageOpt5(NewBuffer);
			break;
	case 6 : BuildMedianImageOpt6(NewBuffer);
			break;
	case 7 : BuildMedianImageOpt7(NewBuffer);
			break;
	case 8 : BuildMedianImageOpt8(NewBuffer);
			break;
	case 9 : BuildMedianImageOpt9(NewBuffer);
			break;
	default : BuildMedianImage(reinterpret_cast<unsigned char *>(NewBuffer));
	}
	BufferIndex++;
	if (BufferIndex >= NumSamples)
		BufferIndex = 0;
}