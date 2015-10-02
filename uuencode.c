
#include "includes.h"

void uuencode(const unsigned char *src, unsigned char *dest, unsigned int nmemb)
// encode nmemb bytes from src into dest.  return length of dest.  if nmemb is
// not a multiple of three the buffer pointed to by src must be at least
// the next multiple of three longer than nmemb because the extra byte or two
// will be read during the encoding.
{
	*dest++=nmemb+0x20;
	while(nmemb)
	{
		*dest=((src[0]>>2)&0x3f);
		*dest+=*dest?0x20:0x60;
		dest++;

		*dest=(((src[0]<<4)&0x30)|((src[1]>>4)&0x0f));
		*dest+=*dest?0x20:0x60;
		dest++;

		*dest=(((src[1]<<2)&0x3c)|((src[2]>>6)&0x03));
		*dest+=*dest?0x20:0x60;
		dest++;

		*dest=((src[2]>>0)&0x3f);
		*dest+=*dest?0x20:0x60;
		dest++;

		src+=3;
		nmemb=(nmemb>=3)?(nmemb-3):0;
	}
	*dest++='\0';
}

int uudecode(const char *src, unsigned char *dest)
// read from uuencoded data, decode and place into area pointed to by
// dest.  return number of bytes in src.  performs no error checking.
// may read up to two bytes past end of buffer pointed to by src
{
	int
		length,
		i,
		j;
	unsigned char
		s0,
		s1,
		s2,
		s3;

	length=*src-0x20;
	src++;
	j=0;
	for(i=0;i<length;i++)
	{
		s0=src[0]<0x60?src[0]-0x20:0x00;
		s1=src[1]<0x60?src[1]-0x20:0x00;
		s2=src[2]<0x60?src[2]-0x20:0x00;
		s3=src[3]<0x60?src[3]-0x20:0x00;
		switch(j)
		{
			case 0:
				*dest=(s0<<2)|((s1>>4)&0x03);
				break;
			case 1:
				*dest=(s1<<4)|((s2>>2)&0x0f);
				break;
			default:
				*dest=(s2<<6)|((s3>>0)&0x3f);
				break;
		}
		dest++;
		j++;
		if(j>2)
		{
			j=0;
			src+=4;
		}
	}
	return(length);
}
