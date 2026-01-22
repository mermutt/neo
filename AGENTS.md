# AGENTS.md - Neo Matrix Rain Terminal Application

## Project Overview

**neo** is a C++ terminal application that recreates the digital rain effect from "The Matrix". It displays streams of random characters scrolling down the terminal screen with configurable colors, character sets, and effects.

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
make -j3
sudo make install
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
- **Language**: C++11 with some C idioms
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

### Common Tasks for AI Agents

1. **Adding New Features**:
   - Check `HACKING` for contribution guidelines
   - Follow existing patterns in Cloud/Droplet classes
   - Update command-line parsing in `neo.cpp`

2. **Bug Fixes**:
   - Reproduce issue with specific CLI options
   - Check terminal compatibility issues
   - Verify Unicode/color support

3. **Performance Improvements**:
   - Profile with `-g -O0` debug build
   - Focus on drawing optimization
   - Consider memory pool optimizations

4. **Documentation Updates**:
   - Update `README.md` for user-facing changes
   - Update `doc/neo.6` for man page changes
   - Add examples to `examples/` directory

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
- **Garbage Characters**: Use `--charset=ascii` or `--colormode=0`
- **No Colors**: Check `TERM` variable, use `--colormode` option
- **Blinking Characters**: Disable with `--noglitch`
- **Unicode Issues**: Ensure locale supports UTF-8, use Unicode-capable font
- **Terminal Resize**: Not handled - terminal size is fixed at startup. The `KEY_RESIZE` event is intentionally ignored as the use case assumes a fixed terminal size.

This document should help AI agents quickly understand the neo codebase and work effectively on future tasks.
