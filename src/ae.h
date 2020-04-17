#include <curses.h>

/* Public Definitions */
WINDOW *getWindowHandle( void ); /* Get Handle for AE Window */
void die( const char * );	 /* Print Die Message on Failure */
void initializeTerminal( void ); /* (re)Initialize Editor Screen */
int getRowOffset( void );
int getColOffset( void );
void setRowOffset( int );
