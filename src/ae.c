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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ae.h"
#include "keyPress.h"
#include "minibuffer.h"
#include "pointMarkRegion.h"
#include "render.h"
#include "buffer.h"
#include "window.h"
#include "navigation.h"
#include "files.h"

/* Macros */
#define CTRL_KEY(k) ((k) & 0x1f)
#define ALT_KEY 27
#define thisRow() (ROWOFFSET + getPointY())
#define thisCol() (COLOFFSET + getPointX())
#define screenRows() (getWinNumRows() - 3)


/* Buffer Status Flag */
enum _sf { ORIGINAL, MODIFIED, READONLY };
const char _sfname[3][9] = { "ORIGINAL", "MODIFIED", "READONLY" };


/* Global Data */
int NUMROWS    =  0;			/* Num Rows in Text Buffer */
int ROWOFFSET  =  0;			/* Buffer Index of Top Row */
int COLOFFSET  =  0;			/* Buffer Index of First Col */
enum _sf STATUSFLAG \
               = ORIGINAL;		/* Is Buffer Modified? */
static char EDITBUFFER[64];		/* Edit Buffer For Text Input */
static int  EBINDEX   =  0;		/* Edit Buffer Index */


/*******************************************************************************
			      HANDLE ERRORS
*******************************************************************************/

/* Print Error Message and Exit */
void die( const char *s ) {

  perror(s);
  sleep(3);
  closeEditor();
  
  exit(EXIT_FAILURE);
}

/*******************************************************************************
                          BUFFER STATUS
*******************************************************************************/


int getEditBufferIndex() {

  return EBINDEX;
}

char getEditBufferChar(int i) {

  return EDITBUFFER[i];
}

void setEditBufferIndex( int x ) {

  EBINDEX = x;
}

void setStatusFlagOriginal( void ) {

  STATUSFLAG = ORIGINAL;
}

/*******************************************************************************
				    UPDATE LINE EDITS
*******************************************************************************/

/* Incorporate Edits Into Row Structure */
void updateLine() {

  int i   = 0;
  int col = 0;

  buff_t buff = getBufferHandle();
  
  int delta = EBINDEX - ( buff[thisRow()]->rPtr -
                          buff[thisRow()]->lPtr );

  int newLen = buff[thisRow()]->len + delta;
    
  char *tmp;                        /* New Text Row */

  if(( tmp = malloc(( sizeof( char ) * newLen ) + 1 )) == NULL )
    die( "updateLine: tmp malloc failed" );

  /* Copy Non-deleted Chars */
  for( i = 0; i<(int)buff[thisRow()]->lPtr; i++ ) {
    tmp[col] = buff[thisRow()]->txt[i];
    col++;
  }

  /* Add Chars from Edit Buffer */
  for( i = 0; i<EBINDEX; i++ ) {
    tmp[col] = EDITBUFFER[i];
    col++;
  }

  /* Add Rest of Chars */
  for( i = buff[thisRow()]->rPtr; i<(int)buff[thisRow()]->len; i++ ) {
    tmp[col] = buff[thisRow()]->txt[i];
    col++;
  }

  tmp[newLen] = '\0';                /* NULL Terminate New String */

  free( buff[thisRow()]->txt );
  buff[thisRow()]->txt  = tmp;
  buff[thisRow()]->len  = newLen;
  buff[thisRow()]->lPtr = 0;
  buff[thisRow()]->rPtr = 0;

  EBINDEX = 0;
}


/*******************************************************************************
                             EDITOR STATE
*******************************************************************************/

/* Cursor Movement Functions */
void updateNavigationState() {

  if( bufferRowEditedP( thisRow() ))
    updateLine();

  setBufferRowEdited( thisRow(), false );
  
  miniBufferClear();
}

/* Edit Line */
void updateEditState() {

  setBufferRowEdited( thisRow(), true );
  STATUSFLAG = MODIFIED;
}

/* Get/Set Row/Col Offset for Nav Functions */
int getRowOffset( void ) {
  return ROWOFFSET;
}
int getColOffset( void ) {
  return COLOFFSET;
}
void setRowOffset( int ro ) {
  ROWOFFSET = ro;
}
void setColOffset( int co ) {
  COLOFFSET = co;
}


/* Update the Number of Lines in Buffer File */
void setNumRows( int x ) {

  NUMROWS = x;
}

int getBufferNumRows( void ) {

  return NUMROWS;
}

/*******************************************************************************
                             INSERT CHARS
*******************************************************************************/

/* Insert User Typed Chars */
void selfInsert( int c ) {

  int PtX     = getPointX();

  /* Save Prior Unsaved Changes to Text Row */ 
  if( bufferRowEditedP( thisRow() ))
    updateNavigationState();

  /* Text Insertion Now Begins at POINT_X */  
  if( !bufferRowEditedP( thisRow() )) {
    setEditBufferPtrs( thisRow(), PtX, PtX );
    updateEditState();
  }

  EDITBUFFER[EBINDEX++] = c;
  setPointX( ++PtX );
}


/*******************************************************************************
                           LINE MANAGEMENT
*******************************************************************************/

/* Kill Line at Point */
void killLine() {

  int PtX = getPointX();
  buff_t buff = getBufferHandle();
  
  /* Line Empty - Delete it */
  if( getBufferLineLen( thisRow() ) == 1 ) {

    if( thisRow() == NUMROWS - 1 ) return; /* Cant Delete if Nothing Follows */

    freeBufferLine( thisRow() );
  }

  /* Delete from POINT to EOL */
  else {                        

    /* Heap Space for Trimmed String */
    char *tmp = malloc( sizeof( char ) * ( PtX + 2 ));
    if( PtX > 0 )
      strncpy( tmp, buff[thisRow()]->txt, PtX );
    free( buff[thisRow()]->txt );

    /* Fix Up row_t For This Row */
    buff[thisRow()]->txt = tmp;
    buff[thisRow()]->len = PtX + 1;
    buff[thisRow()]->txt[buff[thisRow()]->len-1] = '\n';
    buff[thisRow()]->txt[buff[thisRow()]->len] = '\0';

    clrtoeol();
  }
}

/* Indent Newline to Smart Location */
void autoIndent() {

  int len;				/* Len of 'last' row */
  
  int i = 0;				/* Col Index */
  int lastRow = thisRow() - 1;
  buff_t buff = getBufferHandle();

  /* Skip if Middle of Line or Top of Buffer */
  if(( getPointX() != 0 ) || ( thisRow() == 0 ))
    return;

  len = buff[lastRow]->len;

  /* Find Tab Col Prior Row */
  while( buff[lastRow]->txt[i] == ' ' )
    i++;

  /* Skip Blank Lines */
  if( i == len - 1 )
    return;
  
  /* Insert Spaces Until Tab Mark */
  while(( i < len ) && ( i > 0 )) {

    selfInsert( ' ' );
    i--;
  }
}


/* Open a New Line */
void openLine() {

  int PtY = getPointY();
  int PtX = getPointX();

  buff_t buff = getBufferHandle();
  
  /* Double Buffer, If Full */
  if( bufferFullP() ) {
    doubleBufferSize();
    buff = getBufferHandle();		/* New Double-Sized BUFFER */
  }

  /* Move Lines Down */
  int i;

  for( i=NUMROWS; i>thisRow()+1; i-- ) {
    buff[i] = buff[i-1];
  }

  /* Create New Line */
  buff[i] = malloc( sizeof( row_t ));
  buff[i]->len = buff[thisRow()]->len - PtX;
  buff[i]->txt = malloc( sizeof( char ) * ( buff[i]->len + 1 ));
  buff[i]->lPtr   = 0;
  buff[i]->rPtr   = 0;
  buff[i]->editP  = false;


  /* Copy Text Into New Line */
  if( strncpy( buff[i]->txt, 
               buff[thisRow()]->txt + PtX,
               buff[i]->len ) == NULL ) die( "openLine: strncpy failed." );

  buff[i]->txt[buff[i]->len] = '\0';


  /* Heap Space for Trimmed String */
  char *tmp = malloc( sizeof( char ) * ( PtX + 2 ));
  if( PtX > 0 )
    strncpy( tmp, buff[thisRow()]->txt, PtX );
  free( buff[thisRow()]->txt );

  /* Fix Up row_t For This Row */
  buff[thisRow()]->txt = tmp;
  buff[thisRow()]->len = PtX + 1;
  buff[thisRow()]->txt[buff[thisRow()]->len-1] = '\n';
  buff[thisRow()]->txt[buff[thisRow()]->len] = '\0';

  clrtoeol();
  
  /* Move Point */
  setPointX( 0 );
  if( PtY == screenRows() )
    ROWOFFSET++;
  else
    setPointY( ++PtY );

  NUMROWS++;                        /* Increment Num Lines */
}

/*******************************************************************************
                             DELETE CHARS
*******************************************************************************/

void killWord() {

  int currPointX = getPointX();		/* Save Current Point */
  
  forwardWord();			/* Find End Next Word */

  if( getPointX() == currPointX )	/* No Word to Kill */
    return;

  /* Mark Word for Deletion and Restore Point */
  setEditBufferPtrs( thisRow(), currPointX, getPointX() );
  setPointX( currPointX );
  updateEditState();
  updateNavigationState();
}


void combineLineWithPrior() {

  if( thisRow() == 0 )
    return;

  buff_t buff = getBufferHandle();
  
  char *tmp;
        
  /* Allocate Memory for Combined String */
  int preLineLen = buff[thisRow()-1]->len - 1;
  int nxtLineLen = buff[thisRow()]->len;
  int size       = preLineLen + nxtLineLen + 1;

  if(( tmp = malloc( sizeof( char ) * size )) == NULL )
    die( "backspace: tmp malloc failed" );

  /* Copy Old/New Text Into New Line */
  strncpy( tmp, 
           buff[thisRow()-1]->txt,
           preLineLen );
  strncpy( tmp + preLineLen,
           buff[thisRow()]->txt,
           nxtLineLen );
  buff[thisRow()-1]->len = size - 1;
        
  /* Free Old Memory and Set Pointer */
  free( buff[thisRow()-1]->txt );
  buff[thisRow()-1]->txt = tmp;

  /* Destroy Next Line */
  freeBufferLine( thisRow() );

  setPointY( getPointY() - 1 );
  setPointX( preLineLen );
}

void backspace( void ) {

  int PtX     = getPointX();
  buff_t buff = getBufferHandle();
  
  /* Continue Editing This Line? */
  if( buff[thisRow()]->editP ) {

    /* If Deleting Chars After Inserting Chars? */
    /* Then, Save Insertions Befor Making Deletions */ 
    if( buff[thisRow()]->lPtr == buff[thisRow()]->rPtr ) {
      updateLine();
      buff[thisRow()]->lPtr = PtX;
      buff[thisRow()]->rPtr = PtX;
    }

    /* Continue Deleting Chars to BOL */
    if( PtX > 0 ) {
      buff[thisRow()]->lPtr--;
      setPointX( --PtX );
    }

    /* At BOL, Combine With Prior Line */
    else {
      updateNavigationState();
      combineLineWithPrior();
    }
  }

  /* Begin *New* Edits to This Line */
  else {

    if( PtX > 0 ) {
      buff[thisRow()]->lPtr = PtX - 1;
      buff[thisRow()]->rPtr = PtX;
      updateEditState();
      setPointX( --PtX );
    }

    else {
      combineLineWithPrior();
    }
  }
}


/*******************************************************************************
                         PROCESS KEY PRESSES
*******************************************************************************/

/* Meta Menu */
void metaMenu() {

  int c = readKey();

  switch(c) {

    /* Buffer Navigation */
  case 'f':                        /* Forward Word */
    updateNavigationState();
    forwardWord();
    break;

  case 'b':                        /* Backward Word */
    updateNavigationState();
    backwardWord();
    break;

  case 'd':                        /* Kill Forward Word */
    killWord();
    updateEditState();
    break;

  case 'g':                        /* Alias for c-j */
    jumpToLine();
    break;
    
  case 'v':                        /* Forward Word */
    updateNavigationState();
    pageUp();
    break;

  case '<':                              /* Top of Buffer */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    COLOFFSET = 0;
    ROWOFFSET = 0;
    break;

  case '>':                           /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  }  
}


/* eXtension Menu */
void eXtensionMenu() {

  int c = readKey();

  switch(c) {

  case CTRL_KEY('c'):                 /* Close Editor */
    if( STATUSFLAG == MODIFIED )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    closeBuffer();
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case 'k':                           /* Kill Buffer */
    if( STATUSFLAG == MODIFIED )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    break;
    
  case CTRL_KEY('s'):                /* Save Buffer */
    if( STATUSFLAG == MODIFIED ) {
	updateNavigationState();
	saveBuffer();
      }
    else {
      miniBufferMessage( "Buffer not Modified" );
    }
    break;

  case CTRL_KEY('f'):
  case CTRL_KEY('v'):
    /* Close Old Buffer */
    if( STATUSFLAG == MODIFIED )
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
    if( STATUSFLAG == MODIFIED ) {
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
       ( getPointY() > ( ROWOFFSET + screenRows())))
      centerLine();
    break;
  }
}
  

/* Process Keypresses */
void processKeypress() {

  int c       = readKey();
  buff_t buff = getBufferHandle();
  
  switch(c) {
    
    /* Meta Key */
  case ALT_KEY:
    metaMenu();
    break;

    /* CTRL Key */
  case CTRL_KEY('x'):
    eXtensionMenu();
    break;

    /* Keyboard Quit */
  case CTRL_KEY('g'):
    setRegionActive( false );
    setMarkY( -1 );
    setMarkX( -1 );
    break;

    /* Function Keys */
  case KEY_F(1):			/* Help */

    endwin();				/* Dumb Help */
    system( "cat USERGUIDE.md | more" );
    initializeTerminal();
    break;

  case KEY_F(10):			/* Exit */
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case '\t':
    miniBufferMessage( "Auto Indent" );
    autoIndent();
    break;
    
    /* Cursor Movement */
  case KEY_HOME:			/* Home */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    COLOFFSET = 0;
    ROWOFFSET = 0;
    break;
  case CTRL_KEY('l'):                /* Center Line */
    updateNavigationState();
    centerLine();
    break;
  case CTRL_KEY('b'):                /* Point Back */
  case KEY_LEFT:                
    updateNavigationState();
    pointBackward();
    break;
  case CTRL_KEY('a'):                /* Point BOL */
    updateNavigationState();
    setPointX( 0 );
    COLOFFSET = 0;
    break;
  case CTRL_KEY('f'):                /* Point Forward */
  case KEY_RIGHT:
    updateNavigationState();
    pointForward();
    break;
  case CTRL_KEY('e'):                /* Point EOL */
    updateNavigationState();
    pointToEndLine();
    break;
  case CTRL_KEY('j'):                /* Jump to Linenum */
    updateNavigationState();
    jumpToLine();
    break;
  case CTRL_KEY('p'):                /* Prior Line */
  case KEY_UP:
    updateNavigationState();
    priorLine();
    break;
  case CTRL_KEY('n'):                /* Next Line */
  case KEY_DOWN:
    updateNavigationState();
    nextLine();
    break;
  case KEY_PPAGE:                    /* Page Up */
    updateNavigationState();
    pageUp();
    break;
  case KEY_END:                      /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  case KEY_NPAGE:                    /* Page Down */
  case CTRL_KEY('v'):
    updateNavigationState();
    pageDown();
    break;

    /* Create a Newline */
  case '\r':                        /* Enter Key */
    updateNavigationState();
    openLine();
    updateEditState();
    break;

    /* Point/Mark */
  case CTRL_KEY(' '):                /* Set Mark */
    setMarkX( thisCol() );
    setMarkY( thisRow() );
    setRegionActive( true );
    miniBufferMessage( "Mark Set" );
    break;

    /* Edit Text */
  case CTRL_KEY('d'):                /* Delete Char */
  case KEY_DC:

    /* Continue Editing This Line? */
    if( buff[thisRow()]->editP ) {

      /* Start Deleting Chars After Inserting Chars? */
      if( buff[thisRow()]->lPtr == buff[thisRow()]->rPtr ) {
        updateLine();
        buff[thisRow()]->lPtr = getPointX();
        buff[thisRow()]->rPtr = getPointX();
      }
        
      /* Continue Deleting Chars Up to EOL  */
      if( buff[thisRow()]->rPtr < buff[thisRow()]->len )
        buff[thisRow()]->rPtr++;
    }
    
    /* *New* Edit To This Line? */
    else {
      int PtX = getPointX();
      if( (size_t)PtX < buff[thisRow()]->len - 1 ) {
        buff[thisRow()]->lPtr = PtX;
        buff[thisRow()]->rPtr = PtX+1;
        updateEditState();
      }
    }
    break;

  case CTRL_KEY('h'):                /* Backspace */
  case KEY_BACKSPACE:
    backspace();
    break;
  case CTRL_KEY('k'):                   /* Kill Line */
    updateEditState();
    killLine();
    updateNavigationState();
    break;

  case CTRL_KEY('w'):                   /* Kill Region  */
    killRegion();
    updateEditState();
    break;
    
    /* Handle Signals */
  case KEY_RESIZE:                /* Window Resized */
    break;

  default:                        /* Self Insert */
    if( isprint( c ))
      selfInsert( c );
    break;
  }
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
  renderText( getBufferFilename(), _sfname[ STATUSFLAG ], getWinNumRows()-2, getWinNumCols(), NUMROWS );
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
    renderText( getBufferFilename(), _sfname[ STATUSFLAG ], getWinNumRows()-2, getWinNumCols(), NUMROWS );
    processKeypress();
  } 

  /* Shutdown */
  closeEditor();
  return EXIT_SUCCESS;
} 


/***
    Local Variables:
    mode: c
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
