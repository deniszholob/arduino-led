// ****************** LIBRARY SECTION ************************************* //
#include <Arduino.h>
#include <FastLED.h> //https://github.com/FastLED/FastLED

// ***************** USER CONFIG SECTION ********************************* //
// Max Pixels: Test Strip: 96
// Max Pixels: PPG Strip: 270
#define PIN_LED_DATA 6

// To use as a single strip;
// set PPG_SECTIONS, PPG_NODES_PER_SECTION, PPG_CLUSTERS_PER_NODE to 1 and
// set PPG_PIXELS_PER_CLUSTER to max pixels in the strip
// Use the defaults for air conception paramotor like spars
// Experiment with values otherwise :p
#define PPG_PIXELS_PER_CLUSTER 15 // 15: Length of spar strips
#define PPG_CLUSTERS_PER_NODE 3   //  3: Front, Back, Side of the spar
#define PPG_NODES_PER_SECTION 2   //  2: Left, Right spars (cw) (Note, some algs need 6 total nodes)
#define PPG_SECTIONS 3            //  3: Left, Top, Right (Note, some algs assume 3 sections)
#define PPG_NODES (PPG_SECTIONS * PPG_NODES_PER_SECTION)   // sections * 2  = 6
#define PPG_CLUSTERS (PPG_NODES * PPG_CLUSTERS_PER_NODE)   // nodes * 3     = 18
#define PPG_PIXELS (PPG_CLUSTERS * PPG_PIXELS_PER_CLUSTER) // clusters * 15 = 270

#define PPG_LED_MAX_BRIGHTNESS 100 // 100: 0 - 255
#define USE_TEST_LEDS false        // true: Test LEDs are different type than the Paramotor so colors are wrong
#define FLIP_COLORS false          // false: Flips primary/secondary color

// Chose a preset light animation
enum AnimationPatterns {
  A_STATIC,  // No animation, just shows a steady light pattern
  A_CHASER,  // Shift lights aka running lights Becomes STATIC in P_ALT_NODE mode
  A_FLASH,   // Flip Flop b/w colors
  A_IMPLODE, // Meteor inward crush
  A_EXPLODE  // (Default) Meteor outward burst
};
// Chose a preset color pattern (some expect specific pixel arrangements)
enum AnimationColorPatterns {
  P_XMAS,       // (Needs 3 sections) Candy canes with a sparkling xmas tree in the center
  P_ALT_NODE,   // (Needs 6 effective nodes total) Becomes STATIC in CHASER mode
  P_ALT_CLUSTER // (Default) Uses alternating colors
};
// Choose a binary color preset to be used in ALT modes
enum AltColorPresets {
  C_BF,        // Purple, Cyan
  C_EVE,       // Pink, Blue
  C_RETRO,     // Purple, Cyan
  C_FIRE,      // Orange, Red
  C_PUMPKIN,   // Orange, Green
  C_PUMPKIN2,  // Orange, Yellow
  C_HALLOWEEN, // Orange, Purple
  C_JOKER,     // Purple, Green
  C_POLICE,    // Red, Blue
  C_SPYDER     // (Default) Orange, Blue
};
AnimationPatterns useAnimationPattern = AnimationPatterns::A_EXPLODE;
AnimationColorPatterns useAnimationColorPattern = AnimationColorPatterns::P_ALT_CLUSTER;
AltColorPresets useAltColorPreset = AltColorPresets::C_SPYDER;

// **************************************************************************************************** //
// **************************************  GENERAL VARIABLES  ***************************************** //
// **************************************************************************************************** //

struct BiColor {
  CRGB col1;
  CRGB col2;
};

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
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long previousMillis2 = 0; // will store last time LED was updated


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
  if(USE_TEST_LEDS) {
    FastLED.addLeds<TM1809, PIN_LED_DATA>(ledArray, PPG_PIXELS);   // Test Lights
  } else {
    FastLED.addLeds<NEOPIXEL, PIN_LED_DATA>(ledArray, PPG_PIXELS); // Paramotor
  }
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
  switch (useAnimationPattern) {
    case AnimationPatterns::A_STATIC: {
      animationStatic();
      break;
    }
    case AnimationPatterns::A_CHASER: {
      animationChaser();
      break;
    }
    case AnimationPatterns::A_FLASH: {
      animationAltFlash();
      break;
    }
    case AnimationPatterns::A_IMPLODE: {
      animationStarBurst(true);
      break;
    }
    case AnimationPatterns::A_EXPLODE: {
      animationStarBurst(false);
      break;
    }
    default: {
      animationStarBurst(false);
      break;
    }
  }
}


// ---------------------------------------------------------------------------------------------------- //
// ------------------------------------ Pattern Functions: Colors ------------------------------------- //
// ---------------------------------------------------------------------------------------------------- //


/**
 * Special Color Pattern: Xmas - green on top, candy cane on sides
 * WARNING: Assumes 3 sections!!!
 *
 * @param sectionIdx Section that the pixel is in
 * @param pos - Position of a pixel in a cluster
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

/**
 * Special Color Pattern: Alternating 6Node - 2 color pattern within 6 nodes
 * WARNING: Assumes 6 nodes total!!!
 *
 * @param nodeIdx - Node that the pixel is in
 * @param pos - Position of a pixel in a cluster
 * @return CRGB - Color for cluster pixel
 */
CRGB getColorAltNodePattern(byte nodeIdx, byte pos, bool inverse) {
  byte spacing = 8; // 8
  BiColor col = getBiColor();

  // Blue on bottom if spyder
  if(nodeIdx == 0 || nodeIdx == 5) {
    return inverse ? col.col2 : col.col1;
  }

  // Orange in the middle if spyder
  if(nodeIdx == 1 || nodeIdx == 4) {
    return inverse ? col.col1 : col.col2;
  }

  // Orange/Blue Pattern Top if spyder
  // Using XOR operator to flip colors
  if( altPatternDecider(spacing, pos) ^ inverse ) {
    return col.col1;
  } else {
    return col.col2;
  }
}

/**
 * Generic Color Pattern: Alternating - 2 color pattern within a cluster
 *
 * @param pos - Position of a pixel in a cluster
 * @return CRGB - Color for cluster pixel
 */
CRGB getColorAltClusterPattern(byte pos, bool inverse) {
  byte spacing = 4; // 4
  BiColor col = getBiColor();

  // Using XOR operator to flip colors
  if( altPatternDecider(spacing, pos) ^ inverse ) {
    return col.col1;
  } else {
    return col.col2;
  }
}

// ---------------------------------------------------------------------------------------------------- //
// --------------------------------- Pattern Functions: Animations ------------------------------------ //
// ---------------------------------------------------------------------------------------------------- //


// Static colors
void animationStatic() {
  // Serial.println("=== staticColors()");
  for(byte pixel = 0; pixel < PPG_PIXELS_PER_CLUSTER; pixel++) {
    switch (useAnimationColorPattern) {
      case AnimationColorPatterns::P_XMAS: {
        for(byte section = 0; section < PPG_SECTIONS; section++) {
          setSectionPixels(section, pixel, getColorXmas(section, pixel), FLIP_COLORS);
        }
        break;
      }
      case AnimationColorPatterns::P_ALT_NODE: {
        for(byte node = 0; node < PPG_NODES; node++) {
          setNodePixels(node, pixel, getColorAltNodePattern(node, pixel, FLIP_COLORS), false);
        }
        break;
      }
      // case AnimationColorPatterns::P_ALT_CLUSTER: {
      default: {
        for(byte section = 0; section < PPG_SECTIONS; section++) {
          setSectionPixels(section, pixel, getColorAltClusterPattern(pixel, FLIP_COLORS), false);
        }
        break;
      }
    }
  }
  FastLED.show();
}

// Flip Flop the colors
void animationAltFlash() {
  // Serial.println("=== animationAltFlash()");
  int animationSpeed = 400; // 400
  static bool inverse = false;
  if (isTime(animationSpeed, previousMillis)) {
    inverse = !inverse;
  }

  for(byte pixel = 0; pixel < PPG_PIXELS_PER_CLUSTER; pixel++) {
    switch (useAnimationColorPattern) {
      case AnimationColorPatterns::P_XMAS: {
        for(byte section = 0; section < PPG_SECTIONS; section++) {
          setSectionPixels(section, pixel, getColorXmas(section, pixel), inverse);
        }
        break;
      }
      case AnimationColorPatterns::P_ALT_NODE: {
        for(byte node = 0; node < PPG_NODES; node++) {
          setNodePixels(node, pixel, getColorAltNodePattern(node, pixel, inverse), false);
        }
        break;
      }
      // case AnimationColorPatterns::P_ALT_CLUSTER: {
      default: {
        for(byte section = 0; section < PPG_SECTIONS; section++) {
          setSectionPixels(section, pixel, getColorAltClusterPattern(pixel, inverse), false);
        }
        break;
      }
    }
  }
  FastLED.show();
}

// Chaser
void animationChaser() {
  // Serial.println("=== animationChaser()");
  int animationSpeed = 40; // 450 / PPG_PIXELS_PER_CLUSTER * 4;    // 8 msec

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
        byte pixel = animFrame + ledColorIdx;
        if(doRandomDirections) {
          pixel = posOffset + ledColorIdx;
        }
        pixel = safePos(pixel, 0, PPG_PIXELS_PER_CLUSTER);

        switch (useAnimationColorPattern) {
          case AnimationColorPatterns::P_XMAS: {
            for(byte section = 0; section < PPG_SECTIONS; section++) {
              setSectionPixels(section, pixel, getColorXmas(section, ledColorIdx), FLIP_COLORS);
            }
            break;
          }
          case AnimationColorPatterns::P_ALT_NODE: {
            for(byte node = 0; node < PPG_NODES; node++) {
              setNodePixels(node, pixel, getColorAltNodePattern(node, ledColorIdx, FLIP_COLORS), false);
            }
            break;
          }
          // case AnimationColorPatterns::P_ALT_CLUSTER: {
          default: {
            for(byte section = 0; section < PPG_SECTIONS; section++) {
              setSectionPixels(section, pixel, getColorAltClusterPattern(ledColorIdx, false), FLIP_COLORS);
            }
            break;
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

// Meteor Burst/Collapse(if inverse)
void animationStarBurst(bool inverse) {
  // Serial.println("=== animationStarBurst()");
  inverse = !inverse;
  CRGB meteorColor = CRGB(100, 0, 255); // Purple
  CRGB meteorColor2 = CRGB(255, 80, 0); // Orange
  byte meteorSize = 1; // Pixels
  // A larger number makes the tail short and/or disappear faster.
  // Theoretically a value of 64 should reduce the brightness by 25% for each time the meteor gets drawn.
  byte meteorTrailDecay = 100; // 100 msec
  boolean meteorRandomDecay = true;
  byte animationSpeed = 75; // 75 msec

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

          switch (useAnimationColorPattern) {
            case AnimationColorPatterns::P_XMAS: {
              for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
                bool splitInverse = inverse;
                if(sectionIdx == 1){splitInverse = !splitInverse; }
                setSectionPixels(sectionIdx, pos, getColorXmas(sectionIdx, pos), splitInverse);
              }
              break;
            }
            case AnimationColorPatterns::P_ALT_NODE: {
              for(byte nodeIdx = 0; nodeIdx < PPG_NODES; nodeIdx++) {
                setNodePixels(nodeIdx, pos, getColorAltNodePattern(nodeIdx, pos, inverse), false);
              }
              break;
            }
            // case AnimationColorPatterns::P_ALT_CLUSTER: {
            default: {
              for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
                bool splitInverse = inverse;
                if(sectionIdx == 1){splitInverse = !splitInverse; }
                setSectionPixels(sectionIdx, pos, getColorAltClusterPattern(pos, FLIP_COLORS), splitInverse);
              }
              break;
            }
          }

          // Activate all the sections at once
          // setAllSectionPixels(pos, meteorColor2, inverse);

          // Alt Colors
          // for(byte sectionIdx = 0; sectionIdx < PPG_SECTIONS; sectionIdx++) {
          //   bool splitInverse = inverse;
          //   if(sectionIdx == 1){splitInverse = !splitInverse; }
          //   setSectionPixels(sectionIdx, pos, getColorAltClusterPattern(pos), splitInverse);
          // }

          // Spyder Colors
          // for(byte nodeIdx = 0; nodeIdx < PPG_NODES; nodeIdx++) {
          //   setNodePixels(nodeIdx, pos, getColorAltNodePattern(nodeIdx, pos, inverse), false);
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

/** @return color preset based on global user config */
BiColor getBiColor(){
  BiColor col = {
    CRGB(255, 255, 255), // White
    CRGB(255, 255, 255)  // White
  };

  switch (useAltColorPreset) {
    case AltColorPresets::C_BF: {
      // === Brandon Woelfel === //
      col.col1 = CRGB(100, 0, 255); // Purple
      col.col2 = CRGB(0, 255, 200); // Cyan
      break;
    }
    case AltColorPresets::C_EVE: {
      col.col1 = CRGB(200, 0, 190); // Pink
      col.col2 = CRGB(10, 40, 200); // Blue
      break;
    }
    case AltColorPresets::C_RETRO: {
      col.col1 = CRGB(200, 0, 190); // Pink
      col.col2 = CRGB(0, 255, 200); // Cyan
      break;
    }
    case AltColorPresets::C_FIRE: {
      col.col1 = CRGB(255, 80, 0); // Orange
      col.col2 = CRGB(200, 0, 0);  // Red
      break;
    }
    case AltColorPresets::C_PUMPKIN: {
      col.col1 = CRGB(255, 80, 0); // Orange
      col.col2 = CRGB(0, 200, 0); // Green
      break;
    }
    case AltColorPresets::C_PUMPKIN2: {
      col.col1 = CRGB(255, 80, 0); // Orange
      col.col2 = CRGB(255, 150, 0); // Yellow
      break;
    }
    case AltColorPresets::C_HALLOWEEN: { // Boo!
      col.col1 = CRGB(255, 80, 0);  // Orange
      // col.col2 = CRGB(100, 0, 255); // Purple
      col.col2 = CRGB(121, 2, 181); // Purple
      break;
    }
    case AltColorPresets::C_POLICE: { // O.o wee woo wee woo
      col.col1 = CRGB(200, 0, 0);   // Red
      col.col2 = CRGB(10, 40, 200); // Blue
      break;
    }
    case AltColorPresets::C_JOKER: { // Why so Serious?
      col.col1 = CRGB(100, 0, 255); // Purple
      col.col2 = CRGB(0, 200, 0); // Green
      break;
    }
    case AltColorPresets::C_SPYDER: {
      col.col1 = CRGB(255, 100, 0); // Orange
      col.col2 = CRGB(35, 50, 80);  // Blue
      break;
    }
    default: {
      col.col1 = CRGB(255, 0, 0); // Red
      col.col2 = CRGB(0, 0, 0);   // Green
      break;
    }
  }
  return col;
}

/**
 * Checks if the amount of time has passed
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
