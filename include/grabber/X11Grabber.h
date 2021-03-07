#pragma once
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#include <QCoreApplication>

// QT includes
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


// Hyperion-utils includes
#include <utils/ColorRgb.h>
#include <hyperion/Grabber.h>

// X11 includes
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

class X11Grabber : public Grabber , public QAbstractNativeEventFilter
{
public:

	X11Grabber(int cropLeft, int cropRight, int cropTop, int cropBottom, int pixelDecimation);

	~X11Grabber() override;

	bool open();

	bool Setup();

	///
	/// Captures a single snapshot of the display and writes the data to the given image. The
	/// provided image should have the same dimensions as the configured values (_width and
	/// _height)
	///
	/// @param[out] image  The snapped screenshot (should be initialized with correct width and
	/// height)
	///
	int grabFrame(Image<ColorRgb> & image, bool forceUpdate=false);

	///
	/// update dimension according current screen
	int updateScreenDimensions(bool force=false);

	void setVideoMode(VideoMode mode) override;

	///
	/// @brief Apply new width/height values, overwrite Grabber.h implementation as X11 doesn't use width/height, just pixelDecimation to calc dimensions
	///
	bool setWidthHeight(int width, int height) override { return true; }

	///
	/// @brief Apply new pixelDecimation
	///
	bool setPixelDecimation(int pixelDecimation) override;

	///
	/// Set the crop values
	/// @param  cropLeft    Left pixel crop
	/// @param  cropRight   Right pixel crop
	/// @param  cropTop     Top pixel crop
	/// @param  cropBottom  Bottom pixel crop
	///
	void setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom) override;

	///
	/// @brief Discover X11 screens available (for configuration).
	///
	/// @param[in] params Parameters used to overwrite discovery default behaviour
	///
	/// @return A JSON structure holding a list of devices found
	///
	QJsonObject discover(const QJsonObject& params);

protected:
	bool nativeEventFilter(const QByteArray & eventType, void * message, long int * result) override;

private:


	/// Reference to the X11 display (nullptr if not opened)
	Display* _x11Display;
	Window _window;
	XWindowAttributes _windowAttr;

	bool _XShmAvailable, _XShmPixmapAvailable, _XRenderAvailable,  _XRandRAvailable;

	XImage* _xImage;
	XShmSegmentInfo _shminfo;

	Pixmap _pixmap;
	XRenderPictFormat* _srcFormat;
	XRenderPictFormat* _dstFormat;
	XRenderPictureAttributes _pictAttr;
	Picture _srcPicture;
	Picture _dstPicture;

	int _XRandREventBase;

	XTransform _transform;
	int _pixelDecimation;

	unsigned _calculatedWidth;
	unsigned _calculatedHeight;
	unsigned _src_x;
	unsigned _src_y;

	Image<ColorRgb> _image;

	void freeResources();
	void setupResources();
};
