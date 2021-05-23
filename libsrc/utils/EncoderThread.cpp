#include "utils/EncoderThread.h"

EncoderThread::EncoderThread()
	: _data(nullptr)
	, _size(0)
{
	_videoFormatFourCCMap.insert(PixelFormat::YUYV,  libyuv::FOURCC_YUY2);
	_videoFormatFourCCMap.insert(PixelFormat::UYVY,  libyuv::FOURCC_UYVY);
	_videoFormatFourCCMap.insert(PixelFormat::BGR16, libyuv::FOURCC_RGBP);
	_videoFormatFourCCMap.insert(PixelFormat::BGR24, libyuv::FOURCC_24BG);
	_videoFormatFourCCMap.insert(PixelFormat::RGB32, libyuv::FOURCC_RGBA);
	_videoFormatFourCCMap.insert(PixelFormat::BGR32, libyuv::FOURCC_BGRA);
	_videoFormatFourCCMap.insert(PixelFormat::NV12,  libyuv::FOURCC_NV12);
	_videoFormatFourCCMap.insert(PixelFormat::I420,  libyuv::FOURCC_I420);
	_videoFormatFourCCMap.insert(PixelFormat::MJPEG, libyuv::FOURCC_MJPG);
}

EncoderThread::~EncoderThread()
{
	if (_data != nullptr)
		free(_data);
}

void EncoderThread::setup(
	PixelFormat pixelFormat, const uint8_t* data,
	size_t sampleSize, int width, int height,
	unsigned cropLeft, unsigned cropTop, unsigned cropBottom, unsigned cropRight,
	VideoMode videoMode, FlipMode flipMode, int pixelDecimation)
{
	_pixelFormat = pixelFormat;
	_size = sampleSize;
	_width = width;
	_height = height;
	_cropLeft = cropLeft;
	_cropTop = cropTop;
	_cropBottom = cropBottom;
	_cropRight = cropRight;
	_videoMode = videoMode;
	_flipMode = flipMode;
	_pixelDecimation = pixelDecimation;

	if (_data != nullptr)
	{
		free(_data);
		_data = nullptr;
	}

	_data = (uint8_t*)malloc((size_t)_size + 1);
	memcpy(_data, data, _size);
}

void EncoderThread::process()
{
	_busy = true;
	if (_width > 0 && _height > 0)
	{
		int cropRight  = _cropRight;
		int cropBottom = _cropBottom;

		// handle 3D mode
		switch (_videoMode)
		{
			case VideoMode::VIDEO_3DSBS:
				cropRight = _width >> 1;
				break;
			case VideoMode::VIDEO_3DTAB:
				cropBottom = _height >> 1;
				break;
			default:
				break;
		}

		// calculate the output size without scaling
		int outputWidth = _width - _cropLeft - cropRight;
		int outputHeight = _height - _cropTop - cropBottom;

		Image<ColorRgb> outputImage = Image<ColorRgb>();
		uint8_t* argb = (uint8_t*)malloc((size_t)outputWidth * (size_t)outputHeight * 4 + 1);
		libyuv::ConvertToARGB(_data, (_pixelFormat == PixelFormat::MJPEG) ? _size : 0, argb, _width * 4, _cropLeft, _cropTop, _width, _height, outputWidth, outputHeight, libyuv::kRotate0, _videoFormatFourCCMap.value(_pixelFormat));

		if (_pixelDecimation > 1)
		{
			int outputWidthScaled = (outputWidth - (_pixelDecimation >> 1) + _pixelDecimation - 1) / _pixelDecimation;
			int outputHeightScaled = (outputHeight - (_pixelDecimation >> 1) + _pixelDecimation - 1) / _pixelDecimation;
			uint8_t* argbScaled = (uint8_t*)malloc((size_t)outputWidthScaled * (size_t)outputHeightScaled * 4 + 1);
			libyuv::ARGBScale(argb, outputWidth * 4, outputWidth, outputHeight, argbScaled, outputWidthScaled * 4, outputWidthScaled, outputHeightScaled, libyuv::kFilterNone);
			outputImage.resize(outputWidthScaled, outputHeightScaled);
			libyuv::ARGBToRAW(argbScaled, outputWidthScaled * 4, reinterpret_cast<uint8_t*>(outputImage.memptr()), outputWidthScaled * 3, outputWidthScaled, outputHeightScaled);
			free(argbScaled);
		}
		else
		{
			outputImage.resize(outputWidth, outputHeight);
			libyuv::ARGBToRAW(argb, outputWidth * 4, reinterpret_cast<uint8_t*>(outputImage.memptr()), outputWidth * 3, outputWidth, outputHeight);
		}

		free(argb);
		emit newFrame(outputImage);
	}

	_busy = false;
}



// 	// calculate the output size
// 	int outputWidth = (width - _cropLeft - cropRight - (_horizontalDecimation >> 1) + _horizontalDecimation - 1) / _horizontalDecimation;
// 	int outputHeight = (height - _cropTop - cropBottom - (_verticalDecimation >> 1) + _verticalDecimation - 1) / _verticalDecimation;

// 	outputImage.resize(outputWidth, outputHeight);

// 	for (int yDest = 0, ySource = _cropTop + (_verticalDecimation >> 1); yDest < outputHeight; ySource += _verticalDecimation, ++yDest)
// 	{
// 		int yOffset = lineLength * ySource;
// 		if (pixelFormat == PixelFormat::NV12)
// 		{
// 			uOffset = (height + ySource / 2) * lineLength;
// 		}
// 		else if (pixelFormat == PixelFormat::I420)
// 		{
// 			uOffset = (lineLength * (5 * height + ySource) / 4);
// 			vOffset = (lineLength * (4 * height + ySource)) * 4;
// 		}

// 		for (int xDest = 0, xSource = _cropLeft + (_horizontalDecimation >> 1); xDest < outputWidth; xSource += _horizontalDecimation, ++xDest)
// 		{
// 			switch (_flipMode)
// 			{
// 				case FlipMode::HORIZONTAL:

// 					xDestFlip = xDest;
// 					yDestFlip = outputHeight-yDest-1;
// 					break;
// 				case FlipMode::VERTICAL:
// 					xDestFlip = outputWidth-xDest-1;
// 					yDestFlip = yDest;
// 					break;
// 				case FlipMode::BOTH:
// 					xDestFlip = outputWidth-xDest-1;
// 					yDestFlip = outputHeight-yDest-1;
// 					break;
// 				case FlipMode::NO_CHANGE:
// 					xDestFlip = xDest;
// 					yDestFlip = yDest;
// 					break;
// 			}

// 			ColorRgb &rgb = outputImage(xDestFlip, yDestFlip);
// 			switch (pixelFormat)
// 			{
// 				case PixelFormat::UYVY:
// 				{
// 					int index = yOffset + (xSource << 1);
// 					uint8_t y = data[index+1];
// 					uint8_t u = ((xSource&1) == 0) ? data[index  ] : data[index-2];
// 					uint8_t v = ((xSource&1) == 0) ? data[index+2] : data[index  ];
// 					ColorSys::yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
// 				}
// 				break;
// 				case PixelFormat::YUYV:
// 				{
// 					int index = yOffset + (xSource << 1);
// 					uint8_t y = data[index];
// 					uint8_t u = ((xSource&1) == 0) ? data[index+1] : data[index-1];
// 					uint8_t v = ((xSource&1) == 0) ? data[index+3] : data[index+1];
// 					ColorSys::yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
// 				}
// 				break;
// 				case PixelFormat::BGR16:
// 				{
// 					int index = yOffset + (xSource << 1);
// 					rgb.blue  = (data[index] & 0x1f) << 3;
// 					rgb.green = (((data[index+1] & 0x7) << 3) | (data[index] & 0xE0) >> 5) << 2;
// 					rgb.red   = (data[index+1] & 0xF8);
// 				}
// 				break;
// 				case PixelFormat::BGR24:
// 				{
// 					int index = yOffset + (xSource << 1) + xSource;
// 					rgb.blue  = data[index  ];
// 					rgb.green = data[index+1];
// 					rgb.red   = data[index+2];
// 				}
// 				break;
// 				case PixelFormat::RGB32:
// 				{
// 					int index = yOffset + (xSource << 2);
// 					rgb.red   = data[index  ];
// 					rgb.green = data[index+1];
// 					rgb.blue  = data[index+2];
// 				}
// 				break;
// 				case PixelFormat::BGR32:
// 				{
// 					int index = yOffset + (xSource << 2);
// 					rgb.blue  = data[index  ];
// 					rgb.green = data[index+1];
// 					rgb.red   = data[index+2];
// 				}
// 				break;
// 				case PixelFormat::NV12:
// 				{
// 					int index = yOffset + xSource;
// 					uint8_t y = data[index];
// 					uint8_t u = data[uOffset + ((xSource >> 1) << 1)];
// 					uint8_t v = data[uOffset + ((xSource >> 1) << 1) + 1];
// 					ColorSys::yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
// 				}
// 				break;
// 				case PixelFormat::I420:
// 				{
// 					int index = yOffset + xSource;
// 					uint8_t y = data[index];
// 					uint8_t u = data[uOffset + xSource];
// 					uint8_t v = data[vOffset + xSource];
// 					ColorSys::yuv2rgb(y, u, v, rgb.red, rgb.green, rgb.blue);
// 					break;
// 				}
// 				break;
// #ifdef HAVE_TURBO_JPEG
// 				case PixelFormat::MJPEG:
// 				break;
// #endif
// 				case PixelFormat::NO_CHANGE:
// 					Error(Logger::getInstance("ImageResampler"), "Invalid pixel format given");
// 				break;
// 			}
// 		}
// 	}

// #endif
