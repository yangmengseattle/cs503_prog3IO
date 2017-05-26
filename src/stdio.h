//-----------------------------stdio.h---------------------------------------------
// Programmer's Name: Meng Yang
// Course Section Number: CSS503
// Creation Date: May 10th, 2017
// Date of Last Modification: May 25th, 2017
// --------------------------------------------------------------------------------
// Purpose: This assignment intends to familiarize us with the implementation of
// stdio class, the concept of file management, and its superiority over system
// calls
// --------------------------------------------------------------------------------
// Notes: The FILE class has a buffer, which is used for reading and writing data.
// Instead of reading/writing byte by byte, which is what the system call does, the
// data would first fill the buffer and then 1 system call would be called, thus
// improves the efficiency of data management. Each system call involves a switch
// between kernel mode and user mode, which is rather costly.
//---------------------------------------------------------------------------------
#ifndef _MY_STDIO_H_
#define _MY_STDIO_H_
#include <stddef.h> // size_t
#define BUFSIZ 8192 // default buffer size
#define _IONBF 0    // unbuffered
#define _IOLBF 1    // line buffered
#define _IOFBF 2    // fully buffered
#define EOF -1      // end of file

class FILE {
 public:
  FILE( ) : 
    fd( 0 ), pos( 0 ), buffer( (char *)0 ), size( 0 ), actual_size( 0 ),
    mode( _IONBF ), flag( 0 ), bufown( false ), lastop( 0 ), eof( false ) {}
  ~FILE();//destructor of class FILE
  int fd;          // a Unix file descriptor of an opened file, pointer
  int pos;         // the current file position in the buffer
  char *buffer;    // an input or output file stream buffer
  int size;        // the buffer size
  int actual_size; // the actual buffer size when read( ) returns # bytes read smaller than size
  int mode;        // _IONBF, _IOLBF, _IOFBF
  int flag;        // O_RDONLY 
                   // O_RDWR 
                   // O_WRONLY | O_CREAT | O_TRUNC
                   // O_WRONLY | O_CREAT | O_APPEND
                   // O_RDWR   | O_CREAT | O_TRUNC
                   // O_RDWR   | O_CREAT | O_APPEND
  bool bufown;     // true if allocated by stdio.h or false by a user?
  char lastop;     // 'r' or 'w' 
  bool eof;        // true if EOF is reached

};

//function declarations:
int recursive_itoa( int arg );
char *itoa( const int arg );
int printf( const void *format, ... );
int setvbuf( FILE *stream, char *buf, int mode, size_t size );
void setbuf( FILE *stream, char *buf );
FILE *fopen( const char *path, const char *mode );
int fpurge( FILE *stream );
int fflush( FILE *stream );
size_t fread( void *ptr, size_t size, size_t nmemb, FILE *stream );
size_t fwrite( const void *ptr, size_t size, size_t nmemb, FILE *stream );
int fgetc( FILE *stream );
int fputc( int c, FILE *stream );
char *fgets( char *str, int size, FILE *stream );
int fputs( const char *str, FILE *stream );
int feof( FILE *stream );
int fseek( FILE *stream, long offset, int whence );
int fclose( FILE *stream );

#endif
