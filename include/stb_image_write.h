#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

// 只在一个源文件中定义这个宏
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef unsigned int stbiw_uint32;
#define STBIW_ASSERT(x) assert(x)
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

typedef struct {
   stbi_uc *data;
   int w,h,comp;
   const char *filename;
} stbi__write_context;

extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

static void stbiw__writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
         case '2': { int x = va_arg(v,int); unsigned char b[2];
                     b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
                     fwrite(b,2,1,f); break; }
         case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
                     fwrite(b,4,1,f); break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void stbiw__writef(FILE *f, const char *fmt, ...)
{
   va_list v;
   va_start(v, fmt);
   stbiw__writefv(f, fmt, v);
   va_end(v);
}

static void stbiw__write_flush(stbi__write_context *s)
{
   if (s->data) {
      STBIW_FREE(s->data);
      s->data = NULL;
   }
}

static void stbiw__write3(stbi__write_context *s, unsigned char a, unsigned char b, unsigned char c)
{
   s->data[s->comp * s->w * s->h + 0] = a;
   s->data[s->comp * s->w * s->h + 1] = b;
   s->data[s->comp * s->w * s->h + 2] = c;
   s->comp = 3;
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, unsigned char *d)
{
   unsigned char bg[3] = { 255, 255, 255 };
   int k;

   if (write_alpha < 0)
      stbiw__write3(s, d[0], d[0], d[0]); // monochrome
   else
      stbiw__write3(s, d[0], d[1], d[2]); // color
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono)
{
   stbiw_uint32 zero = 0;
   int i,j, k;

   if (y <= 0 || x <= 0) return;

   if (s->data == NULL) {
      s->data = (unsigned char *) STBIW_MALLOC(x * y * comp);
      if (s->data == NULL) { 
         STBIW_FREE(s);
         return;
      }
   }

   if (vdir < 0)
      j = y-1;
   else
      j = 0;

   for (; j >= 0 && j < y; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
      }
   }
}

static int stbi_write_png_core(stbi__write_context *s, int stride_bytes)
{
   int p;
   if (stride_bytes == 0)
      stride_bytes = s->w * s->comp;

   // compute color-based attributes for PNG
   int color_type = 0;
   if (s->comp == 1) color_type = 0;
   else if (s->comp == 2) color_type = 4;
   else if (s->comp == 3) color_type = 2;
   else if (s->comp == 4) color_type = 6;

   FILE *f = fopen(s->filename, "wb");
   if (!f) return 0;

   // write PNG header
   unsigned char header[] = { 137,80,78,71,13,10,26,10 };
   fwrite(header, 8, 1, f);

   // write IHDR
   stbiw__writef(f, "IHDR");
   stbiw__writef(f, "%4d %4d %1d %1d %1d %1d %1d", s->w, s->h, 8, color_type, 0,0,0);

   // write IDAT
   stbiw__writef(f, "IDAT");
   fwrite(s->data, s->w * s->h * s->comp, 1, f);

   // write IEND
   stbiw__writef(f, "IEND");

   fclose(f);
   return 1;
}

int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   stbi__write_context s;
   s.data = NULL;
   s.w = x;
   s.h = y;
   s.comp = comp;
   s.filename = filename;
   return stbi_write_png_core(&s, stride_bytes);
}

#endif // STB_IMAGE_WRITE_IMPLEMENTATION
#endif // INCLUDE_STB_IMAGE_WRITE_H 