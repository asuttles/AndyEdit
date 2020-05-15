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
#include <curses.h>
#include <stdbool.h>

#include "ae.h"
#include "state.h"
#include "statusBar.h"
#include "pointMarkRegion.h"
#include "buffer.h"
#include "edit.h"
#include "window.h"
#include "files.h"

#define DISPLAY_ROWS ( getWinNumRows() - 2 )

/*******************************************************************************
                             RENDER TEXT
*******************************************************************************/

/* Draw and Color the Rows of Text */
void renderText( void ) {

  int i, j;				     /* Iteration Indices */
  int row, col;				     /* Row/Col Being Processed */
  int nextRow;				     /* The Next Row to Process */
  int colMax;				     /* Last Col Index Dispalyed in Window */
  int txtLen;				     /* Length of Text to Display on Line */

  /* Row/Column Initializations */
  int colOffset = getColOffset();
  int rowOffset = getRowOffset();
  int thisRow = rowOffset + getPointY(); /* Row Containing POINT */
  int thisCol = colOffset + getPointX();

  /* File Initializations */
  int fileRows = getBufferNumRows();
  int maxCols  = getWinNumCols();

  /* Iter Across Each Row of Visible Screen */
  for( row = 0; row < DISPLAY_ROWS; row++ ) {

    nextRow = row+rowOffset;		     /* Index of Next Row to Process */

    if( nextRow < fileRows ) {		     /* Write Buffer Text */

      /* Calc How Much of the Text Row Should Be Displayed */
      txtLen = getBufferLineLen( nextRow ) - colOffset;

      /* Calculate what subset of that Text Row Fits in the Term Window */
      colMax = maxCols > txtLen ? txtLen : maxCols;

      /* Write Letter at a Time */
      col = 0;
      for( i=0; i < colMax; i++ ) {

        /* Insert EDIT BUFFER Chars */
        if( nextRow == thisRow                   &&
	    bufferRowEditedP( nextRow )          &&
	    ( getBufferGapSize( nextRow ) == 0 ) &&
            ( i == getBufferGapLeftIndex( nextRow ))) {

	  /* Highlight, If Rendering Active Region */
          if( inRegionP( nextRow, i+colOffset ))
	    attron( COLOR_PAIR( HIGHLT_BACKGROUND ));

	  /* Insert EEDIT BUFFER Chars */
          for( j = 0; j<getEditBufferIndex(); j++ ) {
            mvaddch( row, col, getEditBufferChar(j) );
            col++;
          }

	  // NOT SURE IF THIS DOES ANYTHING...
	  /* Highlight, If Rendering Active Region */
          //if( inRegionP( nextRow, i+colOffset ))
	  //attron( COLOR_PAIR( HIGHLT_BACKGROUND ));

          mvaddch( row, col, getBufferChar( nextRow, i+colOffset ));
          col++;
	  attroff( COLOR_PAIR( HIGHLT_BACKGROUND ));
        }

        /* Ignore Chars In Line Buffer Gap */
        else if( nextRow == thisRow                    &&
		 bufferRowEditedP( nextRow )           &&
		 i >= getBufferGapLeftIndex( nextRow ) &&
		 i <  getBufferGapRightIndex(nextRow )) {

	  continue;
        }

        /* Insert Line Buffer Chars */
        else {
          if( inRegionP( nextRow, i+colOffset ))
	    attron( COLOR_PAIR( HIGHLT_BACKGROUND ));

          mvaddch( row, col, getBufferChar( nextRow, i+colOffset ));
          col++;

	  attroff( COLOR_PAIR( HIGHLT_BACKGROUND ));
        }
      }
    } 

    
    else {                        /* vi style EOF Markers */      
      mvaddch( row, 0, '~' );
      clrtoeol();
    }    
  }

  /* Draw Status Line: Filename, Status, Row/Col info */
  drawStatusLine( getBufferFilename(),
		  getStatusFlagName(),
		  thisRow, fileRows,
		  thisCol, getBufferLineLen( thisRow ));

  move( getPointY(), getPointX() );	     /* Set POINT */
  refresh();
}

/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
