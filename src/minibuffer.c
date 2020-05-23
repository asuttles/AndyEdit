/***
==========================================================================================
            _              _         _____    _ _ _
           / \   _ __   __| |_   _  | ____|__| (_) |_
          / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
         / ___ \| | | | (_| | |_| | | |__| (_| | | |_
        /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.5-beta
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
#include <curses.h>
#include <ctype.h>

#include "ae.h"
#include "keyPress.h"
#include "window.h"
#include "files.h"

char MINIBUFFER[MINIBUFFSIZE];		     /* Minibuffer Input */

static int UNIVERSALCHAR;		     /* Input to Universal Arg */

/*******************************************************************************
			      MINIBUFFER
*******************************************************************************/

/* Write Message to User */
void miniBufferMessage( const char *msg ) {

  int mbRow = getWinNumRows() - 1;
  
  /* Print Input Message */
  mvaddstr( mbRow, 0, msg );
  refresh();
}

/* Clear Minibuffer Messages */
void miniBufferClear() {
  
  int mbRow = getWinNumRows() - 1;
  
  /* Clear Message Buffer */
  move( mbRow, 0 );
  clrtoeol();
  refresh();
}

/* Minibuffer IO */
bool miniBufferGetInput( const char *msg ) {

  int c;                        /* Input Char */

  int mbRow = getWinNumRows() - 1;
  
  /* Print Input Message */
  mvaddstr( mbRow, 0, msg );
  refresh();

  /* Read Inputs */
  int strtCol = strlen( msg);
  int i = 0;
  
  while((( c = readKey()) != '\r' ) &&
        ( i < MINIBUFFSIZE - 1 )) {

    if( c == CTRL_KEY( 'g' )) return false;
	
    if( c == KEY_BACKSPACE ) {

      i--;
      move( mbRow, strtCol+i );
      clrtoeol();
    }

    else {
      mvaddch( mbRow, strtCol+i, c );
      MINIBUFFER[i++] = c;
      refresh();
    }
  }

  MINIBUFFER[i] = '\0';                /* NULL Terminate String */
  
  move( mbRow, 0 );
  clrtoeol();

  return true;
}


bool miniBufferGetSearchString( char *str, int len ) {

  char msg[MINIBUFFSIZE];
  snprintf( msg, MINIBUFFSIZE, "Search String [%s] : ", str );
  
  if( miniBufferGetInput( msg )) {

    /* Accept Default Search String */
    if( strlen( MINIBUFFER ) == 0 ) return true;

    /* Copy NEW Search String */
    strncpy( str, (char *)MINIBUFFER, (size_t)len );
    return true;
  }

  return false;  
}


/* Get a New Filename */
void miniBufferGetFilename( void ) {

  miniBufferGetInput( "Enter a New File Name: " );
  setFilename( (char *)MINIBUFFER );
  
  return;
}

/* Read Integer from Minibuffer */
int miniBufferGetPosInteger( const char *msg ) {

  miniBufferGetInput( msg );

  return atoi( MINIBUFFER );
}

/* Read Universal Argument from MiniBuffer */
int miniBufferGetUniversalArg( void ) {

  int c;				     /* Input Char */
  int val = 0;				     /* Value of univ arg */

  const char *msg = "Enter Argument (c-g to exit) : ";
  
  int mbRow = getWinNumRows() - 1;
  
  /* Print Input Message */
  mvaddstr( mbRow, 0, msg  );
  refresh();

  /* Read Inputs */
  int strtCol = strlen( msg);

  while( isdigit( c = readKey() )) {
    val = ( val * 10 ) + ( c - 48 );
    mvaddch( mbRow, ++strtCol, c );
    refresh();
  }

  miniBufferClear();
  
  UNIVERSALCHAR = c;
  return val;
}

/* Return Char Input After Universal Arg */
int miniBufferGetUniversalChr( void ) {

  return UNIVERSALCHAR;
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

/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
