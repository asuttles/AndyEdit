/***
==========================================================================================
            _              _         _____    _ _ _
           / \   _ __   __| |_   _  | ____|__| (_) |_
          / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
         / ___ \| | | | (_| | |_| | | |__| (_| | | |_
        /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.3
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
#define MINIBUFFSIZE 128

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>

#include "ae.h"
#include "keyPress.h"

char MINIBUFFER[MINIBUFFSIZE];		/* Minibuffer Input */


/*******************************************************************************
			      MINIBUFFER
*******************************************************************************/

/* Write Message to User */
void miniBufferMessage( const char *msg ) {

  int mbRow = getmaxy( getWindowHandle() ) - 1;
  
  /* Print Input Message */
  mvaddstr( mbRow, 0, msg );
  refresh();
}

/* Clear Minibuffer Messages */
void miniBufferClear() {
  
  int mbRow = getmaxy( getWindowHandle() ) - 1;
  
  /* Clear Message Buffer */
  move( mbRow, 0 );
  clrtoeol();
  refresh();
}

/* Minibuffer IO */
void miniBufferGetInput( const char *msg ) {

  int c;                        /* Input Char */

  int mbRow = getmaxy( getWindowHandle() ) - 1;
  
  /* Print Input Message */
  mvaddstr( mbRow, 0, msg );
  refresh();

  /* Read Inputs */
  int col = strlen( msg);
  int i = 0;
  
  while((( c = readKey()) != '\r' ) &&
        ( i < MINIBUFFSIZE - 1 )) {

    mvaddch( mbRow, col+i, c );
    MINIBUFFER[i++] = c;
    refresh();
  }

  MINIBUFFER[i] = '\0';                /* NULL Terminate String */
  
  move( mbRow, 0 );
  clrtoeol();
}


/* Get a New Filename */
void miniBufferGetFilename( char *fn, int fnLength ) {

  int i, nameLen;
  
  char *newFileName = NULL;
  char message[ fnLength + 14 ];	/* User Message */
  
  int yMax = getmaxx( getWindowHandle() ); /* Size of Curses Window */

  
  /* End Curses & Clear garbage off of terminal */
  endwin();
  for( i=0; i<yMax; i++ ) printf( "\n" );
  
  /* Create a message for user */
  printf( ">>>>> Enter a filename for buffer <<<<<\n\n" );
  printf( "Press ENTER to accept default name.\n" );
  printf( "Uses EMACS keybindings and TAB for autocompletion.\n\n" );

  snprintf( message, fnLength + 14, "filename [%s] : ", fn );

  /* Get the new filename */
  if(( newFileName = readline( message )) == NULL )
    die( "miniBufferGetFilename: readline failed" );

  nameLen = strlen( newFileName );
  
  /* Save new non-default filename */
  if( nameLen  > 0 ) {
    
    if( newFileName[ nameLen - 1 ] == ' ' )
      newFileName[ nameLen - 1 ] = '\0';
      
    /* Set global FILENAME */
    strncpy( fn, newFileName, strlen( newFileName ) + 1 );
  }

  free( newFileName );			

  initializeTerminal();			/* Restart curses */

  return;
}

/* Read Integer from Minibuffer */
int miniBufferGetPosInteger( const char *msg ) {

  miniBufferGetInput( msg );

  return atoi( MINIBUFFER );
}


/* Get Y/N Answer from Minibuffer */
bool miniBufferGetYN( const char *msg ) {

  miniBufferGetInput( msg );

  if( MINIBUFFER[0] == 'y' || MINIBUFFER[0] == 'Y' )
    return true;

  if( MINIBUFFER[0] != 'n' && MINIBUFFER[0] != 'N' )
    return miniBufferGetYN( msg );

  return false;
}
