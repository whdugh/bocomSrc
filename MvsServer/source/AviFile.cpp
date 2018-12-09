#include "AviFile.h"
#include <memory.h>
#include <stdlib.h>

#ifndef AVIF_HASINDEX
#define AVIF_HASINDEX		0x00000010	// Index at end of file?
#endif

#ifndef AVIF_ISINTERLEAVED
#define AVIF_ISINTERLEAVED	0x00000100
#endif

#ifndef AVIF_TRUSTCKTYPE
#define AVIF_TRUSTCKTYPE    0x00000800  // Use CKType to find key frames?
#endif

CAviFile::CAviFile()
{
	memset(&avi, 0 ,sizeof(avi));
	memset(&vb, 0 ,sizeof(vb));
}

CAviFile::~CAviFile()
{
	if (&vb)
	{
		free((&vb)->p_data);
	}
}

void CAviFile::vbuf_init( vbuf_t *v )
{
	v->i_data = 0;
	v->i_data_max = 10000;
	v->p_data = (uint8_t*)(malloc( v->i_data_max ));
}

void CAviFile::vbuf_add( vbuf_t *v, int i_data, void *p_data )
{
	if( i_data + v->i_data >= v->i_data_max )
	{
		v->i_data_max += i_data;
		v->p_data = (uint8_t*)realloc( v->p_data, v->i_data_max );
	}
	memcpy( &v->p_data[v->i_data], p_data, i_data );

	v->i_data += i_data;
}

void CAviFile::vbuf_reset( vbuf_t * v)
{
	v->i_data = 0;
}

void CAviFile::avi_init( avi_t *a, FILE *f, float f_fps, char fcc[4], int iWidth, int iHeight)
{
	if(NULL == f)
	{
		printf("avi_init error!");
		return;
	}

	a->f = f;
	a->f_fps = f_fps;
	memcpy( a->fcc, fcc, 4 );

	a->i_width = iWidth;
	a->i_height = iHeight;
	a->i_frame = 0;
	a->i_movi = 0;
	a->i_riff = 0;
	a->i_movi_end = 0;
	a->i_idx_max = 0;
	a->idx = NULL;

	avi_write_header( a );

	a->i_movi = ftell( a->f );
}

void CAviFile::avi_set_dw( void *_p, uint32_t dw )
{
	uint8_t *p = (uint8_t *)_p;

	p[0] = ( dw      )&0xff;
	p[1] = ( dw >> 8 )&0xff;
	p[2] = ( dw >> 16)&0xff;
	p[3] = ( dw >> 24)&0xff;
}

void CAviFile::avi_write( avi_t *a, vbuf_t *v, int b_key )
{
	if(NULL == a->f)
	{
		printf("avi_write error!");
		return;
	}

	int64_t i_pos = ftell( a->f );

	/* chunk header */
	avi_write_fourcc( a, "00dc" );
	avi_write_uint32( a, v->i_data );

	fwrite( v->p_data, v->i_data, 1, a->f );

	if( v->i_data&0x01 )
	{
		/* pad */
		fputc( 0, a->f );
	}

	/* Append idx chunk */
	if( a->i_idx_max <= a->i_frame )
	{
		a->i_idx_max += 1000;
		a->idx = (uint32_t *)realloc( a->idx, a->i_idx_max * 16 );
	}

	memcpy( &a->idx[4*a->i_frame+0], "00dc", 4 );
	avi_set_dw( &a->idx[4*a->i_frame+1], b_key ? AVIIF_KEYFRAME : 0 );
	avi_set_dw( &a->idx[4*a->i_frame+2], i_pos );
	avi_set_dw( &a->idx[4*a->i_frame+3], v->i_data );

	a->i_frame++;
}

void CAviFile::avi_end( avi_t *a )
{
	a->i_movi_end = ftell( a->f );

	/* write index */
	avi_write_idx( a );

	a->i_riff = ftell( a->f );

	/* Fix header */
	fseek( a->f, 0, SEEK_SET );
	avi_write_header( a );

	fprintf( stderr, "avi file written\n" );
	fprintf( stderr, "  - codec: %4.4s\n", a->fcc );
	fprintf( stderr, "  - size: %dx%d\n", a->i_width, a->i_height );
	fprintf( stderr, "  - fps: %.3f\n", a->f_fps );
	fprintf( stderr, "  - frames: %d\n", a->i_frame );
}

/*****************************************************************************
 * avi:
 *****************************************************************************/
void CAviFile::avi_write_uint16( avi_t *a, uint16_t w )
{
    fputc( ( w      ) & 0xff, a->f );
    fputc( ( w >> 8 ) & 0xff, a->f );
}

void CAviFile::avi_write_uint32( avi_t *a, uint32_t dw )
{
    fputc( ( dw      ) & 0xff, a->f );
    fputc( ( dw >> 8 ) & 0xff, a->f );
    fputc( ( dw >> 16) & 0xff, a->f );
    fputc( ( dw >> 24) & 0xff, a->f );
}

void CAviFile::avi_write_fourcc( avi_t *a, char fcc[4] )
{
    fputc( fcc[0], a->f );
    fputc( fcc[1], a->f );
    fputc( fcc[2], a->f );
    fputc( fcc[3], a->f );
}

void CAviFile::avi_write_header( avi_t *a )
{
	if(NULL == a->f)
	{
		printf("avi_write_header error!");
		return;
	}
    avi_write_fourcc( a, "RIFF" );
    avi_write_uint32( a, a->i_riff > 0 ? a->i_riff - 8 : 0xFFFFFFFF );
    avi_write_fourcc( a, "AVI " );

    avi_write_fourcc( a, "LIST" );
    avi_write_uint32( a,  4 + 4*16 + 12 + 4*16 + 4*12 );
    avi_write_fourcc( a, "hdrl" );

    avi_write_fourcc( a, "avih" );
    avi_write_uint32( a, 4*16 - 8 );
    avi_write_uint32( a, 1000000 / static_cast<unsigned int>(a->f_fps) );
    avi_write_uint32( a, 0xffffffff );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, AVIF_HASINDEX|AVIF_ISINTERLEAVED|AVIF_TRUSTCKTYPE);
    avi_write_uint32( a, a->i_frame );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 1 );
    avi_write_uint32( a, 1000000 );
    avi_write_uint32( a, a->i_width );
    avi_write_uint32( a, a->i_height );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 0 );

    avi_write_fourcc( a, "LIST" );
    avi_write_uint32( a,  4 + 4*16 + 4*12 );
    avi_write_fourcc( a, "strl" );

    avi_write_fourcc( a, "strh" );
    avi_write_uint32( a,  4*16 - 8 );
    avi_write_fourcc( a, "vids" );
    avi_write_fourcc( a, a->fcc );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, 1000 );
    avi_write_uint32( a, static_cast<unsigned int>(a->f_fps) * 1000 );
    avi_write_uint32( a, 0 );
    avi_write_uint32( a, a->i_frame );
    avi_write_uint32( a, 1024*1024 );
    avi_write_uint32( a, -1 );
    avi_write_uint32( a, a->i_width * a->i_height );
    avi_write_uint32( a, 0 );
    avi_write_uint16( a, a->i_width );
    avi_write_uint16( a, a->i_height );

    avi_write_fourcc( a, "strf" );
    avi_write_uint32( a,  4*12 - 8 );
    avi_write_uint32( a,  4*12 - 8 );
    avi_write_uint32( a,  a->i_width );
    avi_write_uint32( a,  a->i_height );
    avi_write_uint16( a,  1 );
    avi_write_uint16( a,  24 );
    avi_write_fourcc( a,  a->fcc );
    avi_write_uint32( a, a->i_width * a->i_height );
    avi_write_uint32( a,  0 );
    avi_write_uint32( a,  0 );
    avi_write_uint32( a,  0 );
    avi_write_uint32( a,  0 );

    avi_write_fourcc( a, "LIST" );
    avi_write_uint32( a,  a->i_movi_end > 0 ? a->i_movi_end - a->i_movi + 4: 0xFFFFFFFF );
    avi_write_fourcc( a, "movi" );
}

void CAviFile::avi_write_idx( avi_t *a )
{
    avi_write_fourcc( a, "idx1" );
    avi_write_uint32( a,  a->i_frame * 16 );

	if(a->f)
		fwrite( a->idx, a->i_frame * 16, 1, a->f );
}