// LED strip preset for: PPG AC Nitro 200 that have "ws2815" chipset leds

#ifndef PRESET_H
#define PRESET_H

// Max Pixels: PPG Strip: 270
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

#endif
