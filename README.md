# AndyEdit


     ___,                       ___
    /   |             |        / (_)   |  o
   |    |   _  _    __|        \__   __|    _|_
   |    |  / |/ |  /  |  |   | /    /  |  |  |
    \__/\_/  |  |_/\_/|_/ \_/|/\___/\_/|_/|_/|_/   v0.2
                             /|
			     \|

A simple, line-oriented, terminal-based text editor with an emacs-like keybinding.

The editor is inspired by, but not based on, Microemacs and Kilo.

## Getting Started

To download:

> git clone https://github.com/asuttles/AndyEdit.git


From the top-level directory, make the editor:

> make


## Prerequisites

The AndyEdit editor requires the ncurses library be installed for terminal rendering.

AndyEdit is being developed using the LLVM CLANG 6.0.0 compiler and ncurses 5.7 on OpenBSD 6.4.

AndyEdit uses the POSIX getline utility.  This may not be found on all systems.

## Versioning

### Release 0.2 (WIP)
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

### Release 0.1
  - Splash Screen
  - Open Buffer from command line
  - Initialize terminal and buffer data structures
  - Buffer navigation: up/down lines, paging, home/end, etc.
  - Read/Eval user inputs
  - Capture Control Keys
  - Status bar
  - Text Renderer
  

## TODO
   - Save modified buffers
   - Row highlighting
   - Undo/Redo
   - Rectangle Operations with Point/Mark

## Author

* **Andrew Suttles** - [asuttles](https://github.com/asuttles)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

I would like to acknowledge Richard Stallman for the amazing EMACS text editor and countless of my fellow hobyist software developers who have written the various Ersatz Emacs clones which have inspired this present effort.

