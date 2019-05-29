# Andy Edit

A simple, line-oriented, terminal-based text editor with an emacs-like keybinding.

The editor is inspired by, but not based on, Microemacs and Kilo.

## Getting Started

To download:
'''
> git clone https://github.com/asuttles/AndyEdit.git
'''

From the top-level directory, make the editor:
'''
> make
'''

## Prerequisites

The Andy Edit editor requires the ncurses library be installed for terminal rendering.

The Andy Edit editor is being developed using the LLVM CLANG 6.0.0 compiler and ncurses 5.7 on OpenBSD 6.4.

## Versioning

### Release 0.2 In Active Development
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

### Release 0.1 (Never Released)
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
   - Self insert printable characters
   - Row highlighting
   - Undo/Redo
   - Rectangle Operations with Point/Mark

## Author

* **Andrew Suttles** - [asuttles](github.com/asuttles)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgements

I would like to acknowledge Richard Stallman for the amazing EMACS text editor and countless of my fellow hobyist software developers who have written the various Ersatz Emacs clones which have inspired this present effort.

