#ifndef TideDisplay_h
#define TideDisplay_h

#include <neopixel.h> // from https://github.com/technobly/Particle-NeoPixel
#include <vector>

struct Color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    
    Color (uint8_t red, uint8_t green,uint8_t blue) :
        red(red), green(green), blue(blue) { }
    
    Color () :
        red(0), green(0), blue(0) { }
    
    bool operator==(const Color &other) const;
    bool operator!=(const Color &other) const;
};

class TideDisplay
{
    public:
        // Constructor.
        TideDisplay(unsigned numPixels, unsigned pin, double physicalHeight);
        // Readies the display for input.
        void start();
        // Cause a bubble to run up/down the display, depending on value of rising.
        void bubble();
        // Update the tide height displayed, height in inches.
        void heightUpdate(double height, double nextHeight, bool rising);
        // Display the status of the last read at the top of the display.
        void showStatus(bool success);
        // Returns a Color that is the result of the linear gradient between the two colors.
        // Progress must be a double in the interval [0, 1].
        Color gradient(Color fromColor, Color toColor, double progress);
        double pixelsPerInch;
        
        double pixelToInch(unsigned pixel);
        unsigned inchToPixel(double inch);
        
        int lowThreshold;
        int warnThreshold;
    private:
        int _numPixels; // the number of pixels on the strip
        double _height; // the height of the display, inches
        
        int _pixelsInUse; // the number of pixels currently in use
        int _lastPredPixel; // the last height prediction in pixels
        
        os_thread_return_t _update();
        Thread* updateThread;
        
        std::vector<Color> _indicators;
        double _indicatorIntensity;
        bool _indicatorIntensityRising;
        
        void _bubbleAdvance();
        bool _bubbleRising;
        int _bubblePosition;
        
        void _setBrightness(unsigned led, double brightness);
        Color _colorAt(unsigned pixel);
        
        Adafruit_NeoPixel _pixelStrip;
        Timer _bubbleTimer;
};

#endif
