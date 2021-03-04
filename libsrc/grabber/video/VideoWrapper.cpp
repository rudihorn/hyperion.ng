#include <QMetaType>

#include <grabber/VideoWrapper.h>

// qt
#include <QTimer>

VideoWrapper::VideoWrapper()
	: GrabberWrapper("V4L2", &_grabber, 0, 0, 10)
	, _grabber()
{
	_ggrabber = &_grabber;

	// register the image type
	qRegisterMetaType<Image<ColorRgb>>("Image<ColorRgb>");

	// Handle the image in the captured thread (Media Foundation/V4L2) using a direct connection
	connect(&_grabber, SIGNAL(newFrame(const Image<ColorRgb>&)), this, SLOT(newFrame(const Image<ColorRgb>&)), Qt::DirectConnection);
	connect(&_grabber, SIGNAL(readError(const char*)), this, SLOT(readError(const char*)), Qt::DirectConnection);
}

VideoWrapper::~VideoWrapper()
{
	stop();
}

bool VideoWrapper::getSignalDetectionEnable() const
{
	return _grabber.getSignalDetectionEnabled();
}

bool VideoWrapper::start()
{
	Debug(_log, "");
	return (_grabber.prepare() && _grabber.start() && GrabberWrapper::start());
}

void VideoWrapper::stop()
{
	_grabber.stop();
	GrabberWrapper::stop();
}

bool VideoWrapper::setDevice(const QString& device)
{
	return _grabber.setDevice(device);
}

bool VideoWrapper::setInput(int input)
{
	return _grabber.setInput(input);
}

bool VideoWrapper::setWidthHeight(int width, int height)
{
	return _grabber.setWidthHeight(width, height);
}

bool VideoWrapper::setFramerate(int fps)
{
	return _grabber.setFramerate(fps);
}

bool VideoWrapper::setEncoding(QString enc)
{
	return _grabber.setEncoding(enc);
}

void VideoWrapper::setVideoStandard(VideoStandard videoStandard)
{
	_grabber.setVideoStandard(videoStandard);
}

void VideoWrapper::setPixelDecimation(unsigned pixelDecimation)
{
	_grabber.setPixelDecimation(pixelDecimation);
}

void VideoWrapper::setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom)
{
	_grabber.setCropping(cropLeft, cropRight, cropTop, cropBottom);
}

void VideoWrapper::setFpsSoftwareDecimation(unsigned decimation)
{
	_grabber.setFpsSoftwareDecimation(decimation);
}

void VideoWrapper::setSignalDetectionEnable(bool enable)
{
	_grabber.setSignalDetectionEnable(enable);
}

void VideoWrapper::setSignalDetectionOffset(double verticalMin, double horizontalMin, double verticalMax, double horizontalMax)
{
	_grabber.setSignalDetectionOffset(verticalMin, horizontalMin, verticalMax, horizontalMax);
}

void VideoWrapper::setSignalThreshold(double redSignalThreshold, double greenSignalThreshold, double blueSignalThreshold, int noSignalCounterThreshold)
{
	_grabber.setSignalThreshold( redSignalThreshold, greenSignalThreshold, blueSignalThreshold, noSignalCounterThreshold);
}

void VideoWrapper::setCecDetectionEnable(bool enable)
{
	_grabber.setCecDetectionEnable(enable);
}

bool VideoWrapper::setBrightnessContrastSaturationHue(int brightness, int contrast, int saturation, int hue)
{
	return _grabber.setBrightnessContrastSaturationHue(brightness, contrast, saturation, hue);
}

#if defined(ENABLE_CEC) && !defined(ENABLE_MF)

void VideoWrapper::handleCecEvent(CECEvent event)
{
	_grabber.handleCecEvent(event);
}

#endif

void VideoWrapper::handleSettingsUpdate(settings::type type, const QJsonDocument& config)
{
	if(type == settings::V4L2 && _grabberName.startsWith("V4L2"))
	{
		// extract settings
		const QJsonObject& obj = config.object();
		// reload state
		bool reload = false;

		// Device
		if (_grabber.setDevice(obj["device"].toString("auto")))
			reload = true;

		// Device input
		if (_grabber.setInput(obj["input"].toInt(0)))
			reload = true;

		// Device resolution
		if (_grabber.setWidthHeight(obj["width"].toInt(0), obj["height"].toInt(0)))
			reload = true;

		// Device framerate
		if (_grabber.setFramerate(obj["fps"].toInt(15)))
			reload = true;

		// Device encoding format
		if (_grabber.setEncoding(obj["encoding"].toString("NO_CHANGE")))
			reload = true;

		// Video standard
		_grabber.setVideoStandard(parseVideoStandard(obj["standard"].toString("NO_CHANGE")));

		// Image size decimation
		_grabber.setPixelDecimation(obj["sizeDecimation"].toInt(8));

		// Flip mode
		_grabber.setFlipMode(parseFlipMode(obj["flip"].toString("NO_CHANGE")));

		// Image cropping
		_grabber.setCropping(
			obj["cropLeft"].toInt(0),
			obj["cropRight"].toInt(0),
			obj["cropTop"].toInt(0),
			obj["cropBottom"].toInt(0));

		// Brightness, Contrast, Saturation, Hue
		if (_grabber.setBrightnessContrastSaturationHue(obj["hardware_brightness"].toInt(0), obj["hardware_contrast"].toInt(0), obj["hardware_saturation"].toInt(0), obj["hardware_hue"].toInt(0)))
			reload = true;

		// CEC Standby
		_grabber.setCecDetectionEnable(obj["cecDetection"].toBool(true));

		// Software frame skipping
		_grabber.setFpsSoftwareDecimation(obj["fpsSoftwareDecimation"].toInt(1));

		// Signal detection
		_grabber.setSignalDetectionEnable(obj["signalDetection"].toBool(true));
		_grabber.setSignalDetectionOffset(
			obj["sDHOffsetMin"].toDouble(0.25),
			obj["sDVOffsetMin"].toDouble(0.25),
			obj["sDHOffsetMax"].toDouble(0.75),
			obj["sDVOffsetMax"].toDouble(0.75));
		_grabber.setSignalThreshold(
			obj["redSignalThreshold"].toDouble(0.0)/100.0,
			obj["greenSignalThreshold"].toDouble(0.0)/100.0,
			obj["blueSignalThreshold"].toDouble(0.0)/100.0,
			obj["noSignalCounterThreshold"].toInt(50) );

		// Reload the Grabber if any settings have been changed that require it
		if (reload)
			_grabber.reloadGrabber();
	}
}

void VideoWrapper::newFrame(const Image<ColorRgb> &image)
{
	emit systemImage(_grabberName, image);
}

void VideoWrapper::readError(const char* err)
{
	Error(_log, "Stop grabber, because reading device failed. (%s)", err);
	stop();
}

void VideoWrapper::action()
{
	// dummy as v4l get notifications from stream
}
