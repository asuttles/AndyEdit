#include <curses.h>
#include <stdbool.h>

#define FNLENGTH 128

/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

/* Public Definitions */
void die( const char * );	 /* Print Die Message on Failure */
int getRowOffset( void );
int getColOffset( void );
void setRowOffset( int );
void setColOffset( int );
void setNumRows( int );
int getBufferNumRows( void );

int getEditBufferIndex( void );
char getEditBufferChar(int);
void setEditBufferIndex( int );
void setStatusFlagOriginal( void );

void setDefaultFilename( void );
void setFilename( char * );
char *getBufferFilename( void );

//
void updateLine( void );
