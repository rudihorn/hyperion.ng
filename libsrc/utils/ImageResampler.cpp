#include "utils/ImageResampler.h"
#include <utils/Logger.h>

ImageResampler::ImageResampler()
	: _pixelDecimation(8)
	, _width(0)
	, _height(0)
	, _cropLeft(0)
	, _cropRight(0)
	, _cropTop(0)
	, _cropBottom(0)
	, _videoMode(VideoMode::VIDEO_2D)
	, _flipMode(FlipMode::NO_CHANGE)
	, _threadManager(new EncoderThreadManager(this))
{
	connect(_threadManager, &EncoderThreadManager::newFrame, this, &ImageResampler::newFrame);
	_threadManager->start();
}

ImageResampler::~ImageResampler()
{
		_threadManager->stop();
		disconnect(_threadManager, nullptr, nullptr, nullptr);
		delete _threadManager;
}

void ImageResampler::setWidthHeight(int width, int height)
{
	_width   = width;
	_height  = height;
}

void ImageResampler::setCropping(int cropLeft, int cropRight, int cropTop, int cropBottom)
{
	_cropLeft   = cropLeft;
	_cropRight  = cropRight;
	_cropTop    = cropTop;
	_cropBottom = cropBottom;
}

void ImageResampler::processImage(const uint8_t* data, size_t sampleSize, PixelFormat pixelFormat)
{
	for (int i = 0; i < _threadManager->_threadCount; i++)
	{
		if (!_threadManager->_threads[i]->isBusy())
		{
			_threadManager->_threads[i]->setup(pixelFormat, data, sampleSize, _width, _height, _cropLeft, _cropTop, _cropBottom, _cropRight, _videoMode, _flipMode, _pixelDecimation);
			_threadManager->_threads[i]->process();
			break;
		}
	}
}
