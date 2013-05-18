// Time Median GPU
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include "TM_GPUImpl.h"

void CompareAndSwap(unsigned int& FirstData, unsigned int& SecondData) restrict(amp)
{
	unsigned int FirstResult  = min(FirstData & 0xFF000000, SecondData & 0xFF000000);
	unsigned int SecondResult = max(FirstData & 0xFF000000, SecondData & 0xFF000000); 
	FirstResult  |= min(FirstData & 0x00FF0000, SecondData & 0x00FF0000);
	SecondResult |= max(FirstData & 0x00FF0000, SecondData & 0x00FF0000);
	FirstResult  |= min(FirstData & 0x0000FF00, SecondData & 0x0000FF00);
	SecondResult |= max(FirstData & 0x0000FF00, SecondData & 0x0000FF00);
	FirstResult  |= min(FirstData & 0x000000FF, SecondData & 0x000000FF);
	SecondResult |= max(FirstData & 0x000000FF, SecondData & 0x000000FF);
	FirstData = FirstResult;
	SecondData = SecondResult;
}

void AddSampleAndComputeMedian(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int NumSamples, int BufferIndex, int MedianIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int MAX_MEDIAN_ELEMENTS = 25;
		unsigned int SortBuffer[MAX_MEDIAN_ELEMENTS];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NumSamples + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		for (int i = 0; i < NumSamples; i++)
			SortBuffer[i] = Samples[ElemIndex * NumSamples + i];
		// sort elements
		//for (int SortIteration = (NumSamples - 1); SortIteration > 0; SortIteration--)
		for (int SortIteration = (NumSamples - 1); SortIteration >= MedianIndex; SortIteration--)  //< do not waste time sorting elements before MedianIndex
			for (int j = 1; j <= SortIteration; j++)
				CompareAndSwap(SortBuffer[j-1], SortBuffer[j]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[MedianIndex];
	});
}

void AddSampleAndComputeMedianOpt3(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 3;
		unsigned int SortBuffer[NUM_SAMPLES];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		SortBuffer[0] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 2];
		// sort elements
		CompareAndSwap(SortBuffer[0], SortBuffer[1]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[0], SortBuffer[1]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[1];
	});
}

void AddSampleAndComputeMedianOpt4(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 4;
		unsigned int SortBuffer[NUM_SAMPLES + 1];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		//SortBuffer[0] = 0;
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 3];
		// sort elements
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[2], SortBuffer[3]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[2];
	});
}

void AddSampleAndComputeMedianOpt5(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 5;
		unsigned int SortBuffer[NUM_SAMPLES];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		SortBuffer[0] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 3];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 4];
		// sort elements
		CompareAndSwap(SortBuffer[0], SortBuffer[1]);
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		CompareAndSwap(SortBuffer[0], SortBuffer[3]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[2], SortBuffer[3]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[2];
	});
}

void AddSampleAndComputeMedianOpt6(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 6;
		unsigned int SortBuffer[NUM_SAMPLES + 1];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		//SortBuffer[0] = 0;
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 3];
		SortBuffer[5] = Samples[ElemIndex * NUM_SAMPLES + 4];
		SortBuffer[6] = Samples[ElemIndex * NUM_SAMPLES + 5];
		// sort elements
		CompareAndSwap(SortBuffer[1], SortBuffer[6]);
		CompareAndSwap(SortBuffer[2], SortBuffer[4]);
		CompareAndSwap(SortBuffer[3], SortBuffer[5]);
		CompareAndSwap(SortBuffer[2], SortBuffer[6]);
		CompareAndSwap(SortBuffer[2], SortBuffer[3]);
		CompareAndSwap(SortBuffer[3], SortBuffer[6]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[1], SortBuffer[3]);
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[3];
	});
}

void AddSampleAndComputeMedianOpt7(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 7;
		unsigned int SortBuffer[NUM_SAMPLES];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		SortBuffer[0] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 3];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 4];
		SortBuffer[5] = Samples[ElemIndex * NUM_SAMPLES + 5];
		SortBuffer[6] = Samples[ElemIndex * NUM_SAMPLES + 6];
		// sort elements
		CompareAndSwap(SortBuffer[0], SortBuffer[5]);
		CompareAndSwap(SortBuffer[0], SortBuffer[3]);
		CompareAndSwap(SortBuffer[1], SortBuffer[6]);
		CompareAndSwap(SortBuffer[2], SortBuffer[4]);
		CompareAndSwap(SortBuffer[0], SortBuffer[1]);
		CompareAndSwap(SortBuffer[3], SortBuffer[5]);
		CompareAndSwap(SortBuffer[2], SortBuffer[6]);
		CompareAndSwap(SortBuffer[2], SortBuffer[3]);
		CompareAndSwap(SortBuffer[3], SortBuffer[6]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[1], SortBuffer[3]);
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[3];
	});
}

void AddSampleAndComputeMedianOpt8(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 8;
		unsigned int SortBuffer[NUM_SAMPLES + 1];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		//SortBuffer[0] = 0;
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 3];
		SortBuffer[5] = Samples[ElemIndex * NUM_SAMPLES + 4];
		SortBuffer[6] = Samples[ElemIndex * NUM_SAMPLES + 5];
		SortBuffer[7] = Samples[ElemIndex * NUM_SAMPLES + 6];
		SortBuffer[8] = Samples[ElemIndex * NUM_SAMPLES + 7];
		// sort elements
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[7], SortBuffer[8]);
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		CompareAndSwap(SortBuffer[6], SortBuffer[7]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[7], SortBuffer[8]);
		CompareAndSwap(SortBuffer[5], SortBuffer[8]);
		CompareAndSwap(SortBuffer[4], SortBuffer[7]);
		CompareAndSwap(SortBuffer[3], SortBuffer[6]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[2], SortBuffer[5]);
		CompareAndSwap(SortBuffer[4], SortBuffer[7]);
		CompareAndSwap(SortBuffer[4], SortBuffer[2]);
		CompareAndSwap(SortBuffer[6], SortBuffer[4]);
		CompareAndSwap(SortBuffer[4], SortBuffer[2]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[4];
	});
}

void AddSampleAndComputeMedianOpt9(array<unsigned int, 1>& Samples, array_view<unsigned int, 1>& NewBuffer, array_view<unsigned int, 1>& MedianBuffer, int BufferIndex)
{
	parallel_for_each(MedianBuffer.extent, [=, &Samples](index<1> idx) restrict(amp)
	{
		const int NUM_SAMPLES = 9;
		unsigned int SortBuffer[NUM_SAMPLES];
		unsigned int ElemIndex = idx[0];
		Samples[ElemIndex * NUM_SAMPLES + BufferIndex] = NewBuffer[ElemIndex];
		// copy elements
		SortBuffer[0] = Samples[ElemIndex * NUM_SAMPLES    ];
		SortBuffer[1] = Samples[ElemIndex * NUM_SAMPLES + 1];
		SortBuffer[2] = Samples[ElemIndex * NUM_SAMPLES + 2];
		SortBuffer[3] = Samples[ElemIndex * NUM_SAMPLES + 3];
		SortBuffer[4] = Samples[ElemIndex * NUM_SAMPLES + 4];
		SortBuffer[5] = Samples[ElemIndex * NUM_SAMPLES + 5];
		SortBuffer[6] = Samples[ElemIndex * NUM_SAMPLES + 6];
		SortBuffer[7] = Samples[ElemIndex * NUM_SAMPLES + 7];
		SortBuffer[8] = Samples[ElemIndex * NUM_SAMPLES + 8];
		// sort elements
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[7], SortBuffer[8]);
		CompareAndSwap(SortBuffer[0], SortBuffer[1]);
		CompareAndSwap(SortBuffer[3], SortBuffer[4]);
		CompareAndSwap(SortBuffer[6], SortBuffer[7]);
		CompareAndSwap(SortBuffer[1], SortBuffer[2]);
		CompareAndSwap(SortBuffer[4], SortBuffer[5]);
		CompareAndSwap(SortBuffer[7], SortBuffer[8]);
		CompareAndSwap(SortBuffer[0], SortBuffer[3]);
		CompareAndSwap(SortBuffer[5], SortBuffer[8]);
		CompareAndSwap(SortBuffer[4], SortBuffer[7]);
		CompareAndSwap(SortBuffer[3], SortBuffer[6]);
		CompareAndSwap(SortBuffer[1], SortBuffer[4]);
		CompareAndSwap(SortBuffer[2], SortBuffer[5]);
		CompareAndSwap(SortBuffer[4], SortBuffer[7]);
		CompareAndSwap(SortBuffer[4], SortBuffer[2]);
		CompareAndSwap(SortBuffer[6], SortBuffer[4]);
		CompareAndSwap(SortBuffer[4], SortBuffer[2]);
		// extract median value
		MedianBuffer[ElemIndex] = SortBuffer[4];
	});
}

TM_GPUImpl::TM_GPUImpl(int _NumSamples, int _BufferSize) : TM_BaseClass(_NumSamples, _BufferSize)
{
	GPUBufferSize = BufferSize >> 2;  //< uses ints, not chars
	Samples = new array<unsigned int, 1>(GPUBufferSize * NumSamples);

#ifdef USE_STAGING
	accelerator_view cpuAccView = accelerator(accelerator::cpu_accelerator).default_view;
	accelerator gpuAccel(accelerator::default_accelerator);
	StagingData = new array<unsigned int, 1>(GPUBufferSize, cpuAccView, gpuAccel.default_view);
	MedianBufferView = new array_view<unsigned int, 1>(*StagingData);
#else
	MedianBuffer = new unsigned char[BufferSize];
	MedianBufferView = new array_view<unsigned int, 1>(GPUBufferSize, reinterpret_cast<unsigned int*>(MedianBuffer));
#endif
	MedianBufferView->discard_data();

	switch (NumSamples) {
	case 3 : OptimalStrategy = &AddSampleAndComputeMedianOpt3;
			break;
	case 4 : OptimalStrategy = &AddSampleAndComputeMedianOpt4;
			break;
	case 5 : OptimalStrategy = &AddSampleAndComputeMedianOpt5;
			break;
	case 6 : OptimalStrategy = &AddSampleAndComputeMedianOpt6;
			break;
	case 7 : OptimalStrategy = &AddSampleAndComputeMedianOpt7;
			break;
	case 8 : OptimalStrategy = &AddSampleAndComputeMedianOpt8;
			break;
	case 9 : OptimalStrategy = &AddSampleAndComputeMedianOpt9;
			break;
	default : OptimalStrategy = false;
	}
}

TM_GPUImpl::~TM_GPUImpl(void)
{
	MedianBufferView->discard_data();
	delete MedianBufferView;
#ifdef USE_STAGING
	delete StagingData;
#else
	delete[] MedianBuffer;
#endif
	delete Samples;
}

void TM_GPUImpl::Wait()
{
	Samples->accelerator_view.wait();
}

void TM_GPUImpl::AddSample(void *NewBuffer)
{
	if (NewBuffer == nullptr)
		return;

	DirtyBuffers = true;  //< starting a new computation, so mark output MedianBuffer as dirty
	array_view<unsigned int, 1> NewBufferView(GPUBufferSize, reinterpret_cast<unsigned int*>(NewBuffer));
	if (OptimalStrategy)
		OptimalStrategy(*Samples, NewBufferView, *MedianBufferView, BufferIndex);
	else
		AddSampleAndComputeMedian(*Samples, NewBufferView, *MedianBufferView, NumSamples, BufferIndex, GetIndexOfMedian());
	NewBufferView.discard_data();  //< does not synchronize NewBuffer when NewBufferView is destroyed
	BufferIndex++;
	if (BufferIndex >= NumSamples)
		BufferIndex = 0;
}

void * TM_GPUImpl::GetMedianImage()
{
	if (DirtyBuffers)
	{
		MedianBufferView->synchronize();  //< if MedianBuffer was recomputed, copy it from GPU
		DirtyBuffers = false;
	}
#ifdef USE_STAGING
	return StagingData->data();
#else
	return MedianBuffer;
#endif
}

/// TM_GPUAsyncImpl

TM_GPUAsyncImpl::~TM_GPUAsyncImpl(void)
{
}

void TM_GPUAsyncImpl::AddSample(void *NewBuffer)
{
	if (NewBuffer == nullptr)
		return;

	DirtyBuffers = true;  //< starting a new computation, so mark output MedianBuffer as dirty
	array_view<unsigned int, 1> NewBufferView(GPUBufferSize, reinterpret_cast<unsigned int*>(NewBuffer));
	if (OptimalStrategy)
		OptimalStrategy(*Samples, NewBufferView, *MedianBufferView, BufferIndex);
	else
		AddSampleAndComputeMedian(*Samples, NewBufferView, *MedianBufferView, NumSamples, BufferIndex, GetIndexOfMedian());
	NewBufferView.discard_data();  //< does not synchronize NewBuffer when NewBufferView is destroyed
	MedianBufferUpdated = MedianBufferView->synchronize_async();
	BufferIndex++;
	if (BufferIndex >= NumSamples)
		BufferIndex = 0;
}

void * TM_GPUAsyncImpl::GetMedianImage()
{
	if (DirtyBuffers)
	{
		MedianBufferUpdated.get();
		DirtyBuffers = false;
	}
#ifdef USE_STAGING
	return StagingData->data();
#else
	return MedianBuffer;
#endif
}