#pragma once

// Windows include
#include <Windows.h>

// COM includes
#include <Guiddef.h>

// Qt includes
#include <QObject>
#include <QRectF>
#include <QMap>
#include <QMultiMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// utils includes
#include <utils/PixelFormat.h>
#include <utils/Components.h>
#include <hyperion/Grabber.h>

// decoder thread includes
#include <grabber/MFThread.h>

// TurboJPEG decoder
#ifdef HAVE_TURBO_JPEG
	#include <turbojpeg.h>
#endif

/// Forward class declaration
class SourceReaderCB;
/// Forward struct declaration
struct IMFSourceReader;

///
/// Media Foundation capture class
///

class MFGrabber : public Grabber
{
	Q_OBJECT
	friend class SourceReaderCB;

public:
	struct DeviceProperties
	{
		QString symlink = QString();
		int	width		= 0;
		int	height		= 0;
		int	fps			= 0;
		int numerator	= 0;
		int denominator = 0;
		PixelFormat pf	= PixelFormat::NO_CHANGE;
		GUID guid		= GUID_NULL;
	};

	MFGrabber();
	~MFGrabber() override;

	void receive_image(const void *frameImageBuffer, int size);
	QRectF getSignalDetectionOffset() const { return QRectF(_x_frac_min, _y_frac_min, _x_frac_max, _y_frac_max); }
	bool getSignalDetectionEnabled() const { return _signalDetectionEnabled; }
	void setSignalThreshold(double redSignalThreshold, double greenSignalThreshold, double blueSignalThreshold, int noSignalCounterThreshold) override;
	void setSignalDetectionOffset( double verticalMin, double horizontalMin, double verticalMax, double horizontalMax) override;
	void setSignalDetectionEnable(bool enable) override;
	bool setDevice(QString device) override;
	bool setWidthHeight(int width, int height) override;
	bool setEncoding(QString enc);
	bool setBrightnessContrastSaturationHue(int brightness, int contrast, int saturation, int hue);
	void reloadGrabber();

	///
	/// @brief Discover available Media Foundation USB devices (for configuration).
	/// @param[in] params Parameters used to overwrite discovery default behaviour
	/// @return A JSON structure holding a list of USB devices found
	///
	QJsonArray discover(const QJsonObject& params);

public slots:
	bool prepare();
	bool start();
	void stop();
	void newThreadFrame(unsigned int _workerIndex, const Image<ColorRgb>& image,unsigned int sourceCount);

signals:
	void newFrame(const Image<ColorRgb> & image);
	void readError(const char* err);

private:
	bool init();
	void uninit();
	HRESULT init_device(QString device, DeviceProperties props);
	void uninit_device();
	void enumVideoCaptureDevices();
	void start_capturing();
	void process_image(const void *frameImageBuffer, int size);
	void checkSignalDetectionEnabled(Image<ColorRgb> image);

	QString										_currentDeviceName, _newDeviceName;
	QMap<QString, QList<DeviceProperties>>		_deviceProperties;
	HRESULT										_hr;
	SourceReaderCB*								_sourceReaderCB;
	PixelFormat									_pixelFormat, _pixelFormatConfig;
	int											_lineLength, _frameByteSize,
												_noSignalCounterThreshold, _noSignalCounter,
												_brightness, _contrast, _saturation, _hue;
	volatile unsigned int						_currentFrame;
	ColorRgb									_noSignalThresholdColor;
	bool										_signalDetectionEnabled,
												_noSignalDetected,
												_initialized;
	double										_x_frac_min,
												_y_frac_min,
												_x_frac_max,
												_y_frac_max;
	MFThreadManager								_threadManager;
	IMFSourceReader*							_sourceReader;

#ifdef HAVE_TURBO_JPEG
	int											_subsamp;
#endif
};
