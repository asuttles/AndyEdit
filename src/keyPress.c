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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <curses.h>

#include "ae.h"
#include "window.h"
#include "navigation.h"
#include "pointMarkRegion.h"
#include "files.h"
#include "minibuffer.h"
#include "state.h"
#include "edit.h"
#include "buffer.h"
#include "window.h"
#include "keyPress.h"

/* Macros */
#define thisRow() (getRowOffset() + getPointY())
#define thisCol() (getColOffset() + getPointX())

/* Module Private Function Declarations */
static void _handleKeypress( int );

/*****************************************************************************************
				      READ KEY INPUT
*****************************************************************************************/

/* Read Keypresses */
int readKey() {

  int c;                        /* 'Char' or Flags */

  /* wgetch handles SIGWINCH */
  while(( c = wgetch( getWindowHandle() )) == ERR ) {

    /* Handle Timeouts */
    refresh();
  }
  
  return c;
}


void _universalArgument( void ) {

  int times = miniBufferGetUniversalArg();
  int c     = miniBufferGetUniversalChr();

  if( c == CTRL_KEY('g') )
    return;
  
  while( times-- > 0 ) {
    _handleKeypress( c );
  }
}

/*****************************************************************************************
			       HANDLE EXTENSION MENU INPUTS
*****************************************************************************************/

static void _rectangleMenu( void ) {

  int c = readKey();

  switch(c) {
    
  case 'k':				     /* Kill Rectangle */
    killRectangle();
    break;

  case 't':				     /* Insert Rectangle */
    rectangleInsert();
    break;

  default:
    miniBufferMessage( "unrecognized keypress" );
    break;
  }

  return;
}

/*****************************************************************************************
			       HANDLE EXTENSION MENU INPUTS
*****************************************************************************************/

/* eXtension Menu */
static void _eXtensionMenu( void ) {

  int c = readKey();

  switch(c) {

  case CTRL_KEY('c'):			     /* Close Editor */
    if( statusFlagModifiedP() )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    closeBuffer();
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case 'k':				     /* Kill Buffer */
    if( statusFlagModifiedP() )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    break;

  case 'r':				     /* Rectangle Operations */
    _rectangleMenu();
    break;
    
  case CTRL_KEY('s'):			     /* Save Buffer */
    if( statusFlagModifiedP() ) {
	updateNavigationState();
	saveBuffer();
      }
    else {
      miniBufferMessage( "Buffer not Modified" );
    }
    break;

  case CTRL_KEY('f'):			     /* Open Buffer File */
  case CTRL_KEY('v'):
    /* Close Old Buffer */
    if( statusFlagModifiedP() )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    /* Open New Buffer */
    if( openFile() )
      readBufferFile( getBufferFilename() );
    break;
    
  case CTRL_KEY('w'):                /* Save Buffer As */
    if( statusFlagModifiedP() ) {
	updateNavigationState();
	saveBufferNewName();
      }
    else {
      miniBufferMessage( "Buffer not Modified" );
    }
    break;

  case CTRL_KEY('x'):                /* Swap Point/Mark */
    updateNavigationState();
    swapPointAndMark();
    if(( getPointY() < 0 ) ||
       ( getPointY() > ( getRowOffset() + getScreenRows())))
      centerLine();
    break;
  }
}


/*****************************************************************************************
				 HANDLE META MENU INPUTS
*****************************************************************************************/

/* Meta Menu */
void metaMenu( void ) {

  int c = readKey();

  switch(c) {

    /* Buffer Navigation */
  case 'f':				     /* Forward Word */
    updateNavigationState();
    forwardWord();
    break;

  case 'b':				     /* Backward Word */
    updateNavigationState();
    backwardWord();
    break;

  case 'c':				     /* Capitalize Word */
    capitalizeWord();
    break;

  case 'd':				     /* Kill Forward Word */
    killWord();
    updateEditState();
    break;

  case 'g':				     /* Alias for c-j */
    jumpToLine();
    break;

  case 'l':				     /* Downcase Word */
    downcaseWord();
    break;
    
  case 'u':				     /* Upcase Word */
    upcaseWord();
    break;
    
  case 'v':				     /* Backward Page */
    updateNavigationState();
    pageUp();
    break;

  case '<':				     /* Top of Buffer */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    setColOffset( 0 );
    setRowOffset( 0 );
    break;

  case '>':				     /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  }  
}

/*****************************************************************************************
				   PROCESS KEY PRESSES
*****************************************************************************************/

/* Process Keypresses */
static void _handleKeypress( int c ) {

  switch(c) {
    
    /* Meta Key */
  case ALT_KEY:
    metaMenu();
    break;

    /* CTRL Key */
  case CTRL_KEY('x'):
    _eXtensionMenu();
    break;

    /* Universal Argument */
  case CTRL_KEY('u'):
    _universalArgument();
    break;
    
    /* Keyboard Quit */
  case CTRL_KEY('g'):
    setRegionActive( false );
    clearSearchFlag();
    setMarkY( -1 );
    setMarkX( -1 );
    break;

    /* Function Keys */
  case KEY_F(1):			     /* Help */

    endwin();				     /* Dumb Help */
    system( "cat USERGUIDE.md | more" );
    initializeTerminal();
    break;

  case KEY_F(2):			     /* Find File */
    
    /* Close Old Buffer */
    if( statusFlagModifiedP() )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    /* Open New Buffer */
    if( openFile() )
      readBufferFile( getBufferFilename() );
    break;

    
  case KEY_F(10):			     /* Exit */
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case '\t':
    autoIndent();
    break;
    
    /* Cursor Movement */
  case KEY_HOME:			     /* Home */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    setColOffset( 0 );
    setRowOffset( 0 );
    break;
  case CTRL_KEY('l'):			     /* Center Line */
    updateNavigationState();
    centerLine();
    break;
  case CTRL_KEY('b'):			     /* Point Back */
  case KEY_LEFT:                
    updateNavigationState();
    pointBackward();
    break;
  case CTRL_KEY('a'):			     /* Point BOL */
    updateNavigationState();
    setPointX( 0 );
    setColOffset( 0 );
    break;
  case CTRL_KEY('f'):			     /* Point Forward */
  case KEY_RIGHT:
    updateNavigationState();
    pointForward();
    break;
  case CTRL_KEY('e'):			     /* Point EOL */
    updateNavigationState();
    pointToEndLine();
    break;
  case CTRL_KEY('j'):			     /* Jump to Linenum */
    updateNavigationState();
    jumpToLine();
    break;
  case CTRL_KEY('p'):			     /* Prior Line */
  case KEY_UP:
    updateNavigationState();
    priorLine();
    break;
  case CTRL_KEY('n'):			     /* Next Line */
  case KEY_DOWN:
    updateNavigationState();
    nextLine();
    break;
  case CTRL_KEY('r'):			     /* Search Up for Word */
    updateLine();
    wordSearchBackward();
    break;
  case CTRL_KEY('s'):			     /* Search Down for Word */
    updateLine();
    wordSearchForward();
    break;
  case KEY_PPAGE:			     /* Page Up */
    updateNavigationState();
    pageUp();
    break;
  case KEY_END:				     /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  case KEY_NPAGE:			     /* Page Down */
  case CTRL_KEY('v'):
    updateNavigationState();
    pageDown();
    break;

    /* Create a Newline */
  case '\r':				     /* Enter Key */
    updateNavigationState();
    openLine();
    updateEditState();
    break;

    /* Point/Mark */
  case CTRL_KEY(' '):			     /* Set Mark */
    setMarkX( thisCol() );
    setMarkY( thisRow() );
    setRegionActive( true );
    miniBufferMessage( "Mark Set" );
    break;

    /* Edit Text */
  case CTRL_KEY('d'):			     /* Delete Char */
  case KEY_DC:
    deleteChar();
    break;
  case CTRL_KEY('h'):			     /* Backspace */
  case KEY_BACKSPACE:
    backspace();
    break;
  case CTRL_KEY('k'):			     /* Kill Line */
    updateEditState();
    killLine();
    updateNavigationState();
    miniBufferMessage( "Killed Text" );
    break;

  case CTRL_KEY('y'):			     /* Yank Line */
    yankLine();
    updateNavigationState();
    miniBufferMessage( "Yanked Text" );
    break;
    
  case CTRL_KEY('w'):			     /* Kill Region  */
    killRegion();
    updateEditState();
    break;
    
    /* Handle Signals */
  case KEY_RESIZE:			     /* Window Resized */
    break;

  default:				     /* Self Insert */
    if( isprint( c ))
      selfInsert( c );
    break;
  }
}


void processKeypress( void ) {

  _handleKeypress( readKey() );    
}





/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
