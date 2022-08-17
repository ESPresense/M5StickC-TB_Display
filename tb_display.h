/******************************************************************************
 * Originally written by:
 *
 * Hague Nusseck @ electricidea
 * https://github.com/electricidea/M5StickC-TB_Display
 *
 * Distributed as-is; no warranty is given.
 ******************************************************************************/

// Enable or disable Word Wrap
extern boolean tb_display_word_wrap;

void tb_display_init();

// =============================================================
//           tb_display_show();
// clear the screen and display the text buffer
// =============================================================
void tb_display_show();

// =============================================================
//           tb_display_clear();
// clear the text buffer
// without refreshing the screen
// call tb_display_show() afterwards to clear the screen
// =============================================================
void tb_display_clear();

// =============================================================
//           tb_display_new_line();
// creates a new line and scroll the display upwards
// =============================================================
void tb_display_new_line();

// =============================================================
//        tb_display_print_String(const char *s, int chr_delay = 0);
// print a string
// The string is added to the text buffer and directly printed
// on the screen.
// The otional parameter "chr_delay" allows a "character by character"
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
void tb_display_print_String(const char *s, int chr_delay = 0);

// =============================================================
//           tb_display_print_char(byte data);
// print a single character
// the character is added to the text buffer and
// directly printed on the screen.
// The text is automatically wrapped if longer than the display
// example:
//    tb_display_print_char('X');
// =============================================================
void tb_display_print_char(byte data);
