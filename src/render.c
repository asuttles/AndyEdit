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
#include "ae.h"
#include "statusBar.h"
#include "pointMarkRegion.h"
#include "buffer.h"

/*******************************************************************************
                             RENDER TEXT
*******************************************************************************/

/* Draw and Color the Rows of Text */
void renderText(char *fn,	    /* Filename Rendered */
		const char *status, /* File Status */
		int maxRows,	    /* Max Rows/Cols on Screen */
		int maxCols,
		int fileRows	    /* Number Rows in File */
		) {

  int i, j;			/* Iteration indices */
  int row, col;			/* Row/Col being processed */
  int nextRow;			/* The next row to process */
  int colMax, txtLen;

  /* Initialized Data */
  buff_t buff = getBufferHandle();
  int ebIndex = getEditBufferIndex();

  int rowOffset = getRowOffset();
  int colOffset = getColOffset();

  int thisRow = rowOffset+getPointY(); /* Row Containing POINT */
  int thisCol = colOffset+getPointX();


  for( row = 0; row < maxRows; row++ ) {

    nextRow = row+rowOffset;	     /* Next Row to Process */

    if( nextRow < fileRows ) {        /* Write Buffer Text */

      /* Calculate what subset of the Row Fits in the Term */
      txtLen = (int)buff[nextRow]->len - colOffset;
      colMax = maxCols > txtLen ? txtLen : maxCols;

      /* Write Letter at a Time */
      col = 0;
      for( i=0; i < colMax; i++ ) {

        /* Insert Edit Buffer Chars */
        if( nextRow == thisRow    &&
            buff[nextRow]->editP  &&
            ( buff[nextRow]->lPtr == buff[nextRow]->rPtr ) &&
            ( i == (int)buff[nextRow]->lPtr )) {

          for( j = 0; j<ebIndex; j++ ) {
            mvaddch( row, col, getEditBufferChar(j) );
            col++;
          }

          if( inRegionP( nextRow, i+colOffset ))
            //attron( A_STANDOUT );
	    attron( COLOR_PAIR( HIGHLT_BACKGROUND ));
          mvaddch( row, col, buff[nextRow]->txt[i+colOffset] );
          col++;
	  attroff( COLOR_PAIR( HIGHLT_BACKGROUND ));
          //attroff( A_STANDOUT );
        }

        /* Ignore Chars In Line Buffer Gap */
        else if( nextRow == thisRow     &&
                 buff[thisRow]->editP &&
                 i >= (int)buff[nextRow]->lPtr && 
                 i <  (int)buff[nextRow]->rPtr ) {

	  continue;
        }

        /* Insert Line Buffer Chars */
        else {
          if( inRegionP( nextRow, i+colOffset ))
	    //attron( A_STANDOUT );
	    attron( COLOR_PAIR( HIGHLT_BACKGROUND ));
          mvaddch( row, col, buff[nextRow]->txt[i+colOffset] );
          col++;
          //attroff( A_STANDOUT );
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
  drawStatusLine( fn, status,
		  thisRow, fileRows,
		  thisCol, (int)buff[thisRow]->len );

  move( getPointY(), getPointX() );	/* Set POINT */
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
