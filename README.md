# AndyEdit


         ___,                       ___
        /   |             |        / (_)   |  o
       |    |   _  _    __|        \__   __|    _|_
       |    |  / |/ |  /  |  |   | /    /  |  |  |
        \__/\_/  |  |_/\_/|_/ \_/|/\___/\_/|_/|_/|_/   v0.4-beta
                                 /|
                                 \|


A simple, featureless, one file-at-a-time, line-oriented, terminal-based text editor with emacs-like keybindings.

The editor is inspired by, but not derived from, similar works like mg and Kilo.

This readme was edited in AndyEdit!

## Getting Started

To download:

> git clone https://github.com/asuttles/AndyEdit.git


From the top-level directory, simply type 'make' to install the editor:

> make install


## Prerequisites

The AndyEdit editor requires the ncurses library be installed for terminal rendering.

AndyEdit uses the POSIX 'getline' utility.  This may not be found on all systems.

AndyEdit is being developed using the LLVM clang compiler and ncurses 5.7 on OpenBSD 6.6.
AndyEdit has been tested using the GNU gcc 9.3 compiler and ncurses 6.2 on Void Linux 5.4

   ________________                    
  < AE on OpenBSD! >              
    ---------------                 
     \                              
      \
                 |    . 
             .   |L  /|
         _ . |\ _| \--+._/| .       
        / ||\| Y J  )   / |/| ./    
       J  |)'( |        ` F`.'/     
     -<|  F         __     .-<      
       | /       .-'. `.  /-. L___  
       J \      <    \  | | O\|.-'  
     _J \  .-    \/ O | | \  |F     
    '-F  -<_.     \   .-'  `-' L__  
   __J  _   _.     >-'  )._.   |-'  
   `-|.'   /_.           \_|   F    
     /.-   .                _.<     
    /'    /.'             .'  `\    
     /L  /'   |/      _.-'-\        
    /'J       ___.---'\|            
      |\  .--' V  | `. `            
      |/`. `-.     `._)             
         / .-.\                     
         \ (  `\                    
          `.\                     

## Versioning

Latest Release: 0.3 (23APR20)

### Release 0.3
  - Kill Word
  - Highlight Region Between Point/Mark
  - Kill Buffer
  - Save Buffer on Exit
  - Region Highlighting (when mark active)
  - Kill Region
  - Backspace Works Across Lines
  - Find (alternate) File
  - Display Help

### Release 0.2
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

### Release 0.1 (A Text Viewer)
  - Splash Screen
  - Open Buffer from command line
  - Initialize terminal and buffer data structures
  - Buffer navigation: up/down lines, paging, home/end, etc.
  - Read/Eval user inputs
  - Capture Control Keys
  - Status bar
  - Text Renderer
  
## TODO
   - (auto)TAB Function
   - Undo/Redo
   - Syntax Coloring
   - Region Operations (Kill/Yank)

## Author

* **Andrew Suttles** - [asuttles](https://github.com/asuttles)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

I would like to acknowledge countless of my fellow hobyist software developers who have written the various Ersatz Emacsen which have inspired this present effort.

