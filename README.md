# Konsole System Commands Plugin

A Konsole terminal emulator plugin that provides quick access to commonly used system administration and monitoring commands through a convenient menu interface.

![System Commands Plugin Screenshot](https://github.com/macsplit/konsole-system-commands-plugin/blob/main/system-commands.png?raw=true)

## Purpose

This plugin extends Konsole with a system commands menu that allows users to execute frequently used commands like process monitoring, disk usage checks, network diagnostics, and other system administration tasks without having to remember or type the complete commands.

## Installation

### JSON Configuration File

The plugin configuration file must be placed at:
```
~/.local/share/konsole/system-commands.json
```

### Plugin Binary

The compiled plugin binary (.so file) should be copied to one of the following location:

```
/usr/lib/x86_64-linux-gnu/qt5/plugins/konsoleplugins/
```

## Compatibility

- **Konsole Version:** 22.04 and later
- **Qt Version:** Qt5 5.15+ or Qt6
- **KDE Frameworks:** 5.90+ or KF6
- **Operating System:** Linux distributions similar to Ubuntu with KDE/Plasma desktop

For detailed dependency information, refer to the [Konsole build documentation](https://invent.kde.org/utilities/konsole/-/blob/master/README.md).

## Source

This plugin was extracted from the main [KDE Konsole](https://invent.kde.org/utilities/konsole) repository to be maintained as a standalone component.

## Configuration

The plugin uses a JSON configuration file to define available commands. See `example-system-commands.json` in this repository for the complete configuration format.

Rename this to `system-commands.json` and place it at `~/.local/share/konsole`

### JSON Structure

- `rootCommands`: Array of commands available at the top level
- `categories`: Object containing categorized command groups
  - Each category contains an array of command objects or simple command strings
  - Command objects have `name` and `command` properties
  - Simple strings are treated as both the name and command

## Build Instructions

### Prerequisites

- CMake 3.16+
- Qt5 development libraries or Qt6
- KDE Frameworks development packages
- C++ compiler with C++17 support

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Installation

```bash
# System-wide installation (requires sudo)
sudo make install

# Or copy manually to user directory
cp libsystem-commands-plugin.so ~/.local/lib/qt5/plugins/konsolepart/
```

For complete build dependency information and detailed build instructions, see the main [Konsole repository](https://invent.kde.org/utilities/konsole).

## Contributing

This repository is maintained by [@macsplit](https://github.com/macsplit). Issues and pull requests are welcome.

## Development

Source code is located in the `src/` directory. The plugin follows the standard Konsole plugin architecture using Qt's plugin system.

## License

This plugin follows the same licensing as Konsole itself. See the main Konsole repository for license details.
