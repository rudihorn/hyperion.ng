#pragma once

// Qt includes
#include <QThreadPool>
#include <QRunnable>

// util includes
#include <utils/PixelFormat.h>
#include <utils/ImageResampler.h>

// TurboJPEG decoder
#ifdef HAVE_TURBO_JPEG
	#include <QImage>
	#include <QColor>
	#include <turbojpeg.h>
#endif

// Forward class declaration
class MFThreadManager;

/// Encoder thread for USB devices
class MFThread : public QObject, public QRunnable
{
	Q_OBJECT
	friend class MFThreadManager;

public:
	MFThread();
	~MFThread();

	void setup(
		PixelFormat pixelFormat, uint8_t* sharedData,
		int size, int width, int height, int lineLength,
		int subsamp, unsigned cropLeft, unsigned cropTop, unsigned cropBottom, unsigned cropRight,
		VideoMode videoMode, FlipMode flipMode, int pixelDecimation);
	void run();

	static volatile bool _isActive;

signals:
	void newFrame(const Image<ColorRgb>& data);

private:
	void processImageMjpeg();

#ifdef HAVE_TURBO_JPEG
	tjhandle			_transform,
			 			_decompress;
	tjscalingfactor*	_scalingFactors;
	tjtransform*		_xform;
#endif

	PixelFormat				_pixelFormat;
	uint8_t*				_localData, *_flipBuffer;
	int						_scalingFactorsCount, _width, _height, _lineLength, _subsamp, _currentFrame, _pixelDecimation;
	unsigned long			_size;
	unsigned				_cropLeft, _cropTop, _cropBottom, _cropRight;
	FlipMode				_flipMode;
	ImageResampler			_imageResampler;
};

class MFThreadManager : public QObject
{
	Q_OBJECT

public:
	MFThreadManager()
		: _threads(nullptr)
	{
		_threadCount = QThreadPool::globalInstance()->maxThreadCount();
	}

	~MFThreadManager()
	{
		stop();

		delete[] _threads;
		_threads = nullptr;
	}

	void start()
	{
		_threads = new MFThread*[_threadCount];
		for (int i=0; i < _threadCount; i++)
		{
			_threads[i] = new MFThread();
			_threads[i]->setAutoDelete(false);
		}
	}

	void stop()
	{
		QThreadPool::globalInstance()->clear();
		QThreadPool::globalInstance()->waitForDone();
	}

	int			_threadCount;
	MFThread**	_threads;
};
