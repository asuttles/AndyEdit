/* Restore tty */
void closeEditor( void );
void initializeTerminal( void );
WINDOW *getWindowHandle( void ); /* keyPress.c */
int getWinNumCols( void );
int getWinNumRows( void );
int getWinThisCol( void );
