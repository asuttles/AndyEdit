# AndyEdit


         ___,                       ___
        /   |             |        / (_)   |  o
       |    |   _  _    __|        \__   __|    _|_
       |    |  / |/ |  /  |  |   | /    /  |  |  |
        \__/\_/  |  |_/\_/|_/ \_/|/\___/\_/|_/|_/|_/   v0.5-beta [Release]
                                 /|
                                 \|


A simple, featureless, one file-at-a-time, line-oriented, terminal-based text editor with emacs-like keybindings.

The editor is inspired by, but not derived from, similar works like mg and Kilo.

This readme was edited in AndyEdit!

## Getting Started

To use AndyEdit, down the source code from its git repository and build the project using make.  BSD users will have to install gmake to build the included makefile.

To download:

> git clone https://github.com/asuttles/AndyEdit.git


From the top-level directory, simply type 'make' to install the editor:

> [g]make install

The editor installs in the user's bin directory.

## Prerequisites

The AndyEdit editor requires the ncurses library be installed for terminal rendering.

AndyEdit uses the POSIX 'getline' utility.  This may not be found on all systems.

AndyEdit has been developed using the latest Clang compiler and ncurses 5.7 on OpenBSD 6.7.

AndyEdit has been tested using the GNU gcc 9.3 compiler and ncurses 6.2 on Void Linux 5.4.

## Versioning

Latest Release: 0.5-beta

See [TODO](TODO) file for future release features.

### Release 0.5-beta [RELEASE]
  - Added Universal Argument
  - Added Rudimentary Kill/Yank Line
  - Added backspace feature to minibuffer inputs
  - Added word-search forward/backward feature

### Release 0.4-beta (15 MAY 20)
  - Removed GPL-licensed getline() utility
  - Updated file open/close menu functionality
  - Fully modularized the source tree for ease of maintenance and development. See [ARCHITECTURE](ARCHITECTURE) for rudimentary details on the modules that make up the system.
  - Cleaned up several memory leaks

### Release 0.3-beta (23 APR 20)
  - Kill Word
  - Highlight Region Between Point/Mark
  - Kill Buffer
  - Save Buffer on Exit
  - Region Highlighting (when mark active)
  - Kill Region
  - Backspace Works Across Lines
  - Find (alternate) File
  - Display Help

### Release 0.2-alpha
  - Save Buffer (as)
  - Open Line
  - Kill Line
  - Meta Key by Second Level Dispatch
  - Jump to Line 
  - Minibuffer Read/Write Messages
  - Set Mark and Swap Point/Mark
  - Cursor Movement Function Updates Terminal State
  - Edits to a Line updates Editor State
  - eXtension Menu
  - Dirty Flag/State for Modified Buffer
  - Forward/Backward Word
  - Self insert printable characters
  - Delete and Backspace

### Release 0.1-alpha (A Text Viewer)
  - Splash Screen
  - Open Buffer from command line
  - Initialize terminal and buffer data structures
  - Buffer navigation: up/down lines, paging, home/end, etc.
  - Read/Eval user inputs
  - Capture Control Keys
  - Status bar
  - Text Renderer
  
## Author

* **Andrew Suttles** - [asuttles](https://github.com/asuttles)

Contributions to AndyEdit are welcome.  See [CONTRIBUTE](CONTRIBUTE.md) file for further information.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

I would like to acknowledge countless of my fellow hobyist software developers who have written the various Ersatz Emacsen which have inspired this present effort.

