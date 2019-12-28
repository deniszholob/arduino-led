// ****************** LIBRARY SECTION ************************************* //
#include <FastLED.h> //https://github.com/FastLED/FastLED

// ***************** USER CONFIG SECTION ********************************* //
#define PIN_LED_DATA 6


#define PPG_PIXELS_PER_CLUSTER 15 // 15: Length of spar strips
#define PPG_CLUSTERS_PER_NODE 3   //  3: Front, Back, Side
#define PPG_NODES_PER_SECTION 2   //  2: Left, Right
#define PPG_SECTIONS 3            //  3: Left, Top, Right
#define PPG_NODES (PPG_SECTIONS * PPG_NODES_PER_SECTION)   // sections * 2  = 6
#define PPG_CLUSTERS (PPG_NODES * PPG_CLUSTERS_PER_NODE)   // nodes * 3     = 18
#define PPG_PIXELS (PPG_CLUSTERS * PPG_PIXELS_PER_CLUSTER) // clusters * 15 = 270

#define PPG_LED_MAX_BRIGHTNESS 100 // 0 - 255

// **************************************************************************************************** //
// **************************************  GENERAL VARIABLES  ***************************************** //
// **************************************************************************************************** //

// Cluster is a group of pixels: i.e. led strip on the paramotor arm
struct Cluster {
  bool directionForward;
  bool enabled;
  Cluster() {
    Cluster(true, true);
  }
  Cluster(bool directionForward, bool enabled) {
    this->directionForward = directionForward;
    this->enabled = enabled;
  };
};

struct CRGB ledArray[PPG_PIXELS];
struct Cluster clusterArray[PPG_CLUSTERS];

// Timers
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillis2 = 0;       // will store last time LED was updated


// ==================================================================================================== //
// =========================================  FUNCTIONS: SETUP  ======================================= //
// ==================================================================================================== //

// ARDUINO: the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600); // For Debugging
  Serial.println("=== setup()");
  setupLEDs();
  setupPPGMapping();

  delay(100);
  setupIdentifyLEDStrips();
  delay(1000);
  clearLEDs();

}

// Setup the FAST LED stuff
void setupLEDs() {
  Serial.println("=== setupLEDs()");
  FastLED.addLeds<NEOPIXEL, PIN_LED_DATA>(ledArray, PPG_PIXELS);
  FastLED.setBrightness(PPG_LED_MAX_BRIGHTNESS);
  clearLEDs();
  FastLED.clear();
}

// Set up the led strip mapping for the PPG physical layout
// Nodes are wired up in an alternating direction pattern around the paramotor spars.
void setupPPGMapping() {
  Serial.println("=== setupPPGMapping()");
  for (byte i = 0; i < PPG_CLUSTERS; i++) {
    // Make the middle cluster reversed in each Node
    // (b/c of how its wired vs how it should look)
    // if(i % 3 - 1 == 0) {
    if (i % 2 != 0) {
      clusterArray[i] = Cluster(false, true);
    } else {
      clusterArray[i] = Cluster(true, true);
    }
  }
}

// Display a binary array idx for each led strip on the paramotor
void setupIdentifyLEDStrips() {
  Serial.println("=== setupIdentifyLEDStrips()");
  // Set the first n leds on each strip to show which strip it is
  for (int i = 0; i < PPG_CLUSTERS; i++) {
    // Get the binary representation of the cluster/strip number
    int clusterLEDStates[PPG_PIXELS_PER_CLUSTER] = {0};
    decToBinPixelArray(clusterLEDStates, i + 1);

    // First pixel
    int pixelPosition = clusterPosToGlobalPos(i, 0, !clusterArray[i].directionForward);
    ledArray[pixelPosition] = CRGB::Green;

    // Last pixel
    pixelPosition = clusterPosToGlobalPos(i, PPG_PIXELS_PER_CLUSTER - 1, !clusterArray[i].directionForward);
    ledArray[pixelPosition] = CRGB::Blue;

    // Cluster count in binary
    for (int j = 1; j <= PPG_PIXELS_PER_CLUSTER; j++) {
      pixelPosition = clusterPosToGlobalPos(i, j, clusterArray[i].directionForward);
      if (clusterLEDStates[j - 1] == 1) {
        ledArray[pixelPosition] = CRGB::Red;
      }
    }
  }
  FastLED.show();
}

// ==================================================================================================== //
// ==========================================  FUNCTIONS: LOOP  ======================================= //
// ==================================================================================================== //

// ARDUINO: the loop function runs over and over again forever
void loop() {
  // Serial.println("=== loop()");
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  activateLeds();
}


// Run a pattern depending on the state we are in
void activateLeds() {
  // Serial.println("=== runPattern()");
  // animationStatic();
  animationChaser(true); // True for xmas theme
  // animationAltFlash();
  // animationStarBurst(false);
}


// ---------------------------------------------------------------------------------------------------- //
// ------------------------------------ Pattern Functions: Colors ------------------------------------- //
// ---------------------------------------------------------------------------------------------------- //


/**
 * @brief
 * Color Pattern: Xmas - green on top, candy cane on sides
 * WARNING: Assumes 3 sections!!!
 *
 * @param pos - Position of a pixel in a cluster
 * @param sectionIdx Section that the pixel is in
 * @return CRGB - Color for cluster pixel
 */
CRGB getColorXmas(byte sectionIdx, byte pos) {
  // Serial.println("=== getColorXmas()");
  byte spacing = 4;
  bool isTwinkle = true;

  // Top Section: Green Xmas Tree
  if(sectionIdx == 1) {
    if(isTwinkle && (random(80) == 5)) {
      // return CRGB(180, 255, 140);
      // return CRGB(255, 248, 179);
      // return CRGB(255, 230, 100);
      return CRGB(255, 255, 255);
    }
    // return CRGB(140, 255, 34);
    return CRGB(100, 250, 15);
  }

  // Side sections: Candy cane
  if( altPatternDecider(spacing, pos) == true ) {
    return CRGB(240, 20, 20);
  } else {
    return CRGB(180, 85, 85);
  }
}

CRGB getColorSpyder(byte nodeIdx, byte pos, bool inverse) {
  CRGB spyderBlue = CRGB(35, 50, 80);
  CRGB spyderOrange = CRGB(255, 100, 0);
  byte spacing = 8;

  // Blue on bottom
  if(nodeIdx == 0 || nodeIdx == 5) {
    // return spyderBlue;
    return inverse ? spyderOrange : spyderBlue;
  }

  // Orange in the middle
  if(nodeIdx == 1 || nodeIdx == 4) {
    // return spyderOrange;
    return inverse ? spyderBlue : spyderOrange;
  }

  // Orange/Blue Pattern Top
    if( altPatternDecider(spacing, pos) ^ inverse ) {
      return spyderBlue;
    } else {
      return spyderOrange;
    }
}

CRGB getColorAltPattern(byte pos) {
  byte spacing = 4;

  // === Brandon Woelfel === //
  // CRGB col1 = CRGB(100, 0, 255); // Purple
  // CRGB col2 = CRGB(0, 255, 200); // Cyan

  // === Fireplace === //
  // CRGB col1 = CRGB(200, 0, 0);  // Red
  // CRGB col2 = CRGB(255, 80, 0); // Orange

  // === Evelyn === //
  // CRGB col1 = CRGB(200, 0, 190);
  // CRGB col2 = CRGB(10, 40, 200);

  // === Spyder === //
  CRGB col1 = CRGB(35, 50, 80);  // Blue
  CRGB col2 = CRGB(255, 100, 0); // Orange

  if( altPatternDecider(spacing, pos) ) {
    return col1;
  } else {
    return col2;
  }
}

// ---------------------------------------------------------------------------------------------------- //
// --------------------------------- Pattern Functions: Animations ------------------------------------ //
// ---------------------------------------------------------------------------------------------------- //


void animationStatic() {
  // Serial.println("=== staticColors()");
  for(byte section = 0; section < PPG_SECTIONS; section++) {
    for(byte pixel = 0; pixel < PPG_PIXELS_PER_CLUSTER; pixel++) {
      setSectionPixels(section, pixel, getColorXmas(section, pixel), false);
    }
  }
  FastLED.show();
}

void animationAltFlash() {
  // Serial.println("=== animationSpyder()");

  // Flip Flop
  int animationSpeed = 400;
  static bool inverse = false;
  if (isTime(animationSpeed, previousMillis)) {
    inverse = !inverse;
  }

  for(byte nodeIdx = 0; nodeIdx < PPG_NODES; nodeIdx++) {
    for(byte pixel = 0; pixel < PPG_PIXELS_PER_CLUSTER; pixel++) {
      setNodePixels(nodeIdx, pixel, getColorSpyder(nodeIdx, pixel, inverse), false);
    }
  }
  FastLED.show();
}

void animationChaser(bool xmasSpecial) {
  // Serial.println("=== patternXmas()");
  int animationSpeed = 450 / PPG_PIXELS_PER_CLUSTER * 4;    // 8 msec
  bool inverse = false;

  bool doRandomDirections = true;
  static int direction = 1;
  static int posOffset = 0;

  byte animFrame = 0;
  int rand = random(0, 80);
  while (animFrame < PPG_PIXELS_PER_CLUSTER) {
    // Update only when its time
    if (isTime(animationSpeed, previousMillis)) {

      // Animation Pattern: Shift lights aka running lights
      for (int ledColorIdx = 0; ledColorIdx < PPG_PIXELS_PER_CLUSTER; ledColorIdx++) {
        // Calculate pixel position for current frame
        byte ledPixelIdx = animFrame + ledColorIdx;
        if(doRandomDirections) {
          ledPixelIdx = posOffset + ledColorIdx;
        }
        ledPixelIdx = safePos(ledPixelIdx, 0, PPG_PIXELS_PER_CLUSTER);
        for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
          if(xmasSpecial){
            setSectionPixels(sectionIdx, ledPixelIdx, getColorXmas(sectionIdx, ledColorIdx), inverse);
          } else {
            setSectionPixels(sectionIdx, ledPixelIdx, getColorAltPattern(ledColorIdx), inverse);
          }
        }
      }

      FastLED.show();

      // Next animation frame
      if (doRandomDirections && rand == 1) { direction *= -1; }
      posOffset = safePos(posOffset + direction, 0, PPG_PIXELS_PER_CLUSTER);
      animFrame++;
    }
  }
}

void animationStarBurst(bool inverse) {
  inverse = !inverse;
  CRGB meteorColor = CRGB(100, 0, 255); // Purple
  CRGB meteorColor2 = CRGB(255, 80, 0); // Orange
  byte meteorSize = 1; // Pixels
  // A larger number makes the tail short and/or disappear faster.
  // Theoretically a value of 64 should reduce the brightness by 25% for each time the meteor gets drawn.
  byte meteorTrailDecay = 100;      // msec
  boolean meteorRandomDecay = true;
  byte animationSpeed = 75; // msec

  byte led_start = 0;
  byte led_end = led_start + PPG_PIXELS_PER_CLUSTER;

  byte animFrame = 0;
  int rand = random(0, 80);

  while (animFrame < PPG_PIXELS_PER_CLUSTER) {
    // Update only when its time
    if (isTime(animationSpeed, previousMillis)) {

      // fade brightness all LEDs one step
      // 0 -> 15
      for (int ledPixelIdx = led_start; ledPixelIdx < led_end; ledPixelIdx++) {
        if ( (!meteorRandomDecay) || (random(10) > 5) ) {
          // Activate all the clusters at ones
          fadeAllClusterPixels(ledPixelIdx, inverse, meteorTrailDecay);
        }
      }

      // draw meteor
      // 0 -> 5
      for (int j = 0; j < meteorSize; j++) {
        // Fit the meteor within the bounds (cut off, no wrap)
        byte pos = animFrame-j;
        if ( ( pos < led_end) && (pos >= led_start) ) {
          // Activate all the sections at once
          // setAllSectionPixels(pos, meteorColor2, inverse);
          // Alt Colors
          for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
            bool splitInverse = inverse;
            if(sectionIdx == 1){splitInverse = !splitInverse; }
            setSectionPixels(sectionIdx, pos, getColorAltPattern(pos), splitInverse);
          }
          // Spyder Colors
          // for(byte nodeIdx = 0; nodeIdx < PPG_NODES; nodeIdx++) {
          //   setNodePixels(nodeIdx, pos, getColorSpyder(nodeIdx, pos, inverse), false);
          // }
        }
      }

      FastLED.show();

      // Next animation frame
      animFrame++;
    }
  }
}


// ---------------------------------------------------------------------------------------------------- //
// ======================================== Helper Functions ========================================== //
// ---------------------------------------------------------------------------------------------------- //

/**
 * @brief Checks if the amount of time has passed
 *
 * @param delay - Amount of time to pass to return true
 * @param previousMillis - Updates a new value after true
 * @return true
 * @return false
 */
bool isTime(int delay, unsigned long &previousMillis) {
  unsigned long currentMillis = millis();
  // TODO: handle wrap?
  if (currentMillis - previousMillis > delay) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

// If position is in the alt color zone, returns true
bool altPatternDecider(byte spacing, byte pos) {
  for (byte j = 0; j < spacing; j++) {
    if (pos % (spacing * 2) == j) {
      return true;
    }
  }
  return false;
}

/** Sets a pixel of all the sections */
void setAllSectionPixels(byte pixelIdx, CRGB color, bool inverseDefClusterDir) {
  // For each of the cluster in this node
  for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
    setSectionPixels(sectionIdx, pixelIdx, color, inverseDefClusterDir);
  }
}

/** Sets a pixel of all the nodes in this section */
void setSectionPixels(byte sectionIdx, byte pixelIdx, CRGB color, bool inverseDefClusterDir) {
  byte nodeIdxStart = sectionIdx * PPG_NODES_PER_SECTION;
  byte nodeIdxEnd = nodeIdxStart + PPG_NODES_PER_SECTION;
  // For each of the cluster in this node
  for(byte nodeIdx = nodeIdxStart; nodeIdx < nodeIdxEnd; nodeIdx++) {
    setNodePixels(nodeIdx, pixelIdx, color, inverseDefClusterDir);
  }
}

/** Sets a pixel of all the clusters in this node */
void setNodePixels(byte nodeIdx, byte pixelIdx, CRGB color, bool inverseDefClusterDir) {
  byte clusterIdxStart = nodeIdx * PPG_CLUSTERS_PER_NODE;
  byte clusterIdxEnd = clusterIdxStart + PPG_CLUSTERS_PER_NODE;
  // For each of the cluster in this node
  for(byte clusterIdx = clusterIdxStart; clusterIdx < clusterIdxEnd; clusterIdx++) {
    setClusterPixel(clusterIdx, pixelIdx, color, inverseDefClusterDir);
  }
}

/** Sets a pixel in a cluster a specific color if that cluster is enabled, otherwise its off (black) */
void setClusterPixel(byte clusterIdx, byte pixelIdx, CRGB color, bool inverseDefClusterDir) {
  int pixelGlobalPosition = clusterPosToGlobalPos(clusterIdx, pixelIdx, !clusterArray[clusterIdx].directionForward ^ inverseDefClusterDir);
  if (clusterArray[clusterIdx].enabled) {
    ledArray[pixelGlobalPosition] = color;
  } else {
    ledArray[pixelGlobalPosition] = CRGB(0, 0, 0);
  }
}

// Fade each cluster pixel at the same time
void fadeAllClusterPixels(byte pos, bool inverse, int time) {
  for (int i = 0; i < PPG_CLUSTERS; i++) {
    if (clusterArray[i].enabled) {
      int pixelPosition = clusterPosToGlobalPos(i, pos, !clusterArray[i].directionForward ^ inverse);
      ledArray[pixelPosition].fadeToBlackBy(time);
    }
  }
}

// Wrap pos if outside bounds
int safePos(int pos, int boundLower, int boundUpper) {
  int safePos = pos % (boundUpper - boundLower);
  // C++ modulo keeps the sign instead of wrapping as opposed to wolframalpha
  if (safePos < boundLower) {
    safePos = (boundUpper - boundLower) + safePos;
  }
  return safePos;
}


// 0 - 14
// 15 - 29
// given pos 2 in sec1 => global pos 17
// if reverse => global pos 27
int clusterPosToGlobalPos(byte clusterIdx, byte pos, boolean directionFlip) {
  // Serial.println("=== clusterPosToGlobalPos()");
  if (directionFlip) {
    // 2 * 15 - 2 -1 = 27
    return (clusterIdx + 1) * PPG_PIXELS_PER_CLUSTER - pos - 1;
  }
  // 1 * 15 + 2 = 17
  return clusterIdx * PPG_PIXELS_PER_CLUSTER + pos;
}

// Convert decimal to binary in pixel strip array format
int decToBinPixelArray(int *binaryNum, int n) {
  Serial.println("=== decToBinPixelArray()");
  // counter for binary array
  int i = 0;
  while (n > 0) {
    // storing remainder in binary array
    binaryNum[i] = n % 2;
    n = n / 2;
    i++;
  }
}

// Turn off all the LEDs
void clearLEDs() {
  // Serial.println("=== clearLEDs()");
  for (int i = 0; i < PPG_PIXELS; i++) {
    ledArray[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  // Serial.println("=== setPixel()");
  ledArray[Pixel].r = red;
  ledArray[Pixel].g = green;
  ledArray[Pixel].b = blue;
}

// Prints LED strip info to Serial
void printArray(CRGB *array, int len) {
  // Serial.println("=== printArray()");
  String strip = String("[ ");
  for (int j = 0; j < len; j++) {
    strip = String(strip + array[j] + ", ");
  }
  strip = String(strip + "]");
  Serial.println(strip);
}

// ---------------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------------- //
