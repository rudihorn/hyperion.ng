#pragma once

#include <mfapi.h>
#include <dmo.h>
#include <wmcodecdsp.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <mferror.h>
#include <strmif.h>
#include <comdef.h>

#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "mf.lib")
#pragma comment (lib, "mfplat.lib")
#pragma comment (lib, "mfuuid.lib")
#pragma comment (lib, "mfreadwrite.lib")
#pragma comment (lib, "strmiids.lib")
#pragma comment (lib, "wmcodecdspuuid.lib")

#include <grabber/MFGrabber.h>

#define SAFE_RELEASE(x) if(x) { x->Release(); x = nullptr; }

static PixelFormat GetPixelFormatForGuid(const GUID guid)
{
	if (IsEqualGUID(guid, MFVideoFormat_RGB32)) return PixelFormat::RGB32;
	if (IsEqualGUID(guid, MFVideoFormat_RGB24)) return PixelFormat::BGR24;
	if (IsEqualGUID(guid, MFVideoFormat_YUY2)) return PixelFormat::YUYV;
	if (IsEqualGUID(guid, MFVideoFormat_UYVY)) return PixelFormat::UYVY;
	if (IsEqualGUID(guid, MFVideoFormat_MJPG)) return  PixelFormat::MJPEG;
	if (IsEqualGUID(guid, MFVideoFormat_NV12)) return  PixelFormat::NV12;
	if (IsEqualGUID(guid, MFVideoFormat_I420)) return  PixelFormat::I420;
	return PixelFormat::NO_CHANGE;
};

class SourceReaderCB : public IMFSourceReaderCallback
{
public:
	SourceReaderCB(MFGrabber* grabber)
		: _nRefCount(1)
		, _grabber(grabber)
		, _hrStatus(S_OK)
		, _isBusy(false)
	{
		// Initialize critical section.
		InitializeCriticalSection(&_critsec);
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(SourceReaderCB, IMFSourceReaderCallback),
			{ 0 },
		};
		return QISearch(this, qit, iid, ppv);
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_nRefCount);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG uCount = InterlockedDecrement(&_nRefCount);
		if (uCount == 0)
		{
			delete this;
		}
		return uCount;
	}

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD /*dwStreamIndex*/,
		DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample* pSample)
	{
		EnterCriticalSection(&_critsec);
		_isBusy = true;

		if (_grabber->_sourceReader == nullptr)
		{
			_isBusy = false;
			LeaveCriticalSection(&_critsec);
			return S_FALSE;
		}

		if (dwStreamFlags & MF_SOURCE_READERF_STREAMTICK)
		{
			Debug(_grabber->_log, "Skipping stream gap");
			LeaveCriticalSection(&_critsec);
			_grabber->_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
			return S_OK;
		}

		// Variables declaration
		IMFMediaBuffer* buffer = nullptr;

		if (FAILED(hrStatus))
		{
			_hrStatus = hrStatus;
			_com_error error(_hrStatus);
			Error(_grabber->_log, "%s", error.ErrorMessage());
			goto done;
		}

		if (!pSample)
		{
			Error(_grabber->_log, "Media sample is empty");
			goto done;
		}

		_hrStatus = pSample->ConvertToContiguousBuffer(&buffer);
		if (FAILED(_hrStatus))
		{
			_com_error error(_hrStatus);
			Error(_grabber->_log, "Buffer conversion failed => %s", error.ErrorMessage());
			goto done;
		}

		BYTE* data = nullptr;
		DWORD maxLength = 0, currentLength = 0;
		_hrStatus = buffer->Lock(&data, &maxLength, &currentLength);
		if (FAILED(_hrStatus))
		{
			_com_error error(_hrStatus);
			Error(_grabber->_log, "Access to the buffer memory failed => %s", error.ErrorMessage());
			goto done;
		}

		_grabber->receive_image(data, currentLength);

		_hrStatus = buffer->Unlock();
		if (FAILED(_hrStatus))
		{
			_com_error error(_hrStatus);
			Error(_grabber->_log, "Unlocking the buffer memory failed => %s", error.ErrorMessage());
		}

	done:
		SAFE_RELEASE(buffer);

		_isBusy = false;
		LeaveCriticalSection(&_critsec);
		return _hrStatus;
	}

	BOOL SourceReaderCB::isBusy()
	{
		EnterCriticalSection(&_critsec);
		BOOL result = _isBusy;
		LeaveCriticalSection(&_critsec);

		return result;
	}

	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) { return S_OK; }
	STDMETHODIMP OnFlush(DWORD) { return S_OK; }

private:
	virtual ~SourceReaderCB() { DeleteCriticalSection(&_critsec); }

private:
	long				_nRefCount;
	CRITICAL_SECTION	_critsec;
	MFGrabber*			_grabber;
	BOOL				_bEOS;
	HRESULT				_hrStatus;
	std::atomic<bool>	_isBusy;
};
