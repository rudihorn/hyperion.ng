#pragma once

#include <QObject>

// Hyperion-utils includes
#include <utils/ColorRgb.h>
#include <hyperion/Grabber.h>

class QScreen;

///
/// @brief The platform capture implementation based on QT API
///
class QtGrabber : public Grabber
{
public:

	QtGrabber(int cropLeft=0, int cropRight=0, int cropTop=0, int cropBottom=0, int pixelDecimation=8, int display=0);

	~QtGrabber() override;

	///
	/// Captures a single snapshot of the display and writes the data to the given image. The
	/// provided image should have the same dimensions as the configured values (_width and
	/// _height)
	///
	/// @param[out] image  The snapped screenshot (should be initialized with correct width and
	/// height)
	///
	int grabFrame(Image<ColorRgb> & image);

	///
	/// @brief Set a new video mode
	///
	void setVideoMode(VideoMode mode) override;

	///
	/// @brief Apply new width/height values, overwrite Grabber.h implementation as qt doesn't use width/height, just pixelDecimation to calc dimensions
	///
	bool setWidthHeight(int width, int height) override { return true; }

	///
	/// @brief Apply new pixelDecimation
	///
	bool setPixelDecimation(unsigned pixelDecimation) override;

	///
	/// Set the crop values
	/// @param  cropLeft    Left pixel crop
	/// @param  cropRight   Right pixel crop
	/// @param  cropTop     Top pixel crop
	/// @param  cropBottom  Bottom pixel crop
	///
	void setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom) override;

	///
	/// @brief Apply display index
	///
	void setDisplayIndex(int index) override;

	///
	/// @brief Discover QT screens available (for configuration).
	///
	/// @param[in] params Parameters used to overwrite discovery default behaviour
	///
	/// @return A JSON structure holding a list of devices found
	///
	QJsonObject discover(const QJsonObject& params);

	///
	/// @brief Opens the input device.
	///
	/// @return Zero, on success (i.e. device is ready), else negative
	///
	bool open();

private slots:
	///
	/// @brief is called whenever the current _screen changes it's geometry
	/// @param geo   The new geometry
	///
	void geometryChanged(const QRect &geo);

private:
	///
	/// @brief Setup a new capture display, will free the previous one
	/// @return True on success, false if no display is found
	///
	bool setupDisplay();

	///
	/// @brief Is called whenever we need new screen dimension calculations based on window geometry
	///
	int updateScreenDimensions(bool force);

	///
	/// @brief free the _screen pointer
	///
	void freeResources();

private:

	int _display;
	int _numberOfSDisplays;

	int _pixelDecimation;
	int _calculatedWidth;
	int _calculatedHeight;
	int _src_x;
	int _src_y;
	int _src_x_max;
	int _src_y_max;
	QScreen* _screen;

	bool _isVirtual;
};
