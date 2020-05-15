/***
==========================================================================================
                 _              _         _____    _ _ _
                / \   _ __   __| |_   _  | ____|__| (_) |_
               / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
              / ___ \| | | | (_| | |_| | | |__| (_| | | |_
             /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.4-beta [Draft]
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
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <stdbool.h>

#include "keyPress.h"
#include "render.h"
#include "buffer.h"
#include "window.h"
#include "files.h"
#include "state.h"

/*****************************************************************************************
				      HANDLE ERRORS
*****************************************************************************************/

/* Print Error Message and Exit */
void die( const char *s ) {

  perror(s);
  sleep(3);
  closeEditor();
  
  exit(EXIT_FAILURE);
}

/*****************************************************************************************
				       MAIN PROGRAM
*****************************************************************************************/

/* Display Splash Screen */
void displaySplash( void ) {

  int center = (getWinNumCols() / 2);
  int third  = (getWinNumRows() / 3);
  
  mvaddstr( third + 0, center - 10, "Welcome to Andy Edit!" );
  mvaddstr( third + 1, center - 6, "Version 0.4 beta" );
  mvaddstr( third + 3, center - 9, "(c) Copyright 2020" );

  refresh();
  sleep(2);

  clear();
  renderText();
}

/* Editor Polling Loop */
int main( int argc, char *argv[] ) {

  /* Initialize */
  initializeTerminal();
  initializeBuffer();

  /* Open File or Display Splash */
  if( argc > 1 ) {
    setFilename( argv[1] ); 
    readBufferFile( getBufferFilename() );
  }
  else {
    openEmptyBuffer( DEFAULT );
    displaySplash();
  }
  
  /* Process Key Presses */
  while( true ) {
    renderText();
    processKeypress();
  } 

  /* Shutdown */
  closeEditor();
  return EXIT_SUCCESS;
} 


/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:

   -----------------------------
  < AndyEdit is Udderly Sweet! >
   -----------------------------
        o   ^__^
         o  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
END PROGRAM
 ***/
