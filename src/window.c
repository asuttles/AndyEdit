/***
==========================================================================================
            _              _         _____    _ _ _
           / \   _ __   __| |_   _  | ____|__| (_) |_
          / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
         / ___ \| | | | (_| | |_| | | |__| (_| | | |_
        /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.5-beta [Release]
                             |___/

        Copyright 2020 (andrew.suttles@gmail.com)
        MIT LICENSE

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT 
 OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 DEALINGS IN THE SOFTWARE.

 AndyEDIT is a simple, line-oriented, terminal-based text editor with emacs-like keybindings.

 For more information about AndyEdit, see README.md.

==========================================================================================
 ***/
#include <curses.h>

#include "ae.h"
#include "window.h"

static WINDOW *WIN;				/* Window Handle */

/* Restore tty */
void closeEditor() {

  endwin();
}


/* Needed For 'wgetch' in KeyPress Module */ 
WINDOW *getWindowHandle() {

  return WIN;
}


static void _initColor( void ) {

  start_color();

  init_pair( NORMAL_BACKGROUND, COLOR_WHITE, COLOR_BLACK );
  init_pair( HIGHLT_BACKGROUND, COLOR_WHITE, COLOR_BLUE );
  
  return;
}



/* Prepare tty for Raw nCurses Input */
void initializeTerminal() {

  if(( WIN = initscr() ) == NULL ) { /* Setup ncurses */
    die( "initializeTerminal: initscr failed" );
  }

  if( cbreak() == ERR ) {        /* Unbuffered Input */
    die( "initializeTerminal: cbreak failed" );
  }

  if( noecho() == ERR ) {        /* Do NOT local echo */
    die( "initializeTerminal: noecho failed" );
  }

  if( nonl() == ERR ) {                /* \n != \r\n */
    die( "initializeTerminal: nonl failed" );
  }

  if( keypad( stdscr, TRUE ) == ERR ) { /* Enable keypad */
    die( "initializeterminal: keypad failed" );
  }

  if( raw() == ERR ) {                /* Set ICANON, ISIG, IXON off */
    die( "initializeterminal: raw" );
  }

  if( has_colors() == FALSE ) {
    die( "Teminal Does Not Support Color" );
  }
  else {
    _initColor();
  }
  
  timeout(100);
}


int getWinNumCols( void ) {

  return getmaxx( WIN );
}
int getWinNumRows( void ) {

  return getmaxy( WIN );
}
int getWinThisCol( void ) {

  return getcurx( WIN );
}
int getScreenRows( void ) {

  return getmaxy( WIN ) - 3;
}


/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
