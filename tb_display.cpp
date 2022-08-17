/******************************************************************************
 * Originally written by:
 *
 * Hague Nusseck @ electricidea
 * https://github.com/electricidea/M5StickC-TB_Display
 *
 * Distributed as-is; no warranty is given.
 ******************************************************************************/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "tb_display.h"

// TextSize 1 is very small on the display = hard to read
// Textsize 2 is good readable without the need of an microscope.
// Each size is 8 px taller.  8, 16, 24...
// Display size of M5StickC = 160x80
// With TEXT_HEIGHT=16, the screen can display:
//    5 rows of text in portrait mode
//   10 rows of text in landscape mode

// screen buffer for 10 rows of 60 characters max.
#define TEXT_BUFFER_HEIGHT_MAX 10
#define TEXT_BUFFER_LINE_LENGTH_MAX 60
char text_buffer[TEXT_BUFFER_HEIGHT_MAX][TEXT_BUFFER_LINE_LENGTH_MAX];

int text_buffer_height;
int text_buffer_width;
int text_buffer_write_pointer_x;
int text_buffer_write_pointer_y;
int text_buffer_read_pointer;

#define SCREEN_XSTARTPOS 5
int screen_xpos = SCREEN_XSTARTPOS;
// start writing at the last line
int screen_ypos;
// maximum width of the screen
int screen_max;

// Enable or disable Word Wrap
boolean tb_display_word_wrap = true;

uint32_t tb_display_text_color = TFT_WHITE;
uint32_t tb_display_background_color = TFT_BLACK;

// =============================================================
// Initialization of the Text Buffer and Screen
// ScreenRotation values:
// 1 = Button right
// 2 = Button above
// 3 = Button left
// 4 = Button below
//
// TextSize values: 1, 2, 3...
// each value corresponds to 8px taller: 8, 16, 24...
//
// Display size of M5StickC = 160x80pixel
// With TEXT_HEIGHT=16, the screen can display:
//    5 rows of text in landscape mode
//   10 rows of text in portrait mode
// =============================================================
TFT_eSPI tft = TFT_eSPI();
int charHeight = 16;
int charWidth = 8;
void tb_display_init() {
    tft.init();
    tft.setRotation(1);
    tft.setTextDatum(TC_DATUM);
    tft.setTextFont(TFT_FONT);
    tft.setTextPadding(8);
    float lcdHeight = tft.height();
    float lcdWidth = tft.width();
    screen_max = lcdWidth - 2;
    charHeight = tft.fontHeight();
    charWidth = tft.textWidth("W") + 2;
    text_buffer_height = ceil(lcdHeight / charHeight);
    text_buffer_width = ceil(screen_max / charWidth);
    tb_display_clear();
    tb_display_show();
}

// =============================================================
// clear the text buffer
// without refreshing the screen
// call tb_display_show(); to clear the screen
// =============================================================
void tb_display_clear(){
  for(int line=0; line<TEXT_BUFFER_HEIGHT_MAX; line++){
    for(int charpos=0; charpos<TEXT_BUFFER_LINE_LENGTH_MAX; charpos++){
      text_buffer[line][charpos]='\0';
    }
  }
  text_buffer_read_pointer = 0;
  text_buffer_write_pointer_x = 0;
  text_buffer_write_pointer_y = text_buffer_height-1;
  screen_xpos = SCREEN_XSTARTPOS;
  screen_ypos = tft.height() - charHeight;
}

// =============================================================
// draw a character, and return the width of that char in px
// =============================================================
int tb_drawChar(uint16_t data, int32_t screen_xpos, int32_t screen_ypos) {
    tft.drawChar(screen_xpos, screen_ypos, data, tb_display_text_color, tb_display_background_color, TFT_FONT);
    // auto measure = tft.textWidth(String((char)data));
    return charWidth;
}

// =============================================================
// clear the screen and display the text buffer
// =============================================================
void tb_display_show(){
  tft.fillScreen(tb_display_background_color);
  // Because different text sizes may not fit exactly, this may be negative.  That's okay.
  int yPos = tft.height() - text_buffer_height * charHeight;
  for(int n=0; n<text_buffer_height; n++){
    // modulo operation for line position
    int line = (text_buffer_read_pointer+n) % text_buffer_height;
    int xPos = SCREEN_XSTARTPOS;
    int charpos=0;
    while(xPos < screen_max && text_buffer[line][charpos] != '\0'){
      xPos += tb_drawChar(text_buffer[line][charpos],xPos,yPos);
      charpos++;
    }
    yPos = yPos + charHeight;
  }
  screen_xpos = SCREEN_XSTARTPOS;
}

// =============================================================
// creates a new line and scroll the display upwards
// =============================================================
void tb_display_new_line(){
  text_buffer_write_pointer_x = 0;
  text_buffer_write_pointer_y++;
  text_buffer_read_pointer++;
  // circular buffer...
  if(text_buffer_write_pointer_y >= text_buffer_height)
    text_buffer_write_pointer_y = 0;
  if(text_buffer_read_pointer >= text_buffer_height)
    text_buffer_read_pointer = 0;
  // clear the actual new line for writing (first character a null terminator)
  text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = '\0';
  tb_display_show();
}

// =============================================================
// print a single character
// the character is added to the text buffer and
// directly printed on the screen.
// The text is automatically wrapped if longer than the display
// example:
//    tb_display_print_char('X');
// =============================================================
void tb_display_print_char(byte data){
  // check for LF for new line
  if (data == '\n') {
    // last character in the text_buffer line  should be always a null terminator
    text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = '\0';
    tb_display_new_line();
  }
  // only 'printable' characters
  if (data > 31 && data < 128) {
    // print the character and get the new xpos
    screen_xpos += tb_drawChar(data, screen_xpos, screen_ypos);
    // if maximum number of characters reached
    if(text_buffer_write_pointer_x >= text_buffer_width-1){
      tb_display_new_line();
      // draw the character again because it was out of the screen last time
      screen_xpos += tb_drawChar(data,screen_xpos,screen_ypos);
    }
    // or if line wrap is reached
    if(screen_xpos >= screen_max) {
      // prepare for Word-Wrap stuff...
      // the buffer for storing the last word content
      char Char_buffer[TEXT_BUFFER_LINE_LENGTH_MAX];
      int n = 1;
      Char_buffer[0] = data;
      Char_buffer[n] = '\0';
      // if Word-Wrap, go backwards and get the last "word" by finding the
      // last space character:
      if(tb_display_word_wrap){
        int test_pos = text_buffer_write_pointer_x-1;
        // get backwards and search a space character
        while(test_pos > 0 && text_buffer[text_buffer_write_pointer_y][test_pos] != ' '){
          // store all the characters on the way back to the last space character
          Char_buffer[n] = text_buffer[text_buffer_write_pointer_y][test_pos];
          test_pos--;
          n++;
          Char_buffer[n] = '\0';
        }
        // if there was no space character in the row, Word-Wrap is not possible
        if(test_pos == 0) {
          // don't use the buffer but draw the character passed to the function
          n = 1;
        } else {
          // otherwise use the buffer to print the last found characters of the word
          // but only, if the charachter that causes a word wrap is not a space character
          if(data != ' '){
            // place a \0 at the position of the found space so that the drawing fuction ends here
            text_buffer[text_buffer_write_pointer_y][test_pos] = '\0';
          }
        }
      }
      tb_display_new_line();
      // if character passed to the function is a space character, then don't display
      // it as the first character of the new line
      if(data == ' ')
        // don't use the buffer at all
        n = 0;
      n--;
      while(n >= 0){
        // draw the characters from the buffer back on the screen
        screen_xpos += tb_drawChar(Char_buffer[n],screen_xpos,screen_ypos);
        // write the characters into the screen buffer of the new line
        text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = Char_buffer[n];
        text_buffer_write_pointer_x++;
        n--;
      }
      text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = '\0';
    } else {
      // write the character into the screen buffer
      text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = data;
      text_buffer_write_pointer_x++;
      // following character a null terminator to clear the old characters of the line
      text_buffer[text_buffer_write_pointer_y][text_buffer_write_pointer_x] = '\0';
    }
  }
}

// =============================================================
// print a string
// The string is added to the text buffer and directly printed
// on the screen.
// The optional parameter "chr_delay" allows a "character by character"
// processing of the String. Then, it looks like Teletype or Typewriter
// The delay is in milliseconds.
// The text is automatically wrapped if longer than the display
// example:
//    tb_display_print_String("a new line\n");
//    tb_display_print_String("one\nand two lines\n");
//
//    char String_buffer[128];
//    snprintf(String_buffer, sizeof(String_buffer), "\nthe value: %i",value);
//    tb_display_print_String(String_buffer);
//
//    std::string msg;
//    msg = ss.str();
//    const char * c_msg = msg.c_str();
//    tb_display_print_String(c_msg);
// =============================================================
void tb_display_print_String(const char *s, int chr_delay){
  while(*s != 0){
    tb_display_print_char(*s++);
    if(chr_delay > 0)
      delay(chr_delay);
  }
}
