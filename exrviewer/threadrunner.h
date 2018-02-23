#pragma once

#include <ImfThreading.h>
#include <IlmThread.h>
#include <IlmThreadSemaphore.h>

#include "viewimage.h"

class ThreadRunner : public IlmThread::Thread
{
public:
	ThreadRunner() : IlmThread::Thread() {}
	ThreadRunner(ImageViewer *viewer, int index) :
		IlmThread::Thread()
	{
		Init(viewer, index);
	}

	virtual ~ThreadRunner()
	{
		Close();
		m_Semaphore.wait();
		CloseHandle(m_hComputeEvent);
		CloseHandle(m_hResumeEvent);
	}
	virtual void Init(ImageViewer *viewer, int index)
	{
		m_Worker = viewer;
		m_Index = index;
		m_hComputeEvent = CreateEventW(nullptr, false/*auto - reset*/, false, nullptr);
		m_hResumeEvent = CreateEventW(nullptr, false/*auto - reset*/, false, nullptr);
		m_bExit = false;
		start();
	}
	virtual void run()
	{
		m_Semaphore.post();
		while (!m_bExit)
		{
			WaitForSingleObject(m_hResumeEvent, INFINITE);
			m_Worker->RunThread(m_Index);
			SetEvent(m_hComputeEvent);
		}
	}

	virtual void Close()
	{
		Resume();
		m_bExit = true;
	}

	virtual void Resume()
	{
		SetEvent(m_hResumeEvent);
	}
	virtual void WaitSync()
	{
		WaitForSingleObject(m_hComputeEvent, INFINITE);
	}
private:
	int m_Index;
	ImageViewer *m_Worker;
	bool m_bExit;
	HANDLE m_hComputeEvent;
	HANDLE m_hResumeEvent;
	IlmThread::Semaphore m_Semaphore;
};
