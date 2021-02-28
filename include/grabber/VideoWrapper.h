#pragma once

#include <HyperionConfig.h> // Required to determine the cmake options
#include <hyperion/GrabberWrapper.h>

#if defined(ENABLE_MF)
	#include <grabber/MFGrabber.h>
#elif defined(ENABLE_V4L2)
	#include <grabber/V4L2Grabber.h>
#endif

#if defined(ENABLE_CEC)
	#include <cec/CECEvent.h>
#endif

class VideoWrapper : public GrabberWrapper
{
	Q_OBJECT

public:
	VideoWrapper();
	~VideoWrapper() override;

	bool getSignalDetectionEnable() const;
	bool getCecDetectionEnable() const;

public slots:
	bool start() override;
	void stop() override;

	// Device
	bool setDevice(const QString& device);

	// Device input
	bool setInput(int input);

	// Device resolution
	bool setWidthHeight(int width, int height);

	// Device framerate
	bool setFramerate(int fps);

	// Device encoding format
	bool setEncoding(QString enc);

	// Video standard
	void setVideoStandard(VideoStandard videoStandard);

	// Image size decimation
	void setPixelDecimation(unsigned pixelDecimation);

	// Image cropping
	void setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom) override;

	// Software frame decimation
	void setFpsSoftwareDecimation(unsigned decimation);

	// Signal detection
	void setSignalDetectionEnable(bool enable);
	void setSignalDetectionOffset(double verticalMin, double horizontalMin, double verticalMax, double horizontalMax);
	void setSignalThreshold(double redSignalThreshold, double greenSignalThreshold, double blueSignalThreshold, int noSignalCounterThreshold);

	// CEC Standby
	void setCecDetectionEnable(bool enable);

#if defined(ENABLE_CEC)
	void handleCecEvent(CECEvent event);
#endif

	// Brightness, Contrast, Saturation, Hue
	bool setBrightnessContrastSaturationHue(int brightness, int contrast, int saturation, int hue);

	void handleSettingsUpdate(settings::type type, const QJsonDocument& config) override;

private slots:
	void newFrame(const Image<ColorRgb> & image);
	void readError(const char* err);

	void action() override;

private:
	/// The Media Foundation or V4L2 grabber
#if defined(ENABLE_MF)
	MFGrabber _grabber;
#elif defined(ENABLE_V4L2)
	V4L2Grabber _grabber;
#endif
};
