#include  "lms2012.h"
#include  "c_ui.h"
#include  "d_lcd.h"



#include  <stdio.h>
#include  <string.h>
#include  <math.h>
#include  <fcntl.h>
#include  <unistd.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <errno.h>
#include <endian.h>
#include <linux/fb.h>

#define RED_LEGO	0	
#define GREEN_LEGO	255	
#define BLUE_LEGO	0	

static const unsigned char color0 = (RED_LEGO & 0xF8) | ((GREEN_LEGO & 0xE0) >> 5);
static const unsigned char color1 = ((GREEN_LEGO & 0x1C) << 3) | ((BLUE_LEGO & 0xF8) >> 3);


int dll = 60;
int fll = 22 + 1;
unsigned char vmem[7680];
unsigned char *dbuf = vmem;
unsigned char *fbp = NULL;





UBYTE     PixelTab[] =
{
    0x00, // 000 00000000
    0xE0, // 001 11100000
    0x1C, // 010 00011100
    0xFC, // 011 11111100
    0x03, // 100 00000011
    0xE3, // 101 11100011
    0x1F, // 110 00011111
    0xFF  // 111 11111111
};

void update_to_fb(void)
{
	unsigned long x, y, location, offset, mask;

	for(y = 0; y < 128; y++)
	{
		for(x = 0; x < 178; x++)
		{
			location = (x + y * 220) * 2;
			offset = x % 3;
			if(offset)
			{
				mask = (offset >> 1) ? 0x1 : 0x8;
			}
			else
			{
				mask = 0x80;
			}

			if(vmem[x / 3 + y * 60] & mask)
			{
				*(fbp + location++) = color0;
				*(fbp + location) = color1;
			}
			else
			{
				*(fbp + location++) = 0x0;
				*(fbp + location) = 0x0;
			}
		}
	}


	
}



void      dLcdExec(LCD *pDisp)
{
  UBYTE   *pSrc;
  UBYTE   *pDst;
  ULONG   Pixels;
  UWORD   X;
  UWORD   Y;

	
  if (dbuf)
  {
    if (memcmp((const void*)pDisp,(const void*)&VMInstance.LcdBuffer,sizeof(LCD)) != 0)
    {
      pSrc  =  (*pDisp).Lcd;
      pDst  =  dbuf;

      for (Y = 0;Y < 128;Y++)
      {
        for (X = 0;X < 7;X++)
        {
          Pixels  =  (ULONG)*pSrc;
          pSrc++;
          Pixels |=  (ULONG)*pSrc << 8;
          pSrc++;
          Pixels |=  (ULONG)*pSrc << 16;
          pSrc++;

          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
          Pixels >>= 3;
          *pDst   =  PixelTab[Pixels & 0x07];
          pDst++;
        }
        Pixels  =  (ULONG)*pSrc;
        pSrc++;
        Pixels |=  (ULONG)*pSrc << 8;
        pSrc++;

        *pDst   =  PixelTab[Pixels & 0x07];
        pDst++;
        Pixels >>= 3;
        *pDst   =  PixelTab[Pixels & 0x07];
        pDst++;
        Pixels >>= 3;
        *pDst   =  PixelTab[Pixels & 0x07];
        pDst++;
        Pixels >>= 3;
        *pDst   =  PixelTab[Pixels & 0x07];
        pDst++;
      }

      LCDCopy(&UiInstance.LcdBuffer,&VMInstance.LcdBuffer,sizeof(LCD));
      VMInstance.LcdUpdated  =  1;

      update_to_fb();
    }
  }
}




#ifdef MAX_FRAMES_PER_SEC
void      dLcdAutoUpdate(void)
{
  if (UiInstance.AllowUpdate)
  {
    if (UiInstance.DisplayUpdate)
    {
      dLcdExec(&UiInstance.LcdBuffer);
      UiInstance.DisplayUpdate  =  0;
      UiInstance.DisplayTimer   =  0;
      UiInstance.AllowUpdate    =  0;
    }
  }
}
#endif


void      dLcdUpdate(LCD *pDisp)
{
#ifdef MAX_FRAMES_PER_SEC
    LCDCopy(pDisp,&UiInstance.LcdBuffer,sizeof(LCD));
    UiInstance.DisplayUpdate  =  1;
    dLcdAutoUpdate();
#else
    dLcdExec(pDisp);
#endif
}




void      dLcdInit(UBYTE *pImage)
{
	int x, y, location;
  UiInstance.DispFile = open(LCD_DEVICE_NAME, O_RDWR);
//  if (UiInstance.DispFile < 0) LogErrorNumber(LCD_DEVICE_FILE_NOT_FOUND);

  //ioctl(UiInstance.DispFile, _IOW('S',0, int), NULL);

  //FBCTL(FBIOGET_VSCREENINFO, &var);
  //FBCTL(FBIOGET_FSCREENINFO, &fix);

  /* Display line length in bytes */
  //dll = fix.line_length;
  /* Image file line length in bytes */
  //fll = (var.xres >> 3) + 1;

  fbp = (unsigned char *)mmap(0, 220 * 176 * 2, PROT_WRITE | PROT_READ, MAP_SHARED, UiInstance.DispFile, 0);
//  if (fbp == MAP_FAILED) LogErrorNumber(LCD_DEVICE_FILE_NOT_FOUND);


	for(y = 0; y <= 127 + 1; y++)
	{
		for(x = 0; x <= 177 + 1; x++)
		{
			location = (x + y * 220) * 2;
			*(fbp + location++) = 0x0;
			*(fbp + location) = 0x0;
		}
	}

	y = 128;
	for(x = 0; x < 178 + 1; x++)
	{
		location = (x + y * 220) *2;
		*(fbp + location++) = color0;
		*(fbp + location) = color1;
	}
	
	x = 178;
	for(y = 0; y < 128 + 1; y++)
	{
		location = (x + y * 220) *2;
		*(fbp + location++) = color0;
		*(fbp + location) = color1;
	}




}


UBYTE     dLcdRead(void)
{
  return (0);
}


void      dLcdExit(void)
{
  if (UiInstance.DispFile >= MIN_HANDLE)
  {
    close(UiInstance.DispFile);
  }
}





































void      dLcdScroll(UBYTE *pImage,DATA16 Y0)
{

  memmove(pImage,&pImage[((LCD_WIDTH + 7) / 8) * Y0],(LCD_HEIGHT - Y0) * ((LCD_WIDTH + 7) / 8));
  memset(&pImage[(LCD_HEIGHT - Y0) * ((LCD_WIDTH + 7) / 8)],0,((LCD_WIDTH + 7) / 8) * Y0);
}


void      dLcdDrawPixel(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0)
{
  if ((X0 >= 0) && (X0 < LCD_WIDTH) && (Y0 >= 0) && (Y0 < LCD_HEIGHT))
  {
    if (Color)
    {
      pImage[(X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3)]  |=  (1 << (X0 % 8));
    }
    else
    {
      pImage[(X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3)]  &= ~(1 << (X0 % 8));
    }
  }
}


void      dLcdInversePixel(UBYTE *pImage,DATA16 X0,DATA16 Y0)
{
  if ((X0 >= 0) && (X0 < LCD_WIDTH) && (Y0 >= 0) && (Y0 < LCD_HEIGHT))
  {
    pImage[(X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3)]  ^=  (1 << (X0 % 8));
  }
}



DATA8     dLcdReadPixel(UBYTE *pImage,DATA16 X0,DATA16 Y0)
{
  DATA8   Result = 0;

  if ((X0 >= 0) && (X0 < LCD_WIDTH) && (Y0 >= 0) && (Y0 < LCD_HEIGHT))
  {
    if ((pImage[(X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3)] & (1 << (X0 % 8))))
    {
      Result  =  1;
    }
  }

  return (Result);
}




void      dLcdDrawLine(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  DATA32  XLength;
  DATA32  YLength;
  DATA16  XInc;
  DATA16  YInc;
  DATA32  Diff;
  DATA32  Tmp;


  if (X0 < X1)
  {
    XLength  =  (DATA32)X1 - (DATA32)X0;
    XInc     =  1;
  }
  else
  {
    XLength  =  (DATA32)X0 - (DATA32)X1;
    XInc     = -1;
  }
  if (Y0 < Y1)
  {
    YLength  =  (DATA32)Y1 - (DATA32)Y0;
    YInc     =  1;
  }
  else
  {
    YLength  =  (DATA32)Y0 - (DATA32)Y1;
    YInc     = -1;
  }
  Diff  =  XLength - YLength;

  dLcdDrawPixel(pImage,Color,X0,Y0);

  while ((X0 != X1) || (Y0 != Y1))
  {
    Tmp  =  Diff << 1;
    if (Tmp > (-YLength))
    {
      Diff -=  YLength;
      X0   +=  XInc;
    }
    if (Tmp < XLength)
    {
      Diff +=  XLength;
      Y0   +=  YInc;
    }
    dLcdDrawPixel(pImage,Color,X0,Y0);
  }
}


void      dLcdDrawDotLine(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1,DATA16 On,DATA16 Off)
{
  DATA32  XLength;
  DATA32  YLength;
  DATA16  XInc;
  DATA16  YInc;
  DATA32  Diff;
  DATA32  Tmp;
  DATA16  Count;

  if ((X0 != X1) && (Y0 != Y1))
  {
    dLcdDrawLine(pImage,Color,X0,Y0,X1,Y1);
  }
  else
  {
    if (On < 0)
    {
      On  =  0;
    }
    if (On > 255)
    {
      On  =  255;
    }
    if (Off < 0)
    {
      Off  =  0;
    }
    if (Off > 255)
    {
      Off  =  255;
    }

    if (X0 < X1)
    {
      XLength  =  (DATA32)X1 - (DATA32)X0;
      XInc     =  1;
    }
    else
    {
      XLength  =  (DATA32)X0 - (DATA32)X1;
      XInc     = -1;
    }
    if (Y0 < Y1)
    {
      YLength  =  (DATA32)Y1 - (DATA32)Y0;
      YInc     =  1;
    }
    else
    {
      YLength  =  (DATA32)Y0 - (DATA32)Y1;
      YInc     = -1;
    }
    Diff  =  XLength - YLength;

    dLcdDrawPixel(pImage,Color,X0,Y0);
    Count  =  1;

    while ((X0 != X1) || (Y0 != Y1))
    {
      Tmp  =  Diff << 1;
      if (Tmp > (-YLength))
      {
        Diff -=  YLength;
        X0   +=  XInc;
      }
      if (Tmp < XLength)
      {
        Diff +=  XLength;
        Y0   +=  YInc;
      }
      if (Count < (On + Off))
      {
        if (Count < On)
        {
          dLcdDrawPixel(pImage,Color,X0,Y0);
        }
        else
        {
          dLcdDrawPixel(pImage,1 - Color,X0,Y0);
        }
      }
      Count++;
      if (Count >= (On + Off))
      {
        Count  =  0;
      }
    }
  }
}






void      dLcdPlotPoints(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  dLcdDrawPixel(pImage,Color,X0 + X1,Y0 + Y1);
  dLcdDrawPixel(pImage,Color,X0 - X1,Y0 + Y1);
  dLcdDrawPixel(pImage,Color,X0 + X1,Y0 - Y1);
  dLcdDrawPixel(pImage,Color,X0 - X1,Y0 - Y1);
  dLcdDrawPixel(pImage,Color,X0 + Y1,Y0 + X1);
  dLcdDrawPixel(pImage,Color,X0 - Y1,Y0 + X1);
  dLcdDrawPixel(pImage,Color,X0 + Y1,Y0 - X1);
  dLcdDrawPixel(pImage,Color,X0 - Y1,Y0 - X1);
}








void      dLcdDrawCircle(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 R)
{
  int     X = 0;
  int     Y = R;
  int     P = 3 - 2 * R;

  while (X<Y)
  {
    dLcdPlotPoints(pImage,Color,X0,Y0,X,Y);
    if (P < 0)
    {
      P = P + 4 * X + 6;
    }
    else
    {
      P = P + 4 * (X - Y) + 10;
      Y = Y - 1;
    }
    X = X + 1;
  }
  dLcdPlotPoints(pImage,Color,X0,Y0,X,Y);
}























































typedef   struct
{
  const   char    *pFontBits;           // Pointer to start of font bitmap
  const   DATA16  FontHeight;           // Character height (all inclusive)
  const   DATA16  FontWidth;            // Character width (all inclusive)
  const   DATA16  FontHorz;             // Number of horizontal character in font bitmap
  const   DATA8   FontFirst;            // First character supported
  const   DATA8   FontLast;             // Last character supported
}
FONTINFO;


#include  "normal_font.xbm"
#include  "small_font.xbm"
#include  "large_font.xbm"
#include  "tiny_font.xbm"


FONTINFO  FontInfo[] =
{
  [NORMAL_FONT] = {
                    .pFontBits    = (const char*)normal_font_bits,
                    .FontHeight   = 9,
                    .FontWidth    = 8,
                    .FontHorz     = 16,
                    .FontFirst    = 0x20,
                    .FontLast     = 0x7F
                  },
  [SMALL_FONT] =  {
                    .pFontBits    = (const char*)small_font_bits,
                    .FontHeight   = 8,
                    .FontWidth    = 8,
                    .FontHorz     = 16,
                    .FontFirst    = 0x20,
                    .FontLast     = 0x7F
                  },
  [LARGE_FONT] =  {
                    .pFontBits    = (const char*)large_font_bits,
                    .FontHeight   = 16,
                    .FontWidth    = 16,
                    .FontHorz     = 16,
                    .FontFirst    = 0x20,
                    .FontLast     = 0x7F
                  },
  [TINY_FONT] =   {
                    .pFontBits    = (const char*)tiny_font_bits,
                    .FontHeight   = 7,
                    .FontWidth    = 5,
                    .FontHorz     = 16,
                    .FontFirst    = 0x20,
                    .FontLast     = 0x7F
                  },

};










DATA16    dLcdGetFontWidth(DATA8 Font)
{
  return (FontInfo[Font].FontWidth);
}


DATA16    dLcdGetFontHeight(DATA8 Font)
{
  return (FontInfo[Font].FontHeight);
}





void      dLcdDrawChar(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA8 Font,DATA8 Char)
{
  DATA16  CharWidth;
  DATA16  CharHeight;
  DATA16  CharByteIndex;
  DATA16  LcdByteIndex;
  UBYTE   CharByte;
  DATA16  Tmp,X,Y,TmpX,MaxX;


  CharWidth   =  FontInfo[Font].FontWidth;
  CharHeight  =  FontInfo[Font].FontHeight;

  if ((Char >= FontInfo[Font].FontFirst) && (Char <= FontInfo[Font].FontLast))
  {
    Char -=  FontInfo[Font].FontFirst;

    CharByteIndex  =  (Char % FontInfo[Font].FontHorz) * ((CharWidth + 7) / 8);
    CharByteIndex +=  ((Char / FontInfo[Font].FontHorz) * ((CharWidth + 7) / 8) * CharHeight * FontInfo[Font].FontHorz);

    if (((CharWidth % 8) == 0) && ((X0 % 8) == 0))
    { // Font aligned

      X0             =  (X0 >> 3) << 3;
      LcdByteIndex   =  (X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3);

      if (Color)
      {
        while (CharHeight)
        {
          Tmp  =  0;
          do
          {
            if (LcdByteIndex < sizeof(LCD))
            {
              pImage[LcdByteIndex + Tmp]  =  FontInfo[Font].pFontBits[CharByteIndex + Tmp];
            }
            Tmp++;
          }
          while (Tmp < (CharWidth / 8));

          CharByteIndex +=  (CharWidth * FontInfo[Font].FontHorz) / 8;
          LcdByteIndex  +=  ((LCD_WIDTH + 7) >> 3);
          CharHeight--;
        }
      }
      else
      {
        while (CharHeight)
        {
          Tmp  =  0;
          do
          {
            if (LcdByteIndex < sizeof(LCD))
            {
              pImage[LcdByteIndex + Tmp]  = ~FontInfo[Font].pFontBits[CharByteIndex + Tmp];
            }
            Tmp++;
          }
          while (Tmp < (CharWidth / 8));

          CharByteIndex +=  (CharWidth * FontInfo[Font].FontHorz) / 8;
          LcdByteIndex  +=  ((LCD_WIDTH + 7) >> 3);
          CharHeight--;
        }
      }
    }
    else
    { // Font not aligned

      MaxX          =  X0 + CharWidth;

      if (Color)
      {
        for (Y = 0;Y < CharHeight;Y++)
        {
          TmpX              =  X0;

          for (X = 0;X < ((CharWidth + 7) / 8);X++)
          {
            CharByte  =  FontInfo[Font].pFontBits[CharByteIndex + X];

            for (Tmp = 0;(Tmp < 8) && (TmpX < MaxX);Tmp++)
            {
              if (CharByte & 1)
              {
                dLcdDrawPixel(pImage,1,TmpX,Y0);
              }
              else
              {
                dLcdDrawPixel(pImage,0,TmpX,Y0);
              }
              CharByte >>= 1;
              TmpX++;
            }
          }
          Y0++;
          CharByteIndex +=  ((CharWidth + 7) / 8) * FontInfo[Font].FontHorz;

        }
      }
      else
      {
        for (Y = 0;Y < CharHeight;Y++)
        {
          TmpX              =  X0;

          for (X = 0;X < ((CharWidth + 7) / 8);X++)
          {
            CharByte  =  FontInfo[Font].pFontBits[CharByteIndex + X];

            for (Tmp = 0;(Tmp < 8) && (TmpX < MaxX);Tmp++)
            {
              if (CharByte & 1)
              {
                dLcdDrawPixel(pImage,0,TmpX,Y0);
              }
              else
              {
                dLcdDrawPixel(pImage,1,TmpX,Y0);
              }
              CharByte >>= 1;
              TmpX++;
            }
          }
          Y0++;
          CharByteIndex +=  ((CharWidth + 7) / 8) * FontInfo[Font].FontHorz;
        }
      }
    }
  }
}




void      dLcdDrawText(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA8 Font,DATA8 *pText)
{
  while (*pText)
  {
    if (X0 < (LCD_WIDTH - FontInfo[Font].FontWidth))
    {
      dLcdDrawChar(pImage,Color,X0,Y0,Font,*pText);
      X0 +=  FontInfo[Font].FontWidth;
    }
    pText++;
  }
}













































typedef   struct
{
  const   char    *pIconBits;
  const   DATA16  IconSize;
  const   DATA16  IconHeight;
  const   DATA16  IconWidth;
}
ICONINFO;

#include  "normal_icons.xbm"
#include  "small_icons.xbm"
#include  "large_icons.xbm"
#include  "menu_icons.xbm"
#include  "arrow_icons.xbm"


ICONINFO  IconInfo[] =
{
  [NORMAL_ICON] = {
                    .pIconBits    = normal_icons_bits,
                    .IconSize     = normal_icons_height,
                    .IconHeight   = 12,
                    .IconWidth    = normal_icons_width
                  },
  [SMALL_ICON] =  {
                    .pIconBits    = small_icons_bits,
                    .IconSize     = small_icons_height,
                    .IconHeight   = 8,
                    .IconWidth    = small_icons_width
                  },
  [LARGE_ICON] =  {
                    .pIconBits    = large_icons_bits,
                    .IconSize     = large_icons_height,
                    .IconHeight   = 22,
                    .IconWidth    = large_icons_width
                  },
  [MENU_ICON] =   {
                    .pIconBits    = menu_icons_bits,
                    .IconSize     = menu_icons_height,
                    .IconHeight   = 12,
                    .IconWidth    = menu_icons_width
                  },
  [ARROW_ICON] =  {
                    .pIconBits    = arrow_icons_bits,
                    .IconSize     = arrow_icons_height,
                    .IconHeight   = 12,
                    .IconWidth    = arrow_icons_width
                  },
};







UBYTE    *dLcdGetIconBits(DATA8 Type)
{
  UBYTE  *pResult;

  pResult  =  (UBYTE*)IconInfo[Type].pIconBits;

  return (pResult);
}


DATA16    dLcdGetIconWidth(DATA8 Type)
{
  return (IconInfo[Type].IconWidth);
}


DATA16    dLcdGetIconHeight(DATA8 Type)
{
  return (IconInfo[Type].IconHeight);
}


DATA16    dLcdGetNoOfIcons(DATA8 Type)
{
  return (IconInfo[Type].IconSize / IconInfo[Type].IconHeight);
}






void      dLcdDrawPicture(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 IconWidth,DATA16 IconHeight,UBYTE *pIconBits)
{
  DATA16  IconByteIndex;
  DATA16  LcdByteIndex;
  DATA16  Tmp;

  IconByteIndex  =  0;

  X0             =  (X0 >> 3) << 3;
  LcdByteIndex   =  (X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3);


  if (Color)
  {
    while (IconHeight)
    {
      for (Tmp = 0;Tmp < (IconWidth / 8);Tmp++)
      {
        pImage[LcdByteIndex + Tmp] =  pIconBits[IconByteIndex + Tmp];
      }

      IconByteIndex +=  IconWidth / 8;
      LcdByteIndex  +=  ((LCD_WIDTH + 7) >> 3);
      IconHeight--;
    }
  }
  else
  {
    while (IconHeight)
    {
      for (Tmp = 0;Tmp < (IconWidth / 8);Tmp++)
      {
        pImage[LcdByteIndex + Tmp] = ~pIconBits[IconByteIndex + Tmp];
      }

      IconByteIndex +=  IconWidth / 8;
      LcdByteIndex  +=  ((LCD_WIDTH + 7) >> 3);
      IconHeight--;
    }
  }
}














void      dLcdDrawIcon(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA8 Type,DATA8 No)
{
  DATA16  IconByteIndex;
  DATA16  IconHeight;
  DATA16  IconWidth;
  UBYTE   *pIconBits;

  IconHeight  =  dLcdGetIconHeight(Type);
  IconWidth   =  dLcdGetIconWidth(Type);

  if ((No >= 0) && (No <= dLcdGetNoOfIcons(Type)))
  {
    pIconBits      =  dLcdGetIconBits(Type);
    IconByteIndex  =  ((DATA16)No * IconWidth * IconHeight) / 8;
    dLcdDrawPicture(pImage,Color,X0,Y0,IconWidth,IconHeight,&pIconBits[IconByteIndex]);
  }
}


void      dLcdGetBitmapSize(IP pBitmap,DATA16 *pWidth,DATA16 *pHeight)
{
  *pWidth     =  0;
  *pHeight    =  0;

  if (pBitmap)
  {

    *pWidth     =  (DATA16)pBitmap[0];
    *pHeight    =  (DATA16)pBitmap[1];
  }
}






void      dLcdDrawBitmap(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,IP pBitmap)
{
  DATA16  BitmapWidth;
  DATA16  BitmapHeight;
  DATA16  BitmapByteIndex;
  UBYTE   *pBitmapBytes;
  UBYTE   BitmapByte;
  DATA16  Tmp,X,Y,TmpX,MaxX;

  DATA16  LcdByteIndex;

  if (pBitmap)
  {

    BitmapWidth   =  (DATA16)pBitmap[0];
    BitmapHeight  =  (DATA16)pBitmap[1];
    MaxX          =  X0 + BitmapWidth;
    pBitmapBytes  =  &pBitmap[2];

    if ((BitmapWidth >=0) && (BitmapHeight >= 0))
    {
      if ((X0 % 8) || (BitmapWidth % 8))
      { // X is not aligned

        BitmapWidth       =  ((BitmapWidth + 7) >> 3) << 3;

        if (Color)
        {
          for (Y = 0;Y < BitmapHeight;Y++)
          {
            BitmapByteIndex   =  (Y * BitmapWidth) / 8;
            TmpX              =  X0;

            for (X = 0;X < (BitmapWidth / 8);X++)
            {
              BitmapByte  =  pBitmapBytes[BitmapByteIndex + X];

              for (Tmp = 0;(Tmp < 8) && (TmpX < MaxX);Tmp++)
              {
                if (BitmapByte & 1)
                {
                  dLcdDrawPixel(pImage,1,TmpX,Y0);
                }
                else
                {
                  dLcdDrawPixel(pImage,0,TmpX,Y0);
                }
                BitmapByte >>= 1;
                TmpX++;
              }
            }
            Y0++;
          }
        }
        else
        {
          for (Y = 0;Y < BitmapHeight;Y++)
          {
            BitmapByteIndex   =  (Y * BitmapWidth) / 8;
            TmpX              =  X0;

            for (X = 0;X < (BitmapWidth / 8);X++)
            {
              BitmapByte  =  pBitmapBytes[BitmapByteIndex + X];

              for (Tmp = 0;(Tmp < 8) && (TmpX < MaxX);Tmp++)
              {
                if (BitmapByte & 1)
                {
                  dLcdDrawPixel(pImage,0,TmpX,Y0);
                }
                else
                {
                  dLcdDrawPixel(pImage,1,TmpX,Y0);
                }
                BitmapByte >>= 1;
                TmpX++;
              }
            }
            Y0++;
          }
        }
      }

      else
      { // X is byte aligned

        BitmapByteIndex   =  0;

        LcdByteIndex      =  (X0 >> 3) + Y0 * ((LCD_WIDTH + 7) >> 3);

        if (Color)
        {
          while (BitmapHeight)
          {
            X  =  X0;
            for (Tmp = 0;Tmp < (BitmapWidth / 8);Tmp++)
            {
              if (((LcdByteIndex + Tmp) < LCD_BUFFER_SIZE) && (X < LCD_WIDTH) && (X >= 0))
              {
                pImage[LcdByteIndex + Tmp]  =  pBitmapBytes[BitmapByteIndex + Tmp];
              }
              X +=  8;
            }

            BitmapByteIndex +=  BitmapWidth / 8;
            LcdByteIndex    +=  ((LCD_WIDTH + 7) >> 3);

            BitmapHeight--;
          }
        }
        else
        {
          while (BitmapHeight)
          {
            X  =  X0;
            for (Tmp = 0;Tmp < (BitmapWidth / 8);Tmp++)
            {
              if (((LcdByteIndex + Tmp) < LCD_BUFFER_SIZE) && (X < LCD_WIDTH) && (X >= 0))
              {
                pImage[LcdByteIndex + Tmp]  = ~pBitmapBytes[BitmapByteIndex + Tmp];
              }
              X +=  8;
            }

            BitmapByteIndex +=  BitmapWidth / 8;
            LcdByteIndex    +=  ((LCD_WIDTH + 7) >> 3);

            BitmapHeight--;
          }
        }
      }

    }
  }
}





void      dLcdRect(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  X1--;
  Y1--;
  dLcdDrawLine(pImage,Color,X0,Y0,X0 + X1,Y0);
  dLcdDrawLine(pImage,Color,X0 + X1,Y0,X0 + X1,Y0 + Y1);
  dLcdDrawLine(pImage,Color,X0 + X1,Y0 + Y1,X0,Y0 + Y1);
  dLcdDrawLine(pImage,Color,X0,Y0 + Y1,X0,Y0);
}


void      dLcdFillRect(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  DATA16  StartX;
  DATA16  MaxX;
  DATA16  MaxY;

  StartX  =  X0;
  MaxX    =  X0 + X1;
  MaxY    =  Y0 + Y1;

  for (;Y0 < MaxY;Y0++)
  {
    for (X0 = StartX;X0 < MaxX;X0++)
    {
      dLcdDrawPixel(pImage,Color,X0,Y0);
    }
  }
}




void      dLcdInverseRect(UBYTE *pImage,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  DATA16  StartX;
  DATA16  MaxX;
  DATA16  MaxY;

  StartX  =  X0;
  MaxX    =  X0 + X1;
  MaxY    =  Y0 + Y1;

  for (;Y0 < MaxY;Y0++)
  {
    for (X0 = StartX;X0 < MaxX;X0++)
    {
      dLcdInversePixel(pImage,X0,Y0);
    }
  }
}


void      dLcdPlotLines(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 X1,DATA16 Y1)
{
  dLcdDrawLine(pImage,Color,X0 - X1,Y0 + Y1,X0 + X1,Y0 + Y1);
  dLcdDrawLine(pImage,Color,X0 - X1,Y0 - Y1,X0 + X1,Y0 - Y1);
  dLcdDrawLine(pImage,Color,X0 - Y1,Y0 + X1,X0 + Y1,Y0 + X1);
  dLcdDrawLine(pImage,Color,X0 - Y1,Y0 - X1,X0 + Y1,Y0 - X1);
}


void      dLcdDrawFilledCircle(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0,DATA16 R)
{
  int     X = 0;
  int     Y = R;
  int     P = 3 - 2 * R;

  while (X<Y)
  {
    dLcdPlotLines(pImage,Color,X0,Y0,X,Y);
    if (P < 0)
    {
      P = P + 4 * X + 6;
    }
    else
    {
      P = P + 4 * (X - Y) + 10;
      Y = Y - 1;
    }
    X = X + 1;
  }
  dLcdPlotLines(pImage,Color,X0,Y0,X,Y);
}







DATA8     dLcdCheckPixel(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0)
{
  DATA8   Result = 0;

  if ((X0 >= 0) && (X0 < LCD_WIDTH) && (Y0 >= 0) && (Y0 < LCD_HEIGHT))
  {
    if (dLcdReadPixel(pImage,X0,Y0) != Color)
    {
      Result  =  1;
    }
  }

  return (Result);
}






void      dLcdFlodfill(UBYTE *pImage,DATA8 Color,DATA16 X0,DATA16 Y0)
{
  DATA16  X;
  DATA16  Y;

  Y  =  Y0;
  X  =  X0;

  while (dLcdCheckPixel(pImage,Color,X,Y))
  {
    while (dLcdCheckPixel(pImage,Color,X,Y))
    {
      if (X != X0)
      {
        dLcdDrawPixel(pImage,Color,X,Y);
      }
      X--;
    }
    X  =  X0;
    Y--;
  }

  Y  =  Y0;
  X  =  X0;

  while (dLcdCheckPixel(pImage,Color,X,Y))
  {
    while (dLcdCheckPixel(pImage,Color,X,Y))
    {
      if (X != X0)
      {
        dLcdDrawPixel(pImage,Color,X,Y);
      }
      X--;
    }
    X  =  X0;
    Y++;
  }

  Y  =  Y0;
  X  =  X0;

  while (dLcdCheckPixel(pImage,Color,X,Y))
  {
    while (dLcdCheckPixel(pImage,Color,X,Y))
    {
      dLcdDrawPixel(pImage,Color,X,Y);
      X++;
    }
    X  =  X0;
    Y--;
  }

  Y  =  Y0 + 1;
  X  =  X0;

  while (dLcdCheckPixel(pImage,Color,X,Y))
  {
    while (dLcdCheckPixel(pImage,Color,X,Y))
    {
      dLcdDrawPixel(pImage,Color,X,Y);
      X++;
    }
    X  =  X0;
    Y++;
  }

}



