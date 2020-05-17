/***
==========================================================================================
                      _              _         _____    _ _ _
                     / \   _ __   __| |_   _  | ____|__| (_) |_
                    / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
                   / ___ \| | | | (_| | |_| | | |__| (_| | | |_
                  /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.4-beta
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
#define _POSIX_C_SOURCE 200809L		     /* getline() is POSIX */

#include <stdbool.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ae.h"
#include "buffer.h"
#include "minibuffer.h"
#include "pointMarkRegion.h"
#include "files.h"
#include "state.h"
#include "edit.h"
#include "window.h"

/* Module Constants */
#define MXRWS 512			     /* Initial Buffer Size = 512 Rows */

/* Module Private Data */
static row_t **BUFFER = NULL;		     /* File Buffer */
static int MAXROWS  = MXRWS;		     /* MAX Number of Buffer Lines */
static int NUMROWS  = 0;		     /* Num Rows in Text Buffer */


/*****************************************************************************************
				       BUFFER ROWS
*****************************************************************************************/

/* Update the Number of Lines in Buffer File */
void setBufferNumRows( int x ) {

  NUMROWS = x;
}

/* Get Number of Rows In Buffer File */
int getBufferNumRows( void ) {

  return NUMROWS;
}

/* Get BUFFER Row Index for Line With POINT */
int getBufferRow( void ) {

  return getPointY() + getRowOffset();
}
/* Get BUFFER Col Index for Line With POINT */
int getBufferCol( void ) {

  return getPointX() + getColOffset();
}

/*****************************************************************************************
				       TAB HANDLING
*****************************************************************************************/

/* Convert Tabs to Spaces */
static char *removeTabs( char *line ) {

  size_t i;

  size_t countTabs = 0;
  size_t len = strlen( line );
  
  /* Count the Number of Tabs */
  for( i = 0; i<len; i++ ) {
    if( line[i] == '\t' ) {
      countTabs++;
    }
  }

  /* Return if No Tabs Found */
  if( countTabs == 0 ) return line;

  /* Create Space for 'Newline' Without Tabs */
  char *newline = malloc(( sizeof( char ) *
                           ( len + ( countTabs * 7 ))) + 1 );
  size_t j;
  size_t index = 0;

  /* Copy Line to Newline With Spaces */
  for( i = 0; i<len; i++ ) {

    if( line[i] == '\t' ) {        /* Tabs Found */
      for( j = 0; j<8; j++ ) {
        newline[index] = ' ';
        index++;
      }
    }
    else {                        /* Not a Tab */
      newline[index] = line[i];
      index++;
    }
  }

  newline[index] = '\0';
  free( line );                        /* Delete Old Line */

  return newline;		       /* Save New Line */
}

/*****************************************************************************************
				    BUFFER MANAGEMENT
 *****************************************************************************************/

/* Setup Buffer Data Structure */
void initializeBuffer( void ) {

  /* Reserve Heap Space for Buffer */
  if(( BUFFER = malloc( sizeof( row_t * ) * MXRWS )) == NULL )
    die( "initializeData: BUFFER malloc failed" );
}


/* ! DEPRACATED - Bad Practice */
/* Return Pointer to Text BUFFER */
buff_t getBufferHandle( void ) {

  return BUFFER;
}


/* Open AE on an Empty Buffer */
void openEmptyBuffer( enum _bn bn ) {


  BUFFER[0] = malloc( sizeof( row_t ));
  BUFFER[0]->txt = malloc( sizeof( char ) * 2 );
  BUFFER[0]->txt[0] = '\n';
  BUFFER[0]->txt[1] = '\0';
  BUFFER[0]->len    = 1;
  BUFFER[0]->lPtr   = 0;
  BUFFER[0]->rPtr   = 0;
  BUFFER[0]->editP  = false;

  if( bn == DEFAULT )
    setDefaultFilename();

  setBufferNumRows( 1 );
}

/* Read A Text File from Disk */
void readBufferFile( char * fn ) {

  int i = 0;
  FILE *fp = NULL;
  
  /* Check to See if File Exists and Readable */
  if( access( fn, R_OK | F_OK ) == -1 ) {

    miniBufferMessage( "Filename doesn't exist. Creating buffer for new file." );
    openEmptyBuffer( UNAMED );
    return;
  }
  
  /* Open File for Editing */
  if(( fp = fopen( fn, "r" )) == NULL ) {
    die( "openBuffer: fopen failed." );
  }

  /* Read File Rows into BUFFER */
  while( true ) {

    /* Check Buffer Size */
    if( i == MAXROWS ) {        
      doubleBufferSize();
    }

    /* Reserve Heap Space for Text Row  */
    BUFFER[i] = malloc( sizeof( row_t ));
    BUFFER[i]->len   = 0;
    BUFFER[i]->lPtr  = 0;
    BUFFER[i]->rPtr  = 0;
    BUFFER[i]->editP = false;
    BUFFER[i]->txt   = NULL;

    /* Read Next Text Row */
    if( getline( &BUFFER[i]->txt, &BUFFER[i]->len, fp ) == ERR ) {
      free( BUFFER[i]->txt );		     /* Free Unused i'th BUFFER */
      free( BUFFER[i] );		     
      break;
    }

    /* Clean-up Tabs and Set Length */
    BUFFER[i]->txt = removeTabs( BUFFER[i]->txt );
    BUFFER[i]->len = strlen( BUFFER[i]->txt );

    i++;                        /* Line Counter */
  }
  
  setBufferNumRows( i );

  fclose(fp);
}


/* Save Buffer Lines */
void saveBuffer() {

  int row;
  char msgBuffer[ 128 ];

  FILE *fp = NULL;
  int numRows = getBufferNumRows();
  
  /* Open File for Editing */
  if(( fp = fopen( getBufferFilename(), "w" )) == NULL ) {
    die( "saveBuffer: fopen failed." );
  }

  for( row = 0; row<numRows; row++ ) {
    fprintf( fp, "%s", BUFFER[row]->txt );
  }

  fclose( fp );

  setStatusFlagOriginal();

  snprintf( msgBuffer, 128, "Wrote %d lines to %s", row, getBufferFilename() );
  miniBufferMessage( msgBuffer );
}


/* Save Buffer Lines */
void saveBufferNewName( void ) {

  int row;
  char msgBuffer[ 128 ];
  
  FILE *fp = NULL;
  char *fn = getBufferFilename();

  int numRows = getBufferNumRows();

  
  /* Get Filename to Write */
  miniBufferGetFilename();

  /* Open File for Editing */
  if(( fp = fopen( fn, "w" )) == NULL ) {
    die( "saveBufferNewName: fopen failed." );
  }

  for( row = 0; row<numRows; row++ ) {
    fprintf( fp, "%s", BUFFER[row]->txt );
  }

  fclose( fp );

  setStatusFlagOriginal();

  snprintf( msgBuffer, 128, "Wrote %d lines to %s", row, getBufferFilename() );
  miniBufferMessage( msgBuffer );
}


/* Double Buffer Size */
void doubleBufferSize( void ) {
  
  int newMaxRows = MAXROWS * 2;        /* Double Buffer Size */
    
  if(( BUFFER = realloc( BUFFER, newMaxRows * sizeof( row_t *))) == NULL )
    die( "doubleBufferSize: realloc failed" );

  MAXROWS = newMaxRows;
}


/* Adjust Buffer Size, If Needed */
bool bufferFullP( void ) {

  if( getBufferNumRows() == MAXROWS ) {        
    return true;
  }

  return false;
}


/* Close Text Buffer */
void closeBuffer( void ) {

  int i;

  int numRows = getBufferNumRows();
  
  for( i=0; i<numRows; i++ ) {
    free( BUFFER[i]->txt );
    free( BUFFER[i] );
  }

  free( BUFFER );
  BUFFER = (buff_t)NULL;

  MAXROWS    = MXRWS;
  setBufferNumRows( 0 );
  setPointX( 0 );
  setPointY( 0 );
  setMarkX( -1 );
  setMarkY( -1 );
  setRowOffset( 0 );
  setColOffset( 0 );
  setEditBufferIndex( 0 );
  setRegionActive( false );
  setStatusFlagOriginal();

  clear();
}

/***
    Close Current Buffer 
    and Open Empty Buffer
 ***/

void killBuffer( void ) {

  closeBuffer();

  initializeBuffer();
  
  openEmptyBuffer( DEFAULT );
  
  clear();

}

/*****************************************************************************************
				    BUFFER PROPERTIES
*****************************************************************************************/
int getBufferLineLen( int row ) {

  return BUFFER[row]->len;
}

char getBufferChar( int row, int col ) {

  return BUFFER[ row ]->txt[ col ];
}

bool bufferLineModifiedP( int row ) {

  return ( BUFFER[row]->lPtr != BUFFER[row]->rPtr );
}

bool bufferRowEditedP( int row ) {

  return BUFFER[row]->editP;
}

void setBufferRowEdited( int row, bool pred ) {

  BUFFER[row]->editP = pred;
}

int getBufferGapRightIndex( int row ) {

  return BUFFER[row]->rPtr;
}

int getBufferGapLeftIndex( int row ) {

  return BUFFER[row]->lPtr;
}

int getBufferGapSize( int row ) {
  
  return BUFFER[row]->rPtr - BUFFER[row]->lPtr;
}

void setBufferGapPtrs( int row, int left, int right ) {

  BUFFER[row]->lPtr = left;
  BUFFER[row]->rPtr = right;
}

void increaseBufferGap( int row ) {

  if( BUFFER[row]->rPtr < BUFFER[row]->len - 1 )
    BUFFER[row]->rPtr++;
}

/*****************************************************************************************
				   MODIFY BUFFER LINES
*****************************************************************************************/

/* Free row_t */
void freeBufferLine( int row ) {

  int nRows = getBufferNumRows();
  
  free( BUFFER[row]->txt );
  free( BUFFER[row] );
    
  for( int i=row; i<nRows-1; i++ ) {

    BUFFER[i] = BUFFER[i+1];
  }

  setBufferNumRows( --nRows );
}

/* Delete Point (thisRow,thisCol) to End of Line */
void freeBufferPointToEOL( int thisRow, int thisCol ) {

  /* Create Heap Space for New 'Trimmed' String */
  char *tmp = malloc( sizeof( char ) * ( thisCol + 2 ));
  if( thisCol > 0 )
    strncpy( tmp, BUFFER[thisRow]->txt, thisCol );
  free( BUFFER[thisRow]->txt );

  /* Fix Up thisRow_t For This ThisRow */
  BUFFER[thisRow]->txt = tmp;
  BUFFER[thisRow]->len = thisCol + 1;
  BUFFER[thisRow]->txt[BUFFER[thisRow]->len-1] = '\n';
  BUFFER[thisRow]->txt[BUFFER[thisRow]->len] = '\0';

  return;
}


/* Replace Text String in Buffer Line */
void replaceBufferLineText( int row, int newLen, char *newTxt ) {

  free( BUFFER[row]->txt );
  BUFFER[row]->txt  = newTxt;
  BUFFER[row]->len  = newLen;
  BUFFER[row]->lPtr = 0;
  BUFFER[row]->rPtr = 0;

  return;
}

/* Open a New Line */
void openLine( void ) {

  int PtY = getPointY();
  int PtX = getPointX();

  int thisRow = getRowOffset() + getPointY();
  
  /* Double Buffer, If Full */
  if( bufferFullP() )
    doubleBufferSize();

  /* Move Lines Down */
  int i;

  for( i=getBufferNumRows(); i>thisRow+1; i-- ) {
    BUFFER[i] = BUFFER[i-1];
  }

  /* Create New Line */
  BUFFER[i] = malloc( sizeof( row_t ));
  BUFFER[i]->len = BUFFER[thisRow]->len - PtX;
  BUFFER[i]->txt = malloc( sizeof( char ) * ( BUFFER[i]->len + 1 ));
  BUFFER[i]->lPtr   = 0;
  BUFFER[i]->rPtr   = 0;
  BUFFER[i]->editP  = false;


  /* Copy Text Into New Line */
  if( strncpy( BUFFER[i]->txt, 
               BUFFER[thisRow]->txt + PtX,
               BUFFER[i]->len ) == NULL ) die( "openLine: strncpy failed." );

  BUFFER[i]->txt[BUFFER[i]->len] = '\0';


  /* Heap Space for Trimmed String */
  char *tmp = malloc( sizeof( char ) * ( PtX + 2 ));
  if( PtX > 0 )
    strncpy( tmp, BUFFER[thisRow]->txt, PtX );
  free( BUFFER[thisRow]->txt );

  /* Fix Up row_t For This Row */
  BUFFER[thisRow]->txt = tmp;
  BUFFER[thisRow]->len = PtX + 1;
  BUFFER[thisRow]->txt[BUFFER[thisRow]->len-1] = '\n';
  BUFFER[thisRow]->txt[BUFFER[thisRow]->len] = '\0';

  clrtoeol();
  
  /* Move Point */
  setPointX( 0 );
  if( PtY == getScreenRows() )
    setRowOffset( getRowOffset() + 1 );
  else
    setPointY( ++PtY );

  setBufferNumRows( getBufferNumRows() + 1 ); /* Increment Num Lines */
}

/* Combine 'This' Text Line With Prior Text Line */
void combineLineWithPrior( void ) {

  int thisRow = getRowOffset() + getPointY();
  
  if( thisRow == 0 )
    return;

  char *tmp;				     /* New Text String */
        
  /* Allocate Memory for Combined String */
  int preLineLen = BUFFER[thisRow-1]->len - 1;
  int nxtLineLen = BUFFER[thisRow]->len;
  int size       = preLineLen + nxtLineLen + 1;

  if(( tmp = malloc( sizeof( char ) * size )) == NULL )
    die( "BUFFER: tmp malloc failed" );

  /* Copy Old/New Text Into New Line */
  strncpy( tmp, 
           BUFFER[thisRow-1]->txt,
           preLineLen );
  strncpy( tmp + preLineLen,
           BUFFER[thisRow]->txt,
           nxtLineLen );
  BUFFER[thisRow-1]->len = size - 1;
        
  /* Free Old Memory and Set Pointer */
  free( BUFFER[thisRow-1]->txt );
  BUFFER[thisRow-1]->txt = tmp;

  /* Destroy Next Line */
  freeBufferLine( thisRow );

  setPointY( getPointY() - 1 );
  setPointX( preLineLen );
}


/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
