// Time Median GPU
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

// TimeMedianGPU.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;

#include "TM_CPUImpl.h"
#include "TM_GPUImpl.h"

void FillRandomBuffer(unsigned char *Buffer, int BufferSize)
{
	srand(time(NULL));
	for (int j = 0; j < BufferSize; j++)
		Buffer[j] = rand() & 0xFF;
}

int ElapsedTimeToMSec(int ElapsedTime)
{
	LARGE_INTEGER TimerFrequency;
	LARGE_INTEGER LargeElapsedTime;
	LargeElapsedTime.HighPart = 0;
	LargeElapsedTime.LowPart = ElapsedTime;
	QueryPerformanceFrequency(&TimerFrequency);
	LargeElapsedTime.QuadPart = (LargeElapsedTime.QuadPart * 1000) / TimerFrequency.QuadPart;
	return LargeElapsedTime.LowPart;
}

void PrintBenchmarkResults(vector<int>& BenchmarkSamples)
{
	sort(BenchmarkSamples.begin(), BenchmarkSamples.end());
	cout << ElapsedTimeToMSec(BenchmarkSamples[BenchmarkSamples.size() >> 1]) << " ms (";
	for (auto item = BenchmarkSamples.begin(); item != BenchmarkSamples.end(); ++item)
		cout << ElapsedTimeToMSec(*item) << "ms  ";
	cout << ")" << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "TimeMedianGPU by Stefano Tommesani www.tommesani.com" << endl;
	const int SAMPLE_SIZE_MB = 16;
	const int SAMPLE_SIZE = SAMPLE_SIZE_MB * 1024 * 1024;
	const int BENCHMARK_SAMPLES = 5;
	unsigned char *InputBuffer = new unsigned char[SAMPLE_SIZE];

	for (int NumSamples = 3; NumSamples <= 9; NumSamples++)
	{
		cout << " Testing " << NumSamples << " samples of " << SAMPLE_SIZE_MB << " MB" << endl;
		TM_GPUImpl *GPUImpl = new TM_GPUImpl(NumSamples, SAMPLE_SIZE);
		TM_CPUImpl *CPUImpl = new TM_CPUImpl(NumSamples, SAMPLE_SIZE);
		TM_CPURefImpl *CPURefImpl = new TM_CPURefImpl(NumSamples, SAMPLE_SIZE);
		TM_GPUAsyncImpl *GPUAsyncImpl = new TM_GPUAsyncImpl(NumSamples, SAMPLE_SIZE);
		LARGE_INTEGER CPUStartTime, CPUStopTime, CPURefStartTime, CPURefStopTime, GPUStartTime, GPUStopTime, GPUAddSampleStart, GPUAddSampleStop, GPUGetMedianImageStart, GPUGetMedianImageStop;
		vector<int> CPUBenchmarkSamples;
		vector<int> CPURefBenchmarkSamples;
		vector<int> GPUBenchmarkSamples;
		vector<int> GPUBenchmarkAddSampleSamples;
		vector<int> GPUBenchmarkGetMedianImageSamples;

		for (int i = 0; i < (NumSamples + BENCHMARK_SAMPLES); i++)
		{
			FillRandomBuffer(InputBuffer, SAMPLE_SIZE);

			QueryPerformanceCounter(&CPURefStartTime);
			CPURefImpl->AddSample(InputBuffer);
			void *CPURefImage = CPURefImpl->GetMedianImage();
			QueryPerformanceCounter(&CPURefStopTime);

			// CPU code 
			QueryPerformanceCounter(&CPUStartTime);
			CPUImpl->AddSample(InputBuffer);
			void *CPUImage = CPUImpl->GetMedianImage();
			QueryPerformanceCounter(&CPUStopTime);

			// GPU code
			GPUImpl->Wait();  //< wait for running GPU tasks to complete
			QueryPerformanceCounter(&GPUStartTime);
			GPUImpl->AddSample(InputBuffer);
			void *GPUImage = GPUImpl->GetMedianImage();
			QueryPerformanceCounter(&GPUStopTime);

			// GPU async code
			GPUAsyncImpl->Wait();  //< wait for running GPU tasks to complete
			QueryPerformanceCounter(&GPUAddSampleStart);
			GPUAsyncImpl->AddSample(InputBuffer);	//< step 1: load image into the GPU and start computation
			QueryPerformanceCounter(&GPUAddSampleStop);
			Sleep(50);	//< do some work on the GPU
			QueryPerformanceCounter(&GPUGetMedianImageStart);
			void *OtherGPUImage = GPUAsyncImpl->GetMedianImage();	//< step 2: get the result image from the GPU
			QueryPerformanceCounter(&GPUGetMedianImageStop);

			if (i >= NumSamples)
			{
				// verify results
				if (memcmp(CPURefImage, CPUImage, SAMPLE_SIZE) != 0)
					cout << "Error (CPU code)!" << endl;
				if (memcmp(CPURefImage, GPUImage, SAMPLE_SIZE) != 0)
					cout << "Error (GPU code)!" << endl;
				if (memcmp(CPURefImage, OtherGPUImage, SAMPLE_SIZE) != 0)
					cout << "Error (GPU async code)!" << endl;

				// save benchmark data
				int ElapsedGPUTime = (int)(GPUStopTime.QuadPart - GPUStartTime.QuadPart);
				GPUBenchmarkSamples.push_back(ElapsedGPUTime);
				int ElapsedCPUTime = (int)(CPUStopTime.QuadPart - CPUStartTime.QuadPart);
				CPUBenchmarkSamples.push_back(ElapsedCPUTime);
				int ElapsedCPURefTime = (int)(CPURefStopTime.QuadPart - CPURefStartTime.QuadPart);
				CPURefBenchmarkSamples.push_back(ElapsedCPURefTime);
				int ElapsedGPUAddSampleTime = (int)(GPUAddSampleStop.QuadPart - GPUAddSampleStart.QuadPart);
				GPUBenchmarkAddSampleSamples.push_back(ElapsedGPUAddSampleTime);
				int ElapsedGPUGetMedianImageTime = (int)(GPUGetMedianImageStop.QuadPart - GPUGetMedianImageStart.QuadPart);
				GPUBenchmarkGetMedianImageSamples.push_back(ElapsedGPUGetMedianImageTime);
			}
		}

		cout << "  CPU time (ref)   ";
		PrintBenchmarkResults(CPURefBenchmarkSamples);
		cout << "  CPU time (SSE)   ";
		PrintBenchmarkResults(CPUBenchmarkSamples);
		cout << "  GPU time         ";
		PrintBenchmarkResults(GPUBenchmarkSamples);
		cout << "  GPU Async 1 time ";
		PrintBenchmarkResults(GPUBenchmarkAddSampleSamples);
		cout << "  GPU Async 2 time ";
		PrintBenchmarkResults(GPUBenchmarkGetMedianImageSamples);

		delete GPUImpl;
		delete CPUImpl;
		delete GPUAsyncImpl;
		delete CPURefImpl;
	}
	delete[] InputBuffer;
	cout << "Testing completed" << endl;
	char WaitForUser;
	cin >> WaitForUser;
	return 0;
}
