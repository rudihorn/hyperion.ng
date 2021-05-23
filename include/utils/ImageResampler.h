#pragma once

// util includes
#include <utils/VideoMode.h>
#include <utils/PixelFormat.h>
#include <utils/Image.h>
#include <utils/ColorRgb.h>
#include <utils/EncoderThread.h>

// Determine the cmake options
#include <HyperionConfig.h>

class ImageResampler: public QObject
{
	Q_OBJECT
public:
	ImageResampler();
	~ImageResampler();

	void setPixelDecimation(int decimator) { _pixelDecimation = decimator; }
	void setWidthHeight(int width, int height);
	void setCropping(int cropLeft, int cropRight, int cropTop, int cropBottom);
	void setVideoMode(VideoMode mode) { _videoMode = mode; }
	void setFlipMode(FlipMode mode) { _flipMode = mode; }
	void processImage(const uint8_t * data, size_t sampleSize, PixelFormat pixelFormat);

signals:
	void newFrame(const Image<ColorRgb>& data);

private:
	int						_pixelDecimation,
							_width,
							_height,
							_cropLeft,
							_cropRight,
							_cropTop,
							_cropBottom;
	VideoMode				_videoMode;
	FlipMode				_flipMode;
	EncoderThreadManager*	_threadManager;
};

