#include <stddef.h>

/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

/* Buffer Pointer Type */
typedef row_t** buff_t;

/* Empty Buffer : Default or User Named */
enum _bn { DEFAULT, UNAMED };

/* Buffer Status */
void setBufferNumRows( int );
int getBufferNumRows( void );
int getBufferRow( void );
int getBufferCol( void );

/* Buffer Management */
void initializeBuffer( void );
buff_t getBufferHandle( void );
void openEmptyBuffer( enum _bn );
void readBufferFile( char * );
void saveBuffer( void );
void saveBufferNewName( void );
void doubleBufferSize( void );
bool bufferFullP( void );
void closeBuffer( void );
void killBuffer( void );

/* Buffer Information */
int getBufferLineLen( int ); 
char getBufferChar( int, int );
bool bufferLineModifiedP( int );
bool bufferRowEditedP( int );
void setBufferRowEdited( int, bool );

/* Buffer Line Right and Left Gap Pointers */
int getBufferGapRightIndex( int );
int getBufferGapLeftIndex( int );
int getBufferGapSize( int );

/* Modify Buffer Lines */
void freeBufferLine( int );
void freeBufferPointToEOL( int, int );
void replaceBufferLineText( int, int, char * );
void openLine( void );

/* Edit Buffer Information */
void setBufferGapPtrs( int, int, int );
void increaseBufferGap( int );
void combineLineWithPrior( void );
