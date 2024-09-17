# Energy Mechanism Documentation

[中文版](README_ZH.md)

> Southern University of Science and Technology ARTINX Team
>
> Author: Huang Tingjun (Hardware for Season 23)

*Note: This README is a direct translation from the original Chinese version using ChatGPT. Accuracy of translations is not guranteed.*

## Introduction

### Basic Concept

The software control principle of the energy mechanism can be divided into three parts:

1. The 3508 motor drives the energy mechanism to rotate according to the rules.
2. The LED panel lights up according to official requirements.
3. Impact detection.

The first part is relatively simple and can be implemented with basic electrical control. When paired with an A board and a DR16, remote control can be achieved via the DT7 remote controller.

For the second part, we use WS2812B LED lights, which can be programmed to light up according to official requirements using specific code logic.

As for the third part, due to time constraints, we did not implement impact detection. However, we have left an interface for it, so once the appropriate sensor is connected, the global array can be modified to realize impact detection. Currently, for easier visual calibration, we use infrared remote control to simulate impact detection, displaying the energy mechanism's response when hit.

This document focuses on explaining the implementation of the second and third parts of the code.

### How Did We Get Our Symbol?

For this season, we borrowed the energy mechanism design from the Hanjiang Team at Shenzhen Technology University. We sincerely thank Hanjiang for their generous assistance.

Hanjiang’s large symbol design is nearly complete, lacking only LED boards and impact detection in the bullseye, but it is otherwise quite refined. Of course, there are areas for improvement, such as potential signal interference in the PCB design between the inner and outer frames and occasional color mismatches with data packets when current flow is high (when LEDs are very bright). Additionally, since the LED boards are connected via headers or Dupont wires, poor connections can occur (manifesting as color issues or inconsistent lighting). We resolved this by replacing the connectors with soldered wires.

*Note: All references to LED boards in this document refer to the diagram below.*

<img src="./assets/image-20230820223244092.png" alt="image-20230820223244092" style="zoom: 50%;" />

For the software and main control hardware, we implemented our own solutions.

## Main Control Board

Due to time constraints, we reused the main control board from last season. However, because we added infrared reception functionality (for impact detection) and rewrote the code, the actual board differs somewhat from the annotated schematic to be compatible with Hanjiang’s symbol.

<img src="./assets/image-20230826164944130.png" alt="image-20230826164944130" style="zoom: 50%;" />
<img src="./assets/image-20230826165001402.png" alt="image-20230826165001402" style="zoom:50%;" />
<img src="./assets/image-20230826165031846.png" alt="image-20230826165031846" style="zoom:50%;" />

These are the schematic, PCB, and 3D renderings of the main control board. Since the back of the PCB has no components, only the front is shown.

**Key Notes:**
- The two rows of pins in the middle of the PCB actually correspond to the STM32F103C8T6 minimum system board.
- The 2-pin connector at H9 was abandoned this season.
- The switch at SW1 was changed to a 3-pin header for PWM signal output to the R symbol.
- H5 was changed to an input capture pin connected to the infrared receiver due to a conflict with another DMA channel during code modification (which also conveniently uses three pins: 5V, GND, OUT).
- The other 3-pin headers (H3,4,6,7,8) are used to send PWM signals to the blades. Since Hanjiang’s large symbol uses independent power for each LED panel, only the middle pin outputs PWM, while the other two pins remain disconnected.
- The XT30 input is for 5V power supply. If using an official battery for power, an external 24V-5V transformer is required.
- Due to voltage drops from the numerous LEDs (over 1500 on each blade), an additional 5V supply is needed to maintain voltage consistency at the back end of the circuit.

**Before diving into the code, please read the WS2812B datasheet.**

**Regarding the LED panel on a blade, the connection sequence is: rectangular panel -> strip -> inner frame -> bullseye -> outer frame.**

## Code Section

[Tangent-H/RM2023_Energy_Mechanism: ARTINX LED Code for RM_2023_Energy_Mechanism (github.com)](https://github.com/Tangent-H/RM2023_Energy_Mechanism)

The latest code has been uploaded to GitHub. There’s also a functional version available in the team’s GitLab, though the colors and patterns are not the final version.

### Drive Section

The LED driving code is adapted from the following open-source project:

[Silencer2K/stm32f10x-ws2812b-lib: STM32 WS2812B library (github.com)](https://github.com/Silencer2K/stm32f10x-ws2812b-lib)

This driver is written using the STM32 standard library and only supports one PWM output channel. However, we needed six PWM output channels (5 for the blades and 1 for the R symbol), so we had to modify the code.

The specific code-to-main control board mappings are as follows:

- H3 -> PA1 -> TIM2_CH2 -> DMA1_CH7 (Blade)
- H4 -> PA2 -> TIM2_CH3 -> DMA1_CH1 (Blade)
- H5 -> PA3 -> TIM2_CH4 -> DMA1_CH7 (Conflicted with the first DMA channel, so abandoned; connected to infrared receiver instead)
- H6 -> PA6 -> TIM3_CH1 -> DMA1_CH6 (Blade)
- H7 -> PB1 -> TIM3_CH4 -> DMA1_CH3 (Blade)
- H8 -> PA8 -> TIM1_CH1 -> DMA1_CH2 (Blade)
- H9 -> PB15 (2-pin, abandoned)

The switch GPIO pin (SW1) -> PA0 -> TIM2_CH1 -> DMA1_CH5 (R symbol)

Files for the WS2812B driver are:

```
bitmap.c
bitmap.h
ws2812b.c
ws2812b.h
ws2812b_conf.h
```

The driver code is straightforward (the example below is from the original author's repository and not present in our code):

```c
#include "ws2812b.h"

#define NUM_LEDS    300

RGB_t leds[NUM_LEDS];

int main() {
    ws2812b_Init();

    while (1) {
        while (!ws2812b_IsReady()); // wait

        //
        // Fill leds buffer
        //

        ws2812b_SendRGB(leds, NUM_LEDS);
    }
}
```

In simple terms, you initialize using `ws2812b_Init()` before the while loop. Once `ws2812b_IsReady()` returns true, you fill the LED array and then send the data using `ws2812b_SendRGB()`. The original code also supports sending data using the HSV format, but we did not use that and simply converted between RGB and HSV as needed.

For the PA8 port, since it is connected to an advanced timer (unlike the general timers used by other ports), additional lines of code are needed to enable PWM output:

```c
TIM_Cmd(TIM1, ENABLE);
TIM_CtrlPWMOutputs(TIM1,ENABLE);    // Advanced timer main output needs this line
DMA_Cmd(DMA1_Channel2, ENABLE);
```

The driver had to be rewritten mainly because the STM32F103C8T6 has limited memory. The symbol for the new season adds LED panels in the bullseye, and the original code was not memory efficient enough, causing memory overflow. (The original code used 24 bytes to represent 24 bits of data.)

### Logic Section

The logic in this code mostly follows what was developed last season by Wu Zizheng.

<img src="./assets/image-20230826214356140.png" alt="image-20230826214356140" style="zoom: 50%;" />

The directory structure is shown above. The key folders are Hardware, System, and User, while the others contain standard libraries.

In the *Hardware* folder:
- `bitmap.c`, `bitmap.h`, `ws2812b.c`, `ws2812b.h`, and `ws2812b_conf.h` contain the WS2812B driver code.
- `infrared.c` and `infrared.h` handle infrared detection and response.
- `pattern.h` contains some standard patterns (e.g., the arrow in the "standby" state, the bullseye).
- `led.c` and `led.h` contain the higher-level functions for controlling the energy mechanism’s display patterns and colors.

In the *System* folder:
- `Delay.c` and `Delay.h` implement delay functions using the systick timer.
- `rand.c` and `rand.h` contain functions for randomly selecting a blade (though this turned out to be unnecessary, as the standard `rand` function sufficed for visual purposes).
- `timer.c` and `timer.h` serve three purposes:
  1. Set a refresh rate for the arrow moving on the blade (though this proved less useful, as high refresh rates caused data issues).
  2. Time 2.5s intervals to switch between blades.
  3. Provide timing for infrared reception.

In the *User* folder:
-

 `main.c` is the entry point of the program.

You can start by looking at `main.c`:

```c
#include "stm32f10x.h" // Device header
#include "led.h"
#include "infrared.h"
#include "Delay.h"

int main(void)
{
	IR_Init();
	LED_Init(RedState);	
	
	while (1)
	{
		Delay_ms(100);
		while(!ws2812b_IsReady());
		LED_Update();
	}
}
```

The first two lines initialize the infrared module and the LEDs (defaulting to RedState). Then the main loop begins. A 100ms delay ensures proper data transmission for the LEDs. The loop then waits until the transmission is ready, after which it updates the LEDs based on the current state stored in global variables.

The most important files are `led.c` and `led.h`. `pattern.h` provides the logic for generating specific patterns, while `infrared.c` decodes the infrared signals and modifies the global variables in `led.c` to reflect the current state of the energy mechanism.

Below are the function declarations in `led.h`:

```c
void LED_Init(LED_State_t state);
void LED_PackLightALLData(void); 
// Packs data to light up all LEDs (used for testing)

void LED_PackRectangleData(LED_Leaf_Mode_t leafmode, RGB_t * dst); 
// Packs data for the rectangular panel
void LED_PackStripData(LED_Leaf_Mode_t leafmode, RGB_t * dst); 
// Packs data for the strip
void LED_PackFrameData1(LED_Leaf_Mode_t leafmode, RGB_t * dst); 
// Packs data for the inner frame
void LED_PackFrameData2(LED_Leaf_Mode_t leafmode, RGB_t * dst); 
// Packs data for the outer frame
void LED_PackTargetData(LED_Leaf_Mode_t leafmode, RGB_t * dst); 
// Packs data for the bullseye

void LED_Update(void);
```

The logic implementation is as follows:

`LED_Update()` first calls an internal function `check_LED_Status()` to determine the current status of the energy mechanism, which is controlled by various global variables in `led.c`:

```c
uint8_t debug = 0; // If debug = 1, the blade does not switch every 2.5s, useful for calibration
uint8_t timeout = 0; // Set to 1 by hardware timer interrupt when 2.5s elapses
uint8_t refresh_rectangle = 0; // Set to 1 by hardware timer interrupt when it’s time to refresh the rectangular panel
uint8_t leaf_ring_value[5] = {0, 0, 0, 0, 0}; // Indicates which ring is currently active
static LED_Leaf_Name_t current_Refresh_Leaf = LEAF_0; // Defaults to the first blade

static RGB_t leds[LED_NUM]; // Stores LED data
static RGB_t R_logo[64]; // Stores R symbol data

LED_Leaf_Mode_t leafmode[5] = {LEAF_OFF, LEAF_OFF, LEAF_OFF, LEAF_OFF, LEAF_OFF}; // Stores the state of each blade
RGB_t current_color = {0, 0, 0}; // Color of the current blade

LED_State_t LED_State = RedState; // Defaults to RedState
uint8_t currentLeafStruck = 0; // Indicates if the current blade has been struck
LED_Leaf_Name_t current_striking_leaf = LEAF_0; // The current blade being struck
uint8_t total_struck = 0; // Total number of hits
```

The code loads data sequentially according to the LED board order, and then sends the data in one go to light up the LEDs on a blade. Other blades are handled similarly, so all data is sent sequentially rather than simultaneously.

Regarding the `pattern.h` arrays:

```c
const unsigned char TARGET_STRIKING[1024] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
...
};
```

This array represents the lighting pattern for the 1024 LEDs in the bullseye when the energy mechanism is in standby mode. A `1` indicates the LED is on, while a `0` indicates it is off. 

Arrays in `pattern.h` are stored as `const` to place them in the MCU’s larger flash memory rather than the more limited RAM, as these arrays contain a lot of data.

Animations (like the moving arrow) are achieved by changing the starting point of the array traversal during each iteration, creating the illusion of movement.

For converting images to arrays, we used a tool called `Img2Lcd`. This tool converts images into C arrays suitable for WS2812B LEDs.

For example:

<img src="./assets/image-20230827133512935.png" alt="image-20230827133512935" style="zoom:33%;" />

This 16x16 quarter bullseye was drawn in Photoshop. Due to symmetry, the remaining quarters can be mirrored to create the full bullseye.

Settings in `Img2Lcd` allow the image to be exported as an array with the desired format (256 colors). I replaced `0xFF` with `1` and `0x00` with `0` for clarity.

![image-20230827134156111.png](./assets/image-20230827134156111.png)

Finally, since many WS2812B panels alternate scan directions between odd and even columns, I generate the odd-column data first, then generate the even-column data by scanning from bottom to top. By combining these two sets, we get the correct array representation for the pattern.

![image-20230827134415431.png](./assets/image-20230827134415431.png)

