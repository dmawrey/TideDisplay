#include "TideDisplay.h"

bool Color::operator==(const Color &other) const {
    return red == other.red && green == other.green && blue == other.blue;
}

bool Color::operator!=(const Color &other) const {
    return !(*this == other);
}

TideDisplay::TideDisplay(unsigned numPixels, unsigned pin, double physicalHeight) : 
    _pixelStrip(numPixels, pin),
    _bubbleTimer(25, &TideDisplay::_bubbleAdvance, *this),
    _height(physicalHeight),
    _numPixels(numPixels),
    _indicators(numPixels),
    pixelsPerInch(numPixels / physicalHeight),
    lowThreshold(inchToPixel(15)), warnThreshold(inchToPixel(20)) { }

void TideDisplay::start()
{
    _pixelStrip.begin();
    _pixelStrip.show();
    
    updateThread = new Thread("update", [this]() { _update(); });
}

void TideDisplay::bubble()
{
    if (!_bubbleTimer.isActive()) {
        _bubblePosition = _bubbleRising ? 0 : _pixelsInUse;
        _bubbleTimer.start();
    }
}

void TideDisplay::heightUpdate(double height, double nextHeight, bool rising)
{
    _pixelsInUse = inchToPixel(height);
    _bubbleRising = rising;
    
    // Clear old prediction
    _indicators[_lastPredPixel] = Color(0, 0, 0);
    
    /*
    // Bottom 'status' LED
    if (_pixelsInUse < lowThreshold) {
        _indicators[0] = Color(255, 0, 0);
    } else if (_pixelsInUse < warnThreshold) {
        _indicators[0] = Color(255, 255, 0);
    } else {
        _indicators[0] = Color(0, 255, 0);
    }*/
    
    // Section divider LED's
    if (_pixelsInUse > lowThreshold) {
        _indicators[lowThreshold] = Color(192, 192, 192);
    } else {
        _pixelStrip.setColor(lowThreshold, 0, 0, 0);
        _indicators[lowThreshold] = Color(0, 0, 0);
    }
    if (_pixelsInUse > warnThreshold) {
        _indicators[warnThreshold] = Color(192, 192, 192);
    } else {
        _pixelStrip.setColor(warnThreshold, 0, 0, 0);
        _indicators[warnThreshold] = Color(0, 0, 0);
    }
    
    // Prediction LED
    int nextPredPixel = inchToPixel(nextHeight);
    if (nextPredPixel < _numPixels) {
        _indicators[nextPredPixel] = gradient(Color(255, 0, 0), Color(0, 255, 0), 2 * ((double)nextPredPixel / _numPixels));
        _lastPredPixel = nextPredPixel;
    }
}

void TideDisplay::showStatus(bool success)
{
    if (success) {
        _indicators[_numPixels - 1] = Color(0, 255, 0);
    } else {
        _indicators[_numPixels - 1] = Color(255, 0, 0);
    }
    
    delay(500);
    
    _pixelStrip.setColor(_numPixels - 1, 0, 0, 0);
    _indicators[_numPixels - 1] = Color(0, 0, 0);
}

double TideDisplay::pixelToInch(unsigned pixel)
{
    return pixel / pixelsPerInch;
}

unsigned TideDisplay::inchToPixel(double inch)
{
    return inch * pixelsPerInch;
}

os_thread_return_t TideDisplay::_update()
{
    while (true) {
        for (int i = 0; i < _indicators.size(); i++) {
            Color indicatorColor = _indicators[i];
            
            if (indicatorColor != Color()) {
                Color gradientColor = gradient(indicatorColor, _colorAt(i), _indicatorIntensity);
                
                _pixelStrip.setColorDimmed(i, gradientColor.red, gradientColor.green, gradientColor.blue, (int)(0.75 * 255));
            }
        }
        
        _pixelStrip.show();
        
        _indicatorIntensity += 0.02 * (_indicatorIntensityRising ? 1 : -1);
        
        if (_indicatorIntensity < 0) {
            _indicatorIntensity = 0;
            _indicatorIntensityRising = true;
        } else if (_indicatorIntensity > 1) {
            _indicatorIntensity = 1;
            _indicatorIntensityRising = false;
        }
        
        delay(25);
    }
}

void TideDisplay::_bubbleAdvance()
{
    if (_bubblePosition - 2 > 0) {
        _setBrightness(_bubblePosition - 2, 0.55);
    }
    if (_bubblePosition - 1 > 0) {
        _setBrightness(_bubblePosition - 1, 0.60);
    }
    
    _setBrightness(_bubblePosition, 0.75);
    
    if (_bubblePosition + 1 < _pixelsInUse) {
        _setBrightness(_bubblePosition + 1, 0.60);
    }
    if (_bubblePosition + 2 < _pixelsInUse) {
        _setBrightness(_bubblePosition + 2, 0.55);
    }
    
    if (_bubbleRising)
        _bubblePosition++;
    else
        _bubblePosition--;
        
    if (_bubblePosition < -2 || _bubblePosition >= _pixelsInUse + 2) {
        _bubbleTimer.stop();
        
        for (int i = _pixelsInUse; i < _numPixels; i++) {
            _pixelStrip.setColor(i, 0, 0, 0);
        }
    }
}

void TideDisplay::_setBrightness(unsigned led, double brightness)
{
    Color color = _colorAt(led);
    
    _pixelStrip.setColorDimmed(led, color.red, color.green, color.blue, (int)(brightness * 255));
}

Color TideDisplay::_colorAt(unsigned pixel)
{
    if (pixel >= _pixelsInUse) {
        return Color(0, 0, 0);
    } else {
        return gradient(Color(0, 64, 128), Color(154, 154, 154), (double)pixel / _numPixels);
    }
}

Color TideDisplay::gradient(Color fromColor, Color toColor, double progress)
{
    if (progress > 1)
        progress = 1;
    if (progress < 0)
        progress = 0;
    
    Color result = Color(0, 0, 0);
    result.red = (fromColor.red * (1 - progress)) + (toColor.red * progress);
    result.green = (fromColor.green * (1 - progress)) + (toColor.green * progress);
    result.blue = (fromColor.blue * (1 - progress)) + (toColor.blue * progress);

    return result;
}