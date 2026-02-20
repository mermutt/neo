# AGENTS.md - Neo Matrix Rain Terminal Application

## Project Overview

**neo** is a C++ terminal application that recreates the digital rain effect from "The Matrix". It displays streams of simingly random characters scrolling down the terminal screen with configurable colors, character sets, and effects.
The primary purpose of this application is to display a portion of text from input file at the end of each epoch. Next epoch will display text at offset where the previous epoch ended. Hence, entire input file will be displayed with a number of epochs.

## Key Information for AI Agents

### Build System & Dependencies
- **Build System**: Autotools (autoconf/automake)
- **Compiler**: C++17 compatible (g++ or clang++)
- **Dependencies**:
  - `libncursesw-dev` (wide character ncurses)
  - `build-essential` (make, g++)
  - `autoconf` (for building from repo)

### Project Structure
```
neo/
├── src/                    # Source code
│   ├── neo.cpp            # Main program logic, CLI options, ncurses init
│   ├── neo.h              # Enums, structs, utility functions
│   ├── cloud.cpp          # Cloud class - manages all Droplets
│   ├── cloud.h            # Cloud class header
│   ├── droplet.cpp        # Droplet class - moves/draws characters
│   └── droplet.h          # Droplet class header
├── doc/                   # Documentation
│   ├── HACKING           # Development guide
│   ├── INSTALL           # Installation instructions
│   └── neo.6             # Man page
├── examples/             # Example files
├── assets/               # Images and screenshots
└── configure.ac          # Autoconf configuration
```

### Core Architecture

1. **Main Loop** (`neo.cpp`): Handles initialization, command-line parsing, and the main event loop
2. **Cloud Class** (`cloud.cpp/h`): Manages all Droplets, screen state, colors, and effects
3. **Droplet Class** (`droplet.cpp/h`): Represents a vertical stream of characters, handles movement and drawing

### Key Design Patterns
- **Object Pool Pattern**: Pre-allocates objects (Droplets, characters) for performance
- **Frame Optimization**: Only draws changed characters each frame
- **Tight Coupling**: Cloud and Droplet are tightly coupled with bidirectional calls

### Important Files & Functions

#### `src/neo.h`
- **Enums**: `Charset`, `Color`, `ColorMode`
- **Struct**: `ColorContent` for custom color definitions
- **Functions**: `Die()` (error handling), `Cleanup()`

#### `src/cloud.h`
- **Cloud Class**: Main controller class
- **Key Methods**:
  - `Rain()`: Main animation loop
  - `GetAttr()`: Character attribute calculation
  - `SetColor()`, `SetCharsPerSec()`: Configuration
- **Configuration**: Color modes, shading, bold modes, message display

#### `src/droplet.h`
- **Droplet Class**: Individual character stream
- **Tracking**: `CurLine` (last drawn), `PutLine` (to draw)
- **Movement**: Speed, length, linger time controls

### Build Commands

#### From Repository:
```bash
./autogen.sh
./configure
make -j
```

#### Debug Build:
```bash
./configure CXXFLAGS='-g -O0 -Wall -Werror'
make
```

#### Optimized Build:
```bash
./configure CXXFLAGS="-Os -march=native -mtune=native -DNDEBUG -fno-exceptions -ffunction-sections -fdata-sections -flto" LDFLAGS="-Wl,--gc-sections -flto" CXX="clang++"
make -j3
sudo make install-strip
```

### Testing & Verification
- **Run Tests**: Execute compiled binary `./src/neo` with various options
- **Check Output**: Verify terminal display shows Matrix rain effect
- **Common Options**: `--help`, `--version`, `--message "text"`, `--color green`

### Code Style & Conventions
- **Language**: C++17 with some C idioms
- **Bracing**: K&R style
- **Indentation**: Soft tabs (spaces)
- **Naming**: CamelCase for classes, snake_case for variables/functions
- **Headers**: Use `#pragma once` instead of include guards
- **Error Handling**: `Die()` function for fatal errors

### Development Notes
- **Performance Focus**: Heavy optimization for terminal rendering
- **Unicode Support**: Uses ncursesw for wide character support
- **Color Modes**: Supports 16/256/truecolor modes with auto-detection
- **Character Sets**: Multiple predefined charsets (katakana, Greek, Cyrillic, etc.)

### Quick Reference

| Command | Purpose |
|---------|---------|
| `make` | Build the project |
| `sudo make install` | Install system-wide |
| `sudo make uninstall` | Remove installation |
| `neo -h` | Show help/options |
| `man neo` | View manual page |

### Environment Variables
- `LANG`: Should contain "UTF" for Unicode support (e.g., `en_US.UTF-8`)
- `TERM`: Terminal type for color support
- macOS: May need `LDFLAGS`/`CPPFLAGS` for Homebrew ncurses

### Known Issues & Workarounds
- **No Colors**: Check `TERM` variable, use `--colormode` option
- **Unicode Issues**: Ensure locale supports UTF-8, use Unicode-capable font
- **Terminal Resize**: Not handled - terminal size is fixed at startup. The `KEY_RESIZE` event is intentionally ignored as the use case assumes a fixed terminal size.

### How It Works

1. **Dual-Pass System**:
   - **Simulation Pass**: Before displaying anything, the app runs a complete simulation of an "epoch" to determine where all droplets will end up. At the end of the epoch, it sorts all active droplets and assigns their `_dataOffset` and `_topFreezeLine` so that characters from the input file are displayed column by column (left to right, top to bottom).
   - **Display Pass**: The actual animation replays the same pseudo-random sequence using identical seeds, producing the same droplet positions as the simulation

2. **Key Data Structures**:
   - `Droplet._dataOffset`: Byte offset into the input text file where this droplet's text starts
   - `Droplet._topFreezeLine`: Screen line where the top of this droplet was positioned at epoch end
   - These are calculated during `SimulateEpoch()` and used during the actual `Draw()`

3. **Text Mapping**:
   - When a droplet is drawn at/after its `_topFreezeLine`, characters come from the input file
   - `GetChar()` at `cloud.cpp:447-455` returns file content: `_mmapData[(_mmapOffset + dataOffset) % _mmapSize]`
   - Before `_topFreezeLine`, random characters from the char pool are shown
   - After `_topFreezeLine`, input text characters are displayed

4. **Column-First Reading**:
   - To read the hidden message from screen output, read column by column (top-to-bottom, then next column)
   - Each column's characters form part of the continuous text stream

### Source Files
- **Input text**: `input_text.txt` in the project root (the file to be displayed)
- **Screen capture**: In debug mode, screen content is written to `screen-1.txt`, `screen-2.txt`, etc. at the end of each epoch (text files containing visible screen output)

### Debugging Tips
- Check `SimulateEpoch()` in `cloud.cpp:160-225` for epoch simulation logic
- Check `CountDropletsAndChars()` in `cloud.cpp:350-381` for dataOffset calculation
- Check `GetChar()` in `cloud.cpp:447-455` for character retrieval
- Check `Draw()` in `droplet.cpp:141-187` for the drawing logic that uses `_dataOffset`
- At the end of epoch screen content will be written to `screen-<epoch-number>.txt` file
