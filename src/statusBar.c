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
#include "ae.h"

/* Private Data */
static char statusLine[256];


/*******************************************************************************
                              STATUS BAR
*******************************************************************************/

/* Draw Status Line */
void drawStatusLine( char *fn, const char *status,
		     int row, int numRows,
		     int col, int numCols ) {

  int  curCol, maxCol;

  int  maxX   = getmaxx( getWindowHandle());
  int  curRow = getmaxy( getWindowHandle()) - 2;
  
  attron( A_REVERSE );                /* Reverse Video */

  snprintf( statusLine, maxX > 256 ? 256 : maxX  - 1,
            "--[ %s ]-------(%s)------- Row %d of %d ------- Col %d of %d ------- F1 for Help --- F10 to Quit",
            fn, status, row+1, numRows, col+1, numCols );

  mvaddstr( curRow, 0, statusLine );

  curCol = getcurx( getWindowHandle());
  maxCol = getmaxx( getWindowHandle());
  
  while( curCol < maxCol ) {
    mvaddch( curRow, curCol++, '-' );
  }
  
  attroff( A_REVERSE );
}
