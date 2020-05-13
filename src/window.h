#define NORMAL_BACKGROUND 0
#define HIGHLT_BACKGROUND 1

/* Restore tty */
void closeEditor( void );
void initializeTerminal( void );
WINDOW *getWindowHandle( void ); /* keyPress.c */
int getWinNumCols( void );
int getWinNumRows( void );
int getWinThisCol( void );
int getScreenRows( void );
