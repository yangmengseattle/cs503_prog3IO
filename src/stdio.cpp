//-----------------------------stdio.cpp-------------------------------------------
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
#include "stdio.h"
#include <fcntl.h>     // open
#include <sys/types.h> // read
#include <sys/uio.h>   // read
#include <unistd.h>    // read, close
#include <strings.h>   // bcopy
#include <string.h>    // strlen
#include <stdarg.h>    // format, ...
#include <stdlib.h>    // abs
#include <errno.h>	  //global variable errno


char decimal[100];
//---------------------------------~FILE-------------------------------------------
//Description: destructor of class FILE. It closes any open file by calling system
//call close, releases the buffer by deleting the buffer and setting the buffer to
//NULL.
//---------------------------------------------------------------------------------
FILE::~FILE()
{
	close(fd);
	if(buffer != NULL && bufown)
	{
		delete buffer;
		buffer = NULL;
	}
}

//---------------------------------recursive_itoa----------------------------------
//Description: The itoa recursively turns its integer argument into an array of
//chars.
//---------------------------------------------------------------------------------
int recursive_itoa( int arg )
{
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if ( div > 0 )
		index = recursive_itoa( div );
	decimal[index] = mod + '0';//convert 0 to ASCII value eg. 32
	return ++index;
}

//---------------------------------itoa--------------------------------------------
//Description: The itoa turns its integer argument into an array of chars.
//---------------------------------------------------------------------------------
char *itoa( const int arg )
{
	bzero( decimal, 100 );//write 100 zeroed bytes to decimal
	int order = recursive_itoa( arg );
	char *new_decimal = new char[order + 1];
	bcopy( decimal, new_decimal, order + 1 );
	return new_decimal;
}

//---------------------------------printf------------------------------------------
//Description: The printf utility formats and prints its arguments, after the first,
//under control of the format.
//---------------------------------------------------------------------------------
int printf( const void *format, ... )
{
	va_list list;
	va_start( list, format );//initialise a variable argument list

	char *msg = ( char * )format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0;
	while ( msg[i] != '\0') {//end of a char array
		if ( msg[i] == '%' && msg[i + 1] == 'd' )
		{//find the %d
			buf[j] = '\0';
			nWritten += write( 1, buf, j );//system call, write up to j bytes from
			//the buffer pointed to by buf to the file referred to by the fd 1(stdin).
			//On success, the number of bytes written is returned.
			j = 0;
			i += 2;

			int int_val = va_arg( list, int );//retrieve next argument
			//returns the current additional argument of type int
			char *dec = itoa( abs( int_val ) );//itoa: int to array
			if ( int_val < 0 )
				nWritten += write( 1, "-", 1 );
			nWritten += write( 1, dec, strlen( dec ) );
			delete dec;
		}
		else
			buf[j++] = msg[i++];//assign msg to buf
	}
	if ( j > 0 )
		nWritten += write( 1, buf, j );
	va_end( list );//end using variable argument list
	return 0;
}

//---------------------------------setvbuf-----------------------------------------
//Description: setvbuf is used to alter the buffering behaviour of a stream
//_IONBF  unbuffered
//_IOLBF  line buffered
//_IOFBF  fully buffered
//The setvbuf() function returns 0 on success, or EOF if the request cannot
//be honored
//---------------------------------------------------------------------------------
int setvbuf( FILE *stream, char *buf, int mode, size_t size )
{
	if ( mode != _IONBF && mode != _IOLBF && mode != _IOFBF )
		return -1;
	stream->mode = mode;
	stream->pos = 0;

	if ( stream->buffer != (char *)0 && stream->bufown == true )
	{
		delete stream->buffer;
		stream->buffer = NULL;
	}

	switch ( mode ) {
	case _IONBF://unbuffered
		stream->buffer = (char *)0;
		stream->size = 0;
		stream->bufown = false;
		break;
	case _IOLBF://line buffered
	case _IOFBF://fully buffered
		if ( buf != (char *)0 )
		{
			stream->buffer = buf;
			stream->size   = size;
			stream->bufown = false;
		}
		else
		{
			stream->buffer = new char[BUFSIZ];
			stream->size = BUFSIZ;
			stream->bufown = true;
		}
		break;
	}
	return 0;
}

//---------------------------------setbuf------------------------------------------
//Description: setbuf is a similar alias with setvbuf
//The setbuf() function is exactly equivalent to the call
//setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
//---------------------------------------------------------------------------------
void setbuf( FILE *stream, char *buf )
{
	setvbuf( stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ );
}

//----------------------------------fopen------------------------------------------
//Description: The fopen() function opens the file whose name is the string pointed
//to by path and associates a stream with it. The argument mode points to a string
// beginning with one of the  following sequences.
//---------------------------------------------------------------------------------
FILE *fopen( const char *path, const char *mode )
{
	FILE *stream = new FILE( );//instantiate a FILE object
	setvbuf( stream, (char *)0, _IOFBF, BUFSIZ );

	// fopen( ) mode
	// r or rb           =  O_RDONLY
	// w or wb           =  O_WRONLY | O_CREAT | O_TRUNC
	// a or ab           =  O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b  =  O_RDWR
	// w+ or wb+ or w+b  =  O_RDWR   | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b  =  O_RDWR   | O_CREAT | O_APPEND
	//initialise the object according to the file modes, i.e., flag
	switch( mode[0] ) {
	case 'r':
		if ( mode[1] == '\0' )            // r
			stream->flag = O_RDONLY;
		else if ( mode[1] == 'b' ) {
			if ( mode[2] == '\0' )          // rb
				stream->flag = O_RDONLY;
			else if ( mode[2] == '+' )      // rb+
				stream->flag = O_RDWR;
		}
		else if ( mode[1] == '+' )        // r+  r+b
			stream->flag = O_RDWR;
		break;
	case 'w':
		if ( mode[1] == '\0' )            // w
			stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		else if ( mode[1] == 'b' ) {
			if ( mode[2] == '\0' )          // wb
				stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
			else if ( mode[2] == '+' )      // wb+
				stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		}
		else if ( mode[1] == '+' )        // w+  w+b
			stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		break;
	case 'a':
		if ( mode[1] == '\0' )            // a
			stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		else if ( mode[1] == 'b' ) {
			if ( mode[2] == '\0' )          // ab
				stream->flag = O_WRONLY | O_CREAT | O_APPEND;
			else if ( mode[2] == '+' )      // ab+
				stream->flag = O_RDWR | O_CREAT | O_APPEND;
		}
		else if ( mode[1] == '+' )        // a+  a+b
			stream->flag = O_RDWR | O_CREAT | O_APPEND;
		break;
	}

	mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	//open a file using the system call open
	if ( ( stream->fd = open( path, stream->flag, open_mode ) ) == -1 )
	{
		delete stream;
		printf( "fopen failed\n" );
		stream = NULL;
	}

	return stream;
}

//---------------------------------fpurge------------------------------------------
//Description: fpurge is used to flush a stream, it erases any input or output
//buffered in the given stream. For output streams this discards any unwritten output.
//For input streams this discards any input read from the underlying object but not
//yet obtained via getc(3); this includes any text pushed back via ungetc(3).
//---------------------------------------------------------------------------------
int fpurge( FILE *stream )
{
	// complete it
	if(stream == NULL)//if stream equals to NULL, flush all open output streams
	{
		return EOF;
	}
	else
	{
		if(stream->mode > 0) //line buffered or fully buffered
		{
			//discard whatever is in the buffer by writing the bytes to zero
			bzero(stream->buffer, stream->actual_size);//??actual size or size?
			stream->pos = 0;//set pos to the beginning of the buffer
			stream->actual_size = 0;//set actual size to zero??necessary or not??
		}
		else//not buffered, then return 0
		{
			return 0;
		}

	}

	return 0;
}

//---------------------------------fflush------------------------------------------
//Description: fflush is used to flush a stream, it forces a write of all buffered
//data for the given output or update stream via the stream's underlying write
//function.
//The open status of the stream is unaffected.
//Upon successful completion 0 is returned.  Otherwise, EOF is returned and the
//global variable errno is set to indicate the error.
//---------------------------------------------------------------------------------
int fflush( FILE *stream )
{
	// comlete it
	if(stream == NULL)//if stream equals to NULL, flush all open output streams
	{
		return EOF;// EOF = -1
	}
	//write of all buffered data for the given output, this happens after each
	//write so no loop is required
	//system call write
	int retval = write(stream->fd, stream->buffer, stream->actual_size);
	//set pos to the beginning of the buffer
	stream->pos = 0;
	stream->actual_size = 0;
	return retval == -1 ? EOF : 0;
}

//---------------------------------fread-------------------------------------------
//Description: Reads an array of nmemb elements, each one with a size of size bytes,
//from the stream and stores them in the block of memory specified by ptr.
//The position indicator of the stream is advanced by the total amount of bytes read.
//The total amount of bytes read if successful is (size*nmemb).
//On success, fread() returns the number of items read.
//---------------------------------------------------------------------------------
size_t fread( void *ptr, size_t size, size_t nmemb, FILE *stream )
{
	// complete it
	//If either size or nmemb is zero, the function returns zero and both the stream
	//state and the content pointed by ptr remain unchanged.
	if(size == 0 || nmemb == 0)
	{
		return 0;
	}

	size_t available_size = stream->actual_size - stream->pos;
	const size_t desired_total = nmemb * size;

	if (desired_total <= available_size)
	{
		memcpy(((char*) ptr), stream->buffer + stream->pos, desired_total);
		stream->pos += desired_total;
		return desired_total / size;
	}
	else
	{
		//copy the available bytes in stream->buffer to the destination
		memcpy(((char*) ptr), stream->buffer + stream->pos, available_size);
		ptr = (char*) ptr + available_size;
		int size_read = available_size;
		while (size_read < desired_total && !stream->eof)
		{
			//call the system call, to fill the 8K buffer
			int bytes_read_buf = read(stream->fd, stream->buffer, stream->size);
			if(bytes_read_buf == 0)
			{
				stream->eof = true;
				break;
			}
			else if(bytes_read_buf < 0)
			{
				break;
			}

			//reset the status of stream
			stream->pos = 0;
			stream->actual_size = bytes_read_buf;
			//copy the rest to ptr.
			int rest_size = stream->actual_size < desired_total - available_size
					? stream->actual_size
							: desired_total - available_size;
			memcpy((char*) ptr, stream->buffer + stream->pos, rest_size);
			ptr = (char*) ptr + rest_size;
			size_read += rest_size;
			stream->pos += rest_size;
		}
		return size_read / size;
	}
}

//---------------------------------fwrite-------------------------------------------
//Description: writes nmemb objects, each size bytes long, to the stream pointed to
//by stream, obtaining them from the location given by ptr.
//fwrite() advances the file position indicator for the stream by the number of bytes
//written. It returns the number of objects written.
//If an error occurs, or the end-of-file is reached, the return value is a short
//object count (or zero).
//The function fwrite() returns a value less than nmemb only if a write error
//has occurred.
//----------------------------------------------------------------------------------
size_t fwrite( const void *ptr, size_t size, size_t nmemb, FILE *stream ) {
	// comlete it
	if(size == 0 || nmemb == 0)//there's no data to write
	{
		return 0;
	}
	int bytes_2_write = size * nmemb;

	while(bytes_2_write > 0)
	{
		//calc how many bytes the stream->buffer could contain.
		int bytes_wrote = stream->size - stream->pos < bytes_2_write
				? stream->size - stream->pos
						: bytes_2_write;

		//copy bytes from ptr to stream->buffer
		memcpy(stream->buffer + stream->pos, ptr, bytes_wrote);
		ptr = (char*)ptr + bytes_wrote;
		stream->pos += bytes_wrote;
		stream->actual_size += bytes_wrote;

		//the stream->buffer is full, then write the whole buffer to disk.
		if (stream->pos == stream->size)
		{
			int retval = write(stream->fd, stream->buffer, stream->size);
			stream->pos = 0;
			stream->actual_size = 0;
			if(retval == -1)
			{
				break;
			}
		}

		bytes_2_write -= bytes_wrote;
	}
	return (size * nmemb - bytes_2_write) / size;
}

//---------------------------------fgetc--------------------------------------------
//Description: fgetc()  reads  the  next  character  from  stream and returns it as
//an unsigned char cast to an int, or EOF on end of file or error.
//----------------------------------------------------------------------------------
int fgetc( FILE *stream )
{
	// complete it
	if(stream->pos == stream->actual_size)
	{
		int bytes_read_buf = read(stream->fd, stream->buffer, stream->size);
		if(bytes_read_buf == 0)
		{
			stream->eof = true;
			return EOF;
		}
		else if(bytes_read_buf < 0)
		{
			return EOF;
		}

		stream->pos = 0;
		stream->actual_size = bytes_read_buf;
	}

	if(stream->pos < stream->actual_size)
	{
		char byte = stream->buffer[stream->pos];
		stream->pos++;
		return (int) byte;
	}
	return EOF;
}

//---------------------------------fputc--------------------------------------------
//Description: fputc output a character or word to a stream
//return the character written.  If an error occurs, the value EOF is returned.
//----------------------------------------------------------------------------------
int fputc( int c, FILE *stream )
{
	// complete it
	char ch = (char) c;
	int retval = fwrite(&ch, 1, 1, stream);
	return retval == EOF ? EOF : c;
}

//---------------------------------fgets--------------------------------------------
//Description:Read at most size-1 characters from stream and
//stores them into the buffer pointed to by str. Stop when a
//newline has been read, or the count runs out.
//Return first argument, or NULL if no characters were read.
//Do not return NULL if size == 1.
//----------------------------------------------------------------------------------
char *fgets( char *str, int size, FILE *stream )
{
	// complete it
	int ch;
	char *ptr;

	/* get size bytes or upto a newline */
	for (ptr = str, size--; size > 0; size--)
	{
		if ((ch = fgetc (stream)) == EOF)
			break;
		*ptr++ = ch;
		if (ch == '\n')
			break;
	}
	*ptr = 0;
	if (ptr == str || ch == EOF)
		return NULL;
	return str;
}

//---------------------------------fputs--------------------------------------------
//Description: fputs output a line to a stream, writes the string pointed to by str
//to the stream pointed to by stream.
//----------------------------------------------------------------------------------
int fputs( const char *str, FILE *stream )
{
	// complete it
	while(*str != '\0') {
		int retval = fputc((int) *str++, stream);
		if (retval == EOF) {
			return EOF;
		}
	}
	return 0;
}

int feof( FILE *stream )
{
	return stream->eof == true;
}

//---------------------------------fseek--------------------------------------------
//Description: fseek () repositions a stream by setting the file position indicator
//for the stream pointed to by stream.  The new position, measured in bytes, is
//obtained by adding offset bytes to the position specified by whence.  If whence is
//set to SEEK_SET, SEEK_CUR, or SEEK_END, the offset is relative to the
//start of the file, the current position indicator, or end-of-file,
//respectively.
//----------------------------------------------------------------------------------
int fseek( FILE *stream, long offset, int whence )
{
	// complete it
	int result;

	if ((stream->flag & _IONBF) == 0 && stream->buffer != NULL)
	{
		/* deal with buffering */
		if (stream->flag & O_WRONLY || stream->flag & O_RDWR)
		{
			if (fflush(stream))
			{
				return EOF;
			}
		} else if (stream->flag & O_RDONLY || stream->flag & O_RDWR)
		{
			/* Reading so thrash buffer, do some housekeeping first
			 */
			if (whence == SEEK_CUR)
			{
				/* fix offset so that it's from the last
				 * character the user read (not the last character that was
				 * actually read)
				 */
				if (offset >=0 && offset <= stream->actual_size)
				{
					/* easy shortcut */
					stream->actual_size -= offset;
					stream->pos += offset;
					return 0;
				}
				else
				{
					offset -= stream->actual_size;
				}
			}
			stream->actual_size = 0;
			stream->pos = 0;
		}
	}

	result = (lseek(stream->fd, offset, whence) < 0);
	return result;
}

//---------------------------------fclose-------------------------------------------
//Description: Closes the file associated with the stream and disassociates it.
//All internal buffers associated with the stream are disassociated from it and
//flushed: the content of any unwritten output buffer is written and the content of
//any unread input buffer is discarded.
//If the stream is successfully closed, a zero value is returned.
//On failure, EOF is returned.
//----------------------------------------------------------------------------------
int fclose( FILE *stream )
{
	// complete it
	//if there's data left in the buffer, call flush
	if (stream->flag & O_WRONLY || stream->flag & O_RDWR)
	{
		if (fflush(stream))
		{
			return EOF;
		}
	}
	if (stream->bufown)
	{
		delete stream->buffer;
		stream->buffer = NULL;
		close(stream->fd);//close the file
		delete stream;
		stream = NULL;
	}
	return 0;
}
