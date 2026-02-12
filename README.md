# UART-Battleship-MSPM0
# Wireless Battleship - Embedded Systems Game

## Overview
A two-player wireless implementation of the classic Battleship game on MSPM0G3507 microcontrollers. Players compete on separate devices that communicate via UART to place ships, take turns firing, and track hits/misses in real-time with visual and audio feedback.

## Features

🎮 **Two-Player Wireless Gameplay** - Connect two microcontrollers via UART for head-to-head battles  
🚢 **Interactive Ship Placement** - Drag and drop 5 ships with collision detection  
🎯 **Turn-Based Combat** - Fire at enemy positions with immediate hit/miss feedback  
🌍 **Bilingual Interface** - Full support for English and French  
🎨 **Rich Visual Feedback** - Color-coded screens (green=hit, white=miss, red=lose, green=win)  
🔊 **Audio Feedback** - Sound effects for hits, misses, wins, and losses  
💡 **LED Status Indicators** - Visual turn indicators using onboard LEDs  
📊 **Live Scoreboard** - Real-time tracking of your hits vs opponent's hits  

## What Makes This Project Special

**Wireless Communication Protocol**: Designed a custom 4-byte UART protocol for transmitting game state between devices. Messages include marker coordinates, scores, and command flags, enabling seamless turn-based gameplay across two independent microcontrollers.

**State Machine Architecture**: Implemented a robust finite state machine with 7 states (MainMenu → ShipPlacement → Ready → MarkerPlacement → Waiting → EndGame) that coordinates UI updates, input handling, and wireless synchronization.

**Real-Time Interrupt-Driven Design**: Game logic runs at 30 Hz in a timer interrupt (TIMG12), handling input debouncing, cursor movement, and collision detection, while all ST7735 display operations occur in the main loop to prevent visual artifacts.

**Collision Detection System**: Built a 2D bounding box collision algorithm that prevents ships from overlapping during placement, checking both X and Y axis intersections for all 5 ships dynamically.

**Bilingual UI System**: Implemented a phrase table with 17 UI strings in both English and French, with ADC-controlled language selection and consistent rendering across all game states.

## Game Flow

### 1. Language Selection (MainMenu)
- Use slide potentiometer to choose English or French
- Press ACTION button to start

### 2. Ship Placement (ShipPlacement)
- Place 5 ships on your 8×8 grid:
  - 2× Three-cell ships
  - 2× Two-cell ships  
  - 1× One-cell ship
- Use D-pad to move cursor
- Press ACTION to pick up/set down ships
- Ships cannot overlap (collision detection enforced)
- Press ACTION2 when ready

### 3. Ready State
- System waits for both players to finish ship placement
- Handshake protocol determines who fires first

### 4. Your Turn (MarkerPlacement)
- Fire at enemy positions using cursor
- Place marker with ACTION button
- Press ACTION2 to toggle between your board and enemy board
- See live scores update after each shot

### 5. Opponent's Turn (Waiting)
- Wait for opponent to fire
- Receive hit/miss feedback with visual and audio cues
- Press ACTION2 to view your board or enemy board
- Game automatically transitions back to your turn

### 6. End Game
- First player to sink all 11 ship cells wins
- Display "YOU WIN" (green screen) or "YOU LOSE" (red screen)

## Technical Implementation

### Custom UART Protocol

**4-Byte Message Frames**:
```c
'<' [x_coord] [y_coord] [score]  // Fire marker
'>' [your_hits] [opp_hits] [score] // Hit/miss response  
'#' [padding] [padding] [padding]  // Ready signal
'!' [padding] [padding] [padding]  // Waiting signal
```

### State Machine States

| State | Description | Inputs | Outputs |
|-------|-------------|--------|---------|
| **MainMenu** | Language selection | ADC (language), ACTION (start) | Display language buttons |
| **ShipPlacement** | Position ships on grid | D-pad, ACTION (pick/place), ACTION2 (ready) | Ship sprites, cursor |
| **Ready** | Wait for opponent | UART receive | Handshake messages |
| **MarkerPlacement** | Fire at enemy | D-pad, ACTION (fire), ACTION2 (toggle view) | Markers, scores |
| **Waiting** | Opponent's turn | UART receive | Hit/miss screens, updated scores |
| **EndGame** | Display winner | None | Win/lose screen, sound |

### Collision Detection Algorithm

Ships are represented with bounding boxes:
```c
// For each picked-up ship
uint8_t Xmin = ship.x;
uint8_t Xmax = ship.x + ship.w - 1;
uint8_t Ymin = ship.y - ship.h + 1;
uint8_t Ymax = ship.y;

// Check overlap with all other ships
// X-axis intersection AND Y-axis intersection = collision
```

### Interrupt-Driven Input Handling

**TIMG12 at 30 Hz**:
- Reads button inputs and ADC values
- Debounces keys by comparing `lastKey` vs current `Key`
- Updates cursor position with bounds checking
- Manages all game state transitions
- Sets flags for main loop rendering

### Display Rendering Strategy

**Main Loop Responsibilities**:
- All `ST7735_DrawBitmap()` and `ST7735_OutString()` calls
- Grid redrawing when switching views
- Score updates
- Hit/miss overlays
- Ship and marker rendering

**Why Separate?** Timer interrupts must execute quickly. Display operations can take 10-50ms, causing jitter if done in ISR.

## Hardware Components

- **MSPM0G3507** microcontroller (2 required)
- **ST7735 LCD** (160×128 color display)
- **Slide Potentiometer** (ADC input for language selection)
- **Push Buttons** (D-pad + 2 action buttons)
- **LEDs** (Red, Yellow, Green for status)
- **Speaker/Buzzer** (Audio feedback via DAC)
- **UART Connection** (TX/RX between two devices)

## Pin Configuration

```
PB18 - ADC (slide potentiometer)
PB26 - Green LED (debugging/status)
PB27 - GPIO toggle (interrupt timing verification)
UART1 - Transmit to opponent
UART2 - Receive from opponent
```

## What I Learned

✅ Designed custom communication protocols for embedded systems  
✅ Implemented finite state machines for complex game logic  
✅ Built interrupt-driven architectures with proper ISR design  
✅ Developed 2D collision detection algorithms  
✅ Synchronized two independent systems wirelessly  
✅ Optimized display rendering to prevent visual artifacts  
✅ Debugged race conditions and timing issues  
✅ Managed multi-file embedded C projects  
✅ Integrated multiple peripherals (ADC, UART, DAC, GPIO, Timers)  

## Project Scope

**Time Required**: 40-60 hours total  
- Week 1: Ship placement and grid system (15-20 hours)
- Week 2: UART protocol and turn logic (12-18 hours)  
- Week 3: Collision detection and state transitions (8-12 hours)
- Week 4: Testing, debugging, bilingual support (5-10 hours)

**Lines of Code**: ~1,200 lines in Lab9Main.c + peripheral drivers

**Delivered**:
- Complete embedded C implementation
- Custom PCB design (optional)
- Wireless two-player gameplay
- Bilingual UI support
- Full audio/visual feedback

## Build and Run

### Prerequisites
- Code Composer Studio (CCS) or compatible IDE
- MSPM0 SDK installed
- Two MSPM0G3507 LaunchPads
- UART crossover cable (TX1→RX2, RX1→TX2, GND→GND)

### Compilation
```bash
# In CCS or command line with ARM GCC
make clean
make all
```

### Flashing
1. Connect LaunchPad via USB
2. Flash `Lab9Main.out` to both devices
3. Connect UART pins between devices
4. Power on both simultaneously

### Debugging
- **Green LED (PB27)**: Toggles every interrupt cycle (should blink at 30 Hz)
- **Serial Monitor**: Monitor UART traffic to verify protocol
- **Logic Analyzer**: Capture UART frames to debug synchronization

## Testing Strategy

### Unit Testing
- ✅ Collision detection with all ship configurations
- ✅ UART transmit/receive with test patterns
- ✅ State machine transitions (manual testing)
- ✅ ADC language selection thresholds
- ✅ Bounds checking for cursor movement

### Integration Testing
- ✅ Two-device gameplay (10+ full games)
- ✅ Edge cases (simultaneous inputs, rapid button presses)
- ✅ Win conditions (both players reaching 11 hits)
- ✅ View toggling during opponent's turn
- ✅ Language switching consistency

### Performance Testing
- ✅ Interrupt timing: 30 Hz maintained (verified with oscilloscope)
- ✅ UART bandwidth: <200 bytes/second (well within 9600 baud)
- ✅ Display refresh: <100ms per full redraw

## Known Issues & Improvements

**Current Limitations**:
- No ship rotation after placement
- Fixed 9600 baud UART (could support higher speeds)
- No game restart without reset

**Future Enhancements**:
- Add ship rotation with ACTION3 button
- Implement difficulty levels (AI opponent)
- Add network play via WiFi module
- Persistent high scores in EEPROM
- Animated ship explosion effects

## File Organization

```
src/
├── Lab9Main.c        - Main game loop and state machine
├── UART1.c/h         - UART transmit driver
├── UART2.c/h         - UART receive driver  
├── Switch.c/h        - Button input debouncing
├── LED.c/h           - LED control
├── Sound.c/h         - DAC audio generation
├── ADC1.c/h          - Analog input (slide pot)
├── ST7735.c/h        - Display driver
├── images/           - Sprite bitmaps (ships, cursor, markers)
└── inc/              - Hardware abstraction layer
```

## Acknowledgments

Built as the final project for ECE319K - Introduction to Embedded Systems at UT Austin.  
Team: Gauri Gupta, Clark Rucker  
Instructor: Dr. Jonathan Valvano  

## About

A real-time, interrupt-driven embedded systems project demonstrating wireless communication, state machine design, and peripheral integration on ARM Cortex-M0+ microcontrollers.

---

**Languages**: C (embedded), Assembly (startup code)  
**Peripherals**: UART, ADC, DAC, GPIO, Timers, SPI (display)  
**Tools**: Code Composer Studio, Logic Analyzer, Oscilloscope  
**Architecture**: ARM Cortex-M0+ (MSPM0G3507)
