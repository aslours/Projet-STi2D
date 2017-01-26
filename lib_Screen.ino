#include "SPI.h"
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>
#include <SD.h>

/**************************************************************************
COMMUN DEFINE
**************************************************************************/
char* SALLE_CLASSE = {"J-151"};

/**************************************************************************
DEFINE
**************************************************************************/
#define TFT_DC          9
#define TFT_CS          10 //Must be changed according to the board
#define SD_CS           4
#define STMPE_CS        8

#define ROTATION        1
#define BUFFPIXEL       20
#define TEXT_COLOR      ILI9341_RED

#define TS_MINX         150
#define TS_MINY         130
#define TS_MAXX         3800
#define TS_MAXY         4000

/**************************************************************************
STRUCTURE
**************************************************************************/
typedef struct TFT_MENU {
  char type_menu;
  int8_t count;
  char menus[4][24];
} TFT_MENU;

typedef struct TFT_TOUCH {
  uint16_t x;
  uint16_t y;
} TFT_TOUCH;

typedef struct CONTEXT {
  
} CONTEXT;

/**************************************************************************
GLOABL VARIABLES
**************************************************************************/
static Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
static Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

TFT_MENU *current_menu;

TFT_TOUCH tft_touch;

TFT_MENU menu_prof = {'P', 4, {"Faire l'appel", "Reaffecter une salle", "Visualiser EDT", "Retour"}  };
TFT_MENU menu_eleve = {'E', 3, {"Reserver cantine", "Visualiser EDT", "Retour" } };

/**************************************************************************
PRIVATE FUNCTIONS
**************************************************************************/
static void tft_print(int16_t x, int16_t y, uint16_t color, uint8_t textsize, char* text)
{
  tft.setTextColor(color);
  tft.setTextSize(textsize);
  tft.setCursor(x, y);
  tft.print(text);
}

static void drawButton(int16_t x, int16_t y, uint16_t w, uint16_t h, 
   uint16_t outline, uint16_t fill, uint16_t textcolor, 
   char *label, uint8_t textsize, boolean inverted) 
{
  uint16_t _fill, _outline, _text;
  if(!inverted) 
  {
    _fill    = fill;
    _outline = outline;
    _text    = textcolor;
  } 
  else 
  {
    _fill    = outline;
    _outline = fill;
    _text    = fill;
  }  

  tft.fillRoundRect(x, y, w, h, 4,  _fill);
  tft.drawRoundRect(x, y, w, h, 4, _outline);
  tft.setCursor(((320-(strlen(label)*(6*textsize)))/2)+3, y+(textsize*5));
  tft.setTextColor(_text);
  tft.setTextSize(textsize);
  tft.print(label);      
}

static void tft_draw_menus_items()
{
  uint16_t fill;
  
  for (uint8_t i= 0; i<current_menu->count; i++)
  {
    fill = i < current_menu->count-1 ? ILI9341_BLUE : ILI9341_ORANGE;
    drawButton(30, 62+(i*46), 260, 34, ILI9341_WHITE, fill, ILI9341_WHITE, current_menu->menus[i], 2, false);
  }
}

static void tft_execute_menu_prof(int8_t menu)
{
  switch (menu) 
  {
    case 0:
      tft_draw_wait_appel(ILI9341_ORANGE);
      delay(2000);
      tft_draw_card_student("Pennetier", "Clementine", "clem.bmp", "1STI2D2", "SIN", "13/12/2016 08:00", 2);
      delay(700);
      tft_draw_wait_appel(ILI9341_ORANGE);
      delay(2000);  
      tft_draw_card_student("Deverriere", "Lea", "lea.bmp", "1STI2D3", "SIN", "13/12/2016 08:00", 2);
      delay(700);    
      tft_draw_wait_appel(ILI9341_ORANGE);      
      delay(2000);  
      tft_draw_menu_prof("Mr Lamard");
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      tft_draw_wait_badge(ILI9341_BLUE);
      delay(2000);
      tft_draw_menu_eleve("Clementine Pennetier");
  }
}


static void tft_execute_menu_eleve(int8_t menu)
{

  Serial.println(menu);
  switch (menu) 
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      tft_draw_wait_badge(ILI9341_BLUE);
      delay(2000);
      tft_draw_menu_prof("Mr Lamard");
  }
}

static bool tft_touched()
{
  if (!ts.bufferEmpty())
  {
    TS_Point point = ts.getPoint();
    point.x = map(point.x, TS_MINY, TS_MAXY, 0, tft.height());
    point.y = map(point.y, TS_MINX, TS_MAXX, 0, tft.width());
    tft_touch.y = tft.height() - point.x;
    tft_touch.x = point.y;     
    return true;
  }
  return false;
}

static void tft_draw_header()
{
  tft_print(5, 0, TEXT_COLOR, 3, "Lycee Monod");
  tft_print(228, 0, ILI9341_NAVY, 3, SALLE_CLASSE); 
  tft.drawLine(0, 25, 319, 25, TEXT_COLOR);
  tft.drawLine(0, 26, 319, 26, TEXT_COLOR);
  tft.drawLine(0, 27, 319, 27, TEXT_COLOR); 
}

static int8_t tft_get_menu_touched()
{
  int8_t menu = -1;
  
  if ((tft_touch.x > 30 && tft_touch.x < 250) && (tft_touch.y >62))
  {
    menu = (int8_t)((tft_touch.y-62)/46);
    if (menu < current_menu->count)
    {    
        uint16_t color = menu < current_menu->count-1 ? ILI9341_BLUE : ILI9341_ORANGE;
        drawButton(30, 62+(menu*46), 260, 34, ILI9341_WHITE, color, ILI9341_WHITE, current_menu->menus[menu], 2, true);
        delay(100);  
        drawButton(30, 62+(menu*46), 260, 34, ILI9341_WHITE, color, ILI9341_WHITE, current_menu->menus[menu], 2, false);
    }
      else 
        return -1;
  }
  return menu;
}

void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) 
  {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) 
  { // BMP signature
    Serial.print(F("File size: ")); 
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) 
      { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) 
        { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) 
          { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer))
            { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }
  bmpFile.close();
  if(!goodBmp) 
    Serial.println(F("BMP format not recognized."));
}

uint16_t read16(File &f) 
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) 
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

/**************************************************************************
PUBLIC FUNCTIONS
**************************************************************************/
void tft_init_shield()
{
  tft.begin();
  yield();
  ts.begin();

  
  if (!SD.begin(SD_CS)){
    Serial.println("initialization SD failed!");
  }
  tft.setRotation(1);
}


/**************************************
 * Ecran d'attente du badge 
 **************************************/
void tft_draw_wait_badge(uint16_t color)
{
  tft.fillScreen(ILI9341_BLACK);
  tft_draw_header();
    
  tft.drawRoundRect(30, 58, 260, 152, 4, color);
  tft.drawRoundRect(31, 59, 258, 150, 4, color);
  tft.drawRoundRect(32, 60, 256, 148, 4, color);

  tft_print(97, 72, color, 3, "Attente");  
  tft_print(142, 72+52, color, 2, "de");
  tft_print(115, 72+94, color, 3, "Badge");  
}
/**************************************
 * Ecran d'attente du badge élève APPEL
 **************************************/
void tft_draw_wait_appel(uint16_t color)
{
  tft.fillScreen(ILI9341_BLACK);
  tft_draw_header();

  tft_print(62, 48, color, 3, "Appel Eleve");
    
  tft.drawRoundRect(70, 58+41, 180, 100, 4, color);
  tft.drawRoundRect(71, 58+42, 178, 98, 4, color);
  tft.drawRoundRect(72, 58+43, 176, 96, 4, color);

  tft_print(99, 108, color, 3, "Attente");  
  tft_print(144, 108+30, color, 2, "de");
  tft_print(117, 108+60, color, 3, "Badge");  
}

void tft_draw_card_student(char* nom, char* prenom, char* photo, char* classe, char* matiere, char* date, uint8_t duree)
{
  tft.fillScreen(ILI9341_BLACK);
  tft_draw_header();

  //draw picture
  tft.drawRect(9, 49, 122, 92, TEXT_COLOR);
  tft.drawRect(8, 48, 124, 94, TEXT_COLOR);
  bmpDraw(photo, 10, 50);

  tft.setTextColor(ILI9341_DARKGREY);
  tft.setCursor(140, 50);
  tft.setTextSize(2); 
  tft.print(classe); 
  tft.drawLine(140, 66, 310, 66, ILI9341_DARKGREY);
  tft.drawLine(140, 67, 310, 67, ILI9341_DARKGREY);  

  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(150, 75);
  tft.print(prenom);
  tft.setCursor(150, 96);
  tft.print(nom);

  tft.setCursor(10, 140+20);
  tft.setTextColor(ILI9341_DARKGREY); 
  tft.print("Cours");
  tft.setCursor(10+70, 140+22);
  tft.print(": ");
  tft.setTextColor(ILI9341_NAVY);
  tft.print(matiere);
  
  tft.setCursor(10, 140+40);
  tft.setTextColor(ILI9341_DARKGREY); 
  tft.print("Date");
  tft.setCursor(10+70, 140+44);
  tft.print(": "),
  tft.setTextColor(ILI9341_NAVY);
  tft.print("29/11/2016 14:05");

  tft.setCursor(10, 140+60);
  tft.setTextColor(ILI9341_DARKGREY); 
  tft.print("Duree");
  tft.setCursor(10+70, 140+66);
  tft.print(": ");
  tft.setTextColor(ILI9341_NAVY);
  char buffer[10];
  if (duree == 1)
    sprintf(buffer, "%d heure", duree);
  else  
    sprintf(buffer, "%d heures", duree);
  tft.print(buffer);  
}

/**************************************
 * Menu Prof
 **************************************/
void tft_draw_menu_prof(char* prof)
{  
  current_menu = &menu_prof;
  tft.fillScreen(ILI9341_BLACK);
  tft_draw_header();
  tft_print(5, 36, ILI9341_NAVY, 2, "Bonjour "); 
  tft.setTextColor(ILI9341_GREEN);
  tft.print(prof);
  tft_draw_menus_items();
}

void tft_draw_menu_eleve(char* eleve)
{
  current_menu = &menu_eleve;
  tft.fillScreen(ILI9341_BLACK);
  tft_draw_header();
  tft_print(5, 36, ILI9341_GREEN, 2, eleve); 
  tft_draw_menus_items();
}

/**************************************************************************
USED TO ARDUINO
**************************************************************************/

void setup() 
{
  Serial.begin(115200);
  tft_init_shield();

  tft_draw_wait_badge(ILI9341_BLUE);
  delay(2000);
  tft_draw_menu_prof("Mr Lamard");
}

void loop() 
{
  if (tft_touched() == true)
  {  
    //Be carefull it is the main menus  
    //if (context.mainmenu == true)    
    int8_t menu = tft_get_menu_touched();
    if (menu > -1)
    {   
        if (current_menu->type_menu == 'P')
        {
          tft_execute_menu_prof(menu);
          //tft_draw_menu_prof("Mr Lamard");
        }  
        else
          tft_execute_menu_eleve(menu);
    }
    while (!ts.bufferEmpty())
      ts.getPoint(); 
  }


/*  
  if (!ts.bufferEmpty())
  { 
    TS_Point p = ts.getPoint();
    p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    int y = tft.height() - p.x;
    int x = p.y;
    
    if ((x > 30 && x<250) && (y>62))
    {
      int8_t menu_index = (int8_t)((y-62)/46);
      if (menu_index < current_menu->count)
      {
        fill = menu_index < current_menu->count-1 ? ILI9341_BLUE : ILI9341_ORANGE;
        drawButton(30, 62+(menu_index*46), 260, 34, ILI9341_WHITE, fill, ILI9341_WHITE, current_menu->menus[menu_index], 2, true);
        delay(100);  
        while (!ts.bufferEmpty())
          ts.getPoint(); 
        drawButton(30, 62+(menu_index*46), 260, 34, ILI9341_WHITE, fill, ILI9341_WHITE, current_menu->menus[menu_index], 2, false);
        if (current_menu->type_menu == 'P')
          tft_execute_menu_prof(menu_index);
        else
          tft_execute_menu_eleve(menu_index);
        
      }  
    }
  }
  */
}
