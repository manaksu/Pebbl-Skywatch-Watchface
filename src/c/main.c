/*
 * SkyWatch Watchface — Pebble Time Steel (basalt, 144x168)
 *
 * Full-screen sky canvas with Tuscany pixel art horizon.
 * Sky colour transitions day→dusk→night.
 * Sun arcs left→right during day, moon arcs right→left at night.
 * Moon phase calculated from day of year.
 * Stars appear at night.
 *
 * Bottom strip: same sky colour as canvas (seamless).
 *   Left  : 3 health widgets (user-selectable)
 *   Right : clock (user-selectable style)
 *
 * AppMessage keys (Automatic, alphabetical order):
 *   0 = clockstyle   (0=dot  1=pixel  2=digital)
 *   1 = widget0      (slot 0 stat)
 *   2 = widget1      (slot 1 stat)
 *   3 = widget2      (slot 2 stat)
 *
 * Widget values: 0=steps 1=bpm 2=battery 3=calories 4=distance
 *
 * Persist keys: 1=clockstyle  2=widget0  3=widget1  4=widget2
 */

#include <pebble.h>

/* ================================================================ */
/*  CONSTANTS                                                        */
/* ================================================================ */

#define SCREEN_W   144
#define SCREEN_H   168
#define SKY_H      108
#define STRIP_Y    SKY_H

/* Clock — bottom right */
#define CLOCK_X    88
#define CLOCK_Y    (STRIP_Y + 3)
#define CLOCK_SIZE 54
#define CLOCK_CX   27
#define CLOCK_CY   27
#define CLOCK_R    23
#define PX_STEP    3
#define PX_SZ      2

/* Health widgets — bottom left */
#define WIDGET_X   3
#define WIDGET_Y   (STRIP_Y + 4)

/* Sun/moon pixel size */
#define SP         2

/* Clock styles */
#define CLOCK_DOT     0
#define CLOCK_PIXEL   1
#define CLOCK_DIGITAL 2

/* Widget types */
#define WIDGET_STEPS    0
#define WIDGET_BPM      1
#define WIDGET_BATTERY  2
#define WIDGET_CALORIES 3
#define WIDGET_DISTANCE 4

/* Persist */
#define PERSIST_CLOCKSTYLE 1
#define PERSIST_WIDGET0    2
#define PERSIST_WIDGET1    3
#define PERSIST_WIDGET2    4

/* AppMessage */
#define MSG_CLOCKSTYLE 0
#define MSG_WIDGET0    1
#define MSG_WIDGET1    2
#define MSG_WIDGET2    3

/* ================================================================ */
/*  TUSCANY SCENE — 72x36 grid at SP=2px each pixel                */
/* ================================================================ */

static const int16_t TUSCANY[][2] = {
  {19,1},{19,2},{20,2},{14,3},{19,3},{20,3},{29,3},{30,3},
  {14,4},{19,4},{20,4},{29,4},{30,4},{36,4},
  {13,5},{14,5},{18,5},{19,5},{20,5},{23,5},{29,5},{30,5},{31,5},{36,5},
  {13,6},{14,6},{18,6},{19,6},{20,6},{23,6},{29,6},{30,6},{31,6},{35,6},{36,6},
  {13,7},{14,7},{18,7},{19,7},{20,7},{23,7},{24,7},{28,7},{29,7},{30,7},{31,7},{35,7},{36,7},
  {13,8},{14,8},{15,8},{18,8},{19,8},{20,8},{23,8},{24,8},{28,8},{29,8},{30,8},{31,8},{35,8},{36,8},
  {13,9},{14,9},{15,9},{17,9},{18,9},{19,9},{20,9},{23,9},{24,9},
  {28,9},{29,9},{30,9},{31,9},{35,9},{36,9},{37,9},{52,9},
  {12,10},{13,10},{14,10},{15,10},{17,10},{18,10},{19,10},{20,10},{22,10},{23,10},{24,10},
  {28,10},{29,10},{30,10},{31,10},{35,10},{36,10},{37,10},
  {50,10},{51,10},{52,10},{53,10},{54,10},{56,10},{57,10},
  {12,11},{13,11},{14,11},{15,11},{17,11},{18,11},{19,11},{20,11},{21,11},{22,11},{23,11},{24,11},{25,11},
  {28,11},{29,11},{30,11},{31,11},{32,11},{34,11},{35,11},{36,11},{37,11},
  {49,11},{50,11},{51,11},{52,11},{53,11},{54,11},{55,11},{56,11},{57,11},{58,11},{59,11},
  {12,12},{13,12},{14,12},{15,12},{16,12},{17,12},{18,12},{19,12},{20,12},{21,12},{22,12},{23,12},{24,12},{25,12},
  {28,12},{29,12},{30,12},{31,12},{32,12},{34,12},{35,12},{36,12},{37,12},{38,12},
  {46,12},{47,12},{48,12},{49,12},{50,12},{51,12},{52,12},{53,12},{54,12},{55,12},{56,12},{57,12},{58,12},{59,12},
  {7,13},{8,13},
  {12,13},{13,13},{14,13},{15,13},{16,13},{17,13},{18,13},{19,13},{20,13},{21,13},{22,13},{23,13},{24,13},{25,13},
  {28,13},{29,13},{30,13},{31,13},{32,13},{34,13},{35,13},{36,13},{37,13},{38,13},
  {44,13},{46,13},{47,13},{48,13},{49,13},{50,13},{51,13},{52,13},{53,13},{54,13},{55,13},{56,13},{57,13},{58,13},{59,13},
  {63,13},{64,13},{65,13},{66,13},
  {10,14},{11,14},{12,14},{13,14},{14,14},{15,14},{16,14},{17,14},{18,14},{19,14},{20,14},{21,14},{22,14},{23,14},{24,14},{25,14},
  {28,14},{29,14},{30,14},{31,14},{32,14},{34,14},{35,14},{36,14},{37,14},{38,14},
  {44,14},{46,14},{47,14},{48,14},{49,14},{51,14},{52,14},{53,14},{54,14},{55,14},
  {59,14},{60,14},{61,14},{67,14},{69,14},{70,14},
  {12,15},{13,15},{14,15},{15,15},{16,15},{17,15},{18,15},{19,15},{20,15},{21,15},{22,15},{23,15},{24,15},{25,15},
  {28,15},{29,15},{30,15},{31,15},{32,15},{34,15},{35,15},{36,15},{37,15},{38,15},
  {44,15},{45,15},{49,15},{50,15},{54,15},{55,15},{56,15},{62,15},{65,15},
  {12,16},{13,16},{14,16},{15,16},{16,16},{17,16},{18,16},{19,16},{20,16},{21,16},{22,16},{23,16},{24,16},{25,16},
  {27,16},{28,16},{29,16},{30,16},{31,16},{32,16},{33,16},{34,16},{35,16},{36,16},{37,16},{38,16},{40,16},
  {46,16},{47,16},{52,16},{67,16},{69,16},
  {13,17},{14,17},{15,17},{19,17},{20,17},{23,17},{24,17},{25,17},
  {27,17},{28,17},{29,17},{30,17},{31,17},{34,17},{35,17},{36,17},{37,17},{38,17},
  {42,17},{43,17},{45,17},{48,17},{53,17},{63,17},{66,17},{71,17},
  {11,18},{19,18},{23,18},{24,18},{25,18},{28,18},{29,18},{30,18},{32,18},{34,18},{35,18},{36,18},{37,18},
  {43,18},{46,18},{57,18},{61,18},{65,18},{68,18},{71,18},
  {6,19},{7,19},{8,19},{9,19},{10,19},{11,19},{12,19},
  {15,19},{16,19},{17,19},{18,19},{19,19},{20,19},{21,19},{22,19},
  {26,19},{29,19},{30,19},{33,19},{35,19},{36,19},{37,19},{38,19},
  {50,19},{53,19},{59,19},{60,19},
  {3,20},{4,20},{11,20},{12,20},{13,20},{19,20},{24,20},{25,20},
  {27,20},{28,20},{29,20},{30,20},{33,20},{35,20},{36,20},{41,20},{49,20},{66,20},{67,20},{70,20},
  {1,21},{2,21},{9,21},{10,21},{11,21},{19,21},{20,21},{21,21},{22,21},
  {29,21},{30,21},{31,21},{32,21},{33,21},{34,21},{35,21},{36,21},
  {47,21},{52,21},{63,21},{66,21},{70,21},
  {6,22},{7,22},{18,22},{19,22},{20,22},{26,22},{29,22},{30,22},{32,22},{33,22},{35,22},{36,22},{37,22},
  {42,22},{43,22},{44,22},{57,22},{62,22},{70,22},
  {4,23},{6,23},{7,23},{15,23},{23,23},{24,23},{29,23},{30,23},{35,23},{36,23},
  {45,23},{46,23},{56,23},{62,23},{70,23},
  {21,24},{22,24},{28,24},{29,24},{33,24},{34,24},{35,24},{38,24},{39,24},
  {43,24},{44,24},{47,24},{48,24},{49,24},{69,24},
  {10,25},{11,25},{18,25},{19,25},{26,25},{27,25},{37,25},{38,25},{39,25},
  {42,25},{43,25},{47,25},{50,25},{51,25},{52,25},{53,25},{61,25},{69,25},
  {17,26},{23,26},{24,26},{30,26},{31,26},{36,26},{40,26},{41,26},{42,26},{46,26},{47,26},{65,26},{69,26},
  {7,27},{8,27},{14,27},{15,27},{16,27},{22,27},{35,27},{41,27},{46,27},{59,27},{60,27},{65,27},{69,27},
  {13,28},{21,28},{45,28},{62,28},{63,28},{65,28},
  {12,29},{33,29},{34,29},{39,29},{44,29},
  {18,30},{26,30},{27,30},{32,30},{33,30},{39,30},{44,30},{68,30},{69,30},
  {9,31},{10,31},{17,31},{25,31},{26,31},{38,31},
  {31,32},{43,32},
  {37,33},{15,34},
};
#define TUSCANY_LEN ((int)(sizeof(TUSCANY)/sizeof(TUSCANY[0])))

/* ================================================================ */
/*  GLOBALS                                                          */
/* ================================================================ */

static Window     *s_window;
static Layer      *s_canvas_layer;
static Layer      *s_clock_layer;
static TextLayer  *s_widget_layers[3];
static char        s_widget_bufs[3][16];

static int  s_clockstyle  = CLOCK_DOT;
static int  s_widgets[3]  = {WIDGET_STEPS, WIDGET_BPM, WIDGET_BATTERY};

/* ================================================================ */
/*  SKY COLOUR                                                       */
/* ================================================================ */

static uint8_t u8lerp(uint8_t a, uint8_t b, int t, int d) {
  return (uint8_t)(a + (int)(b - a) * t / d);
}

static GColor sky_color(int hour, int min) {
  int hm = hour * 60 + min;
  if      (hm >= 600 && hm <= 960)  return GColorFromRGB(0xb4,0xc8,0xd4); /* day */
  else if (hm > 420  && hm < 600) { int t=hm-420,d=180; return GColorFromRGB(u8lerp(0x0a,0xb4,t,d),u8lerp(0x0a,0xc8,t,d),u8lerp(0x1a,0xd4,t,d)); } /* dawn */
  else if (hm > 960  && hm < 1140){ int t=hm-960,d=180; return GColorFromRGB(u8lerp(0xb4,0x1a,t,d),u8lerp(0xc8,0x08,t,d),u8lerp(0xd4,0x08,t,d)); } /* dusk */
  else if (hm >= 1140&& hm < 1260){ int t=hm-1140,d=120;return GColorFromRGB(u8lerp(0x1a,0x02,t,d),u8lerp(0x08,0x02,t,d),u8lerp(0x08,0x05,t,d)); } /* late */
  return GColorFromRGB(0x02,0x02,0x05); /* night */
}

static bool is_daytime(int hour)  { return hour >= 7  && hour <= 18; }
static bool is_nighttime(int hour){ return hour >  20 || hour < 5;   }

static GColor fg_color(int hour) {
  return is_daytime(hour) ? GColorBlack : GColorWhite;
}

/* ================================================================ */
/*  ARC POSITION — wide shallow arc near top of sky                 */
/* ================================================================ */

static void arc_pos(int t_num, int t_den, int *ox, int *oy) {
  *ox = 4 + (SCREEN_W - 8) * t_num / t_den;
  int startY = SKY_H * 25 / 100;  /* 27px */
  int peakY  = SKY_H *  8 / 100;  /*  9px */
  int rise   = (startY - peakY) * 4 * t_num / t_den * (t_den - t_num) / t_den;
  *oy = startY - rise;
  if (*oy < 8) *oy = 8;
}

/* ================================================================ */
/*  SUN                                                              */
/* ================================================================ */

static void draw_sun(GContext *ctx, int cx, int cy, GColor col) {
  graphics_context_set_fill_color(ctx, col);
  static const int8_t ring[][2] = {
    {-1,-3},{0,-3},{1,-3},{-2,-2},{2,-2},{-3,-1},{3,-1},
    {-3,0},{3,0},{-3,1},{3,1},{-2,2},{2,2},{-1,3},{0,3},{1,3}
  };
  static const int8_t rays[][2] = {
    {-5,0},{-6,0},{5,0},{6,0},{0,-5},{0,-6},{0,5},{0,6},
    {-4,-4},{-5,-5},{4,-4},{5,-5},{-4,4},{-5,5},{4,4},{5,5}
  };
  for (int i=0;i<16;i++) graphics_fill_rect(ctx,GRect(cx+ring[i][0]*SP,cy+ring[i][1]*SP,SP,SP),0,GCornerNone);
  for (int i=0;i<16;i++) graphics_fill_rect(ctx,GRect(cx+rays[i][0]*SP,cy+rays[i][1]*SP,SP,SP),0,GCornerNone);
}

/* ================================================================ */
/*  MOON — lit pixels only, phase-based                             */
/* ================================================================ */

static void draw_moon(GContext *ctx, int cx, int cy, int phase_num, int phase_den, GColor col) {
  static const int8_t disc[][2] = {
    {-1,-3},{0,-3},{1,-3},
    {-2,-2},{-1,-2},{0,-2},{1,-2},{2,-2},
    {-3,-1},{-2,-1},{-1,-1},{0,-1},{1,-1},{2,-1},{3,-1},
    {-3,0},{-2,0},{-1,0},{0,0},{1,0},{2,0},{3,0},
    {-3,1},{-2,1},{-1,1},{0,1},{1,1},{2,1},{3,1},
    {-2,2},{-1,2},{0,2},{1,2},{2,2},
    {-1,3},{0,3},{1,3}
  };
  graphics_context_set_fill_color(ctx, col);
  for (int i=0;i<37;i++) {
    int x = disc[i][0];
    bool lit;
    if (phase_num * 2 <= phase_den) {
      int term = (phase_num * 6 / phase_den) - 3;
      lit = x >= term;
    } else {
      int term = ((phase_num - phase_den/2) * 6 / phase_den) - 3;
      lit = x <= term;
    }
    if (lit) graphics_fill_rect(ctx,GRect(cx+x*SP,cy+disc[i][1]*SP,SP,SP),0,GCornerNone);
  }
}

/* ================================================================ */
/*  STARS                                                            */
/* ================================================================ */

static const uint8_t STAR_X[] = {4,12,22,35,48,58,68,80,92,102,112,124,134,8,28,44,64,76,88,108,118,132,18,52,96,128};
static const uint8_t STAR_Y[] = {8,18,6,14,4,20,10,16,4,12,8,18,6,24,28,22,30,26,20,24,28,14,32,18,28,22};
#define N_STARS 26

static void draw_stars(GContext *ctx, int seed) {
  for (int i=0;i<N_STARS;i++) {
    bool big = ((STAR_X[i]*7 + STAR_Y[i]*13 + seed) % 5 == 0);
    graphics_context_set_fill_color(ctx, GColorWhite);
    if (big) {
      graphics_fill_rect(ctx,GRect(STAR_X[i]-1,STAR_Y[i],3,1),0,GCornerNone);
      graphics_fill_rect(ctx,GRect(STAR_X[i],STAR_Y[i]-1,1,3),0,GCornerNone);
    } else {
      graphics_fill_rect(ctx,GRect(STAR_X[i],STAR_Y[i],1,1),0,GCornerNone);
    }
  }
}

/* ================================================================ */
/*  TUSCANY SCENE                                                    */
/* ================================================================ */

static void draw_tuscany(GContext *ctx, GColor col) {
  int oy = SKY_H - 36 * SP;  /* scene base aligned to horizon */
  graphics_context_set_fill_color(ctx, col);
  for (int i=0;i<TUSCANY_LEN;i++)
    graphics_fill_rect(ctx, GRect(TUSCANY[i][0]*SP, oy+TUSCANY[i][1]*SP, SP, SP), 0, GCornerNone);
}

/* ================================================================ */
/*  CANVAS UPDATE                                                    */
/* ================================================================ */

static void canvas_update(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int hour = t->tm_hour, min = t->tm_min;
  int hm   = hour * 60 + min;
  bool night = is_nighttime(hour);

  GColor sky = sky_color(hour, min);
  GColor fg  = fg_color(hour);

  /* Fill entire screen — strip uses same colour automatically */
  graphics_context_set_fill_color(ctx, sky);
  graphics_fill_rect(ctx, GRect(0,0,SCREEN_W,SCREEN_H), 0, GCornerNone);

  /* Stars at night */
  if (night || hour < 6)
    draw_stars(ctx, t->tm_yday);

  /* Sun: 6:00–20:00 */
  if (hm >= 360 && hm <= 1200) {
    int sx, sy;
    arc_pos(hm - 360, 840, &sx, &sy);
    draw_sun(ctx, sx, sy, fg);
  }

  /* Moon: 20:00–6:00 (opposite arc, right→left) */
  {
    int m_num = -1;
    if      (hm >= 1200) m_num = hm - 1200;
    else if (hm <= 360)  m_num = hm + 240;   /* 0–6am: offset by 240 */
    int m_den = 600;
    if (m_num >= 0 && m_num <= m_den) {
      int mx, my;
      arc_pos(m_den - m_num, m_den, &mx, &my);  /* reversed */
      int phase = t->tm_yday % 30;
      GColor mc = night ? GColorLightGray : GColorDarkGray;
      draw_moon(ctx, mx, my, phase, 30, mc);
    }
  }

  /* Tuscany scene */
  draw_tuscany(ctx, fg);
}

/* ================================================================ */
/*  CLOCK — dot, pixel, digital                                     */
/* ================================================================ */

static void plot_dot(GContext *ctx, int x, int y, int sz) {
  int gx = ((x-CLOCK_CX+PX_STEP/2)/PX_STEP)*PX_STEP+CLOCK_CX;
  int gy = ((y-CLOCK_CY+PX_STEP/2)/PX_STEP)*PX_STEP+CLOCK_CY;
  graphics_fill_rect(ctx, GRect(gx,gy,sz,sz), 0, GCornerNone);
}

static void draw_dot_clock(GContext *ctx, int hour, int min) {
  for (int i=0;i<60;i++) {
    int32_t a=TRIG_MAX_ANGLE*i/60;
    int x=CLOCK_CX+(int)(sin_lookup(a)*CLOCK_R/TRIG_MAX_RATIO);
    int y=CLOCK_CY-(int)(cos_lookup(a)*CLOCK_R/TRIG_MAX_RATIO);
    graphics_context_set_fill_color(ctx, i%5==0 ? GColorChromeYellow : GColorDarkGray);
    plot_dot(ctx,x,y, i%5==0 ? 3 : 2);
  }
  graphics_context_set_fill_color(ctx, GColorRed);
  { int32_t a=TRIG_MAX_ANGLE*min/60;
    for (int r=PX_STEP*2;r<=CLOCK_R-2;r+=PX_STEP)
      plot_dot(ctx,CLOCK_CX+(int)(sin_lookup(a)*r/TRIG_MAX_RATIO),CLOCK_CY-(int)(cos_lookup(a)*r/TRIG_MAX_RATIO),2); }
  graphics_context_set_fill_color(ctx, GColorIslamicGreen);
  { int32_t a=TRIG_MAX_ANGLE*(hour*60+min)/720;
    int len=CLOCK_R*58/100;
    for (int r=PX_STEP*2;r<=len;r+=PX_STEP)
      plot_dot(ctx,CLOCK_CX+(int)(sin_lookup(a)*r/TRIG_MAX_RATIO),CLOCK_CY-(int)(cos_lookup(a)*r/TRIG_MAX_RATIO),2); }
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx,GRect(CLOCK_CX-2,CLOCK_CY-2,4,4),0,GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,GRect(CLOCK_CX-1,CLOCK_CY-1,2,2),0,GCornerNone);
}

static void draw_pixel_clock(GContext *ctx, int hour, int min) {
  for (int i=0;i<60;i++) {
    int32_t a=TRIG_MAX_ANGLE*i/60;
    int x=CLOCK_CX+(int)(sin_lookup(a)*CLOCK_R/TRIG_MAX_RATIO);
    int y=CLOCK_CY-(int)(cos_lookup(a)*CLOCK_R/TRIG_MAX_RATIO);
    int gx=((x-CLOCK_CX)/PX_STEP)*PX_STEP+CLOCK_CX;
    int gy=((y-CLOCK_CY)/PX_STEP)*PX_STEP+CLOCK_CY;
    graphics_context_set_fill_color(ctx, i%5==0 ? GColorChromeYellow : GColorDarkGray);
    graphics_fill_rect(ctx,GRect(gx,gy,PX_SZ,PX_SZ),0,GCornerNone);
  }
  graphics_context_set_fill_color(ctx, GColorRed);
  { int32_t a=TRIG_MAX_ANGLE*min/60;
    for (int r=PX_STEP*2;r<=CLOCK_R-2;r+=PX_STEP) {
      int gx=((CLOCK_CX+(int)(sin_lookup(a)*r/TRIG_MAX_RATIO)-CLOCK_CX)/PX_STEP)*PX_STEP+CLOCK_CX;
      int gy=((CLOCK_CY-(int)(cos_lookup(a)*r/TRIG_MAX_RATIO)-CLOCK_CY)/PX_STEP)*PX_STEP+CLOCK_CY;
      graphics_fill_rect(ctx,GRect(gx,gy,PX_SZ,PX_SZ),0,GCornerNone); }
  }
  graphics_context_set_fill_color(ctx, GColorIslamicGreen);
  { int32_t a=TRIG_MAX_ANGLE*(hour*60+min)/720;
    int len=CLOCK_R*58/100;
    for (int r=PX_STEP*2;r<=len;r+=PX_STEP) {
      int gx=((CLOCK_CX+(int)(sin_lookup(a)*r/TRIG_MAX_RATIO)-CLOCK_CX)/PX_STEP)*PX_STEP+CLOCK_CX;
      int gy=((CLOCK_CY-(int)(cos_lookup(a)*r/TRIG_MAX_RATIO)-CLOCK_CY)/PX_STEP)*PX_STEP+CLOCK_CY;
      graphics_fill_rect(ctx,GRect(gx,gy,PX_SZ,PX_SZ),0,GCornerNone); }
  }
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx,GRect(CLOCK_CX-2,CLOCK_CY-2,4,4),0,GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,GRect(CLOCK_CX-1,CLOCK_CY-1,2,2),0,GCornerNone);
}

static void clock_layer_update(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int hour = t->tm_hour % 12, min = t->tm_min;

  if (s_clockstyle == CLOCK_DIGITAL) {
    char buf[6];
    bool h24 = clock_is_24h_style();
    int dh = h24 ? t->tm_hour : (t->tm_hour%12==0 ? 12 : t->tm_hour%12);
    snprintf(buf, sizeof(buf), "%02d:%02d", dh, min);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, buf,
      fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
      GRect(0, CLOCK_CY-16, CLOCK_SIZE, 32),
      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    if (!h24) {
      graphics_context_set_text_color(ctx, GColorLightGray);
      graphics_draw_text(ctx, t->tm_hour < 12 ? "AM" : "PM",
        fonts_get_system_font(FONT_KEY_GOTHIC_09),
        GRect(0, CLOCK_CY+14, CLOCK_SIZE, 12),
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
  } else if (s_clockstyle == CLOCK_PIXEL) {
    draw_pixel_clock(ctx, hour, min);
  } else {
    draw_dot_clock(ctx, hour, min);
  }
}

/* ================================================================ */
/*  HEALTH WIDGETS                                                   */
/* ================================================================ */

static const char *widget_label(int w) {
  switch(w) {
    case WIDGET_STEPS:    return "STP";
    case WIDGET_BPM:      return "BPM";
    case WIDGET_BATTERY:  return "BAT";
    case WIDGET_CALORIES: return "CAL";
    case WIDGET_DISTANCE: return "DST";
    default:              return "---";
  }
}

static void update_widget(int slot) {
  char val[10] = "---";
  switch(s_widgets[slot]) {
    case WIDGET_STEPS: {
      uint32_t v = (uint32_t)health_service_sum_today(HealthMetricStepCount);
      snprintf(val,sizeof(val),"%lu",(unsigned long)v); break; }
    case WIDGET_BPM: {
      uint32_t v = (uint32_t)health_service_peek_current_value(HealthMetricHeartRateBPM);
      snprintf(val,sizeof(val),"%lu",(unsigned long)v); break; }
    case WIDGET_BATTERY:
      snprintf(val,sizeof(val),"%d%%",(int)battery_state_service_peek().charge_percent); break;
    case WIDGET_CALORIES: {
      uint32_t v = (uint32_t)health_service_sum_today(HealthMetricActiveKCalories);
      snprintf(val,sizeof(val),"%lu",(unsigned long)v); break; }
    case WIDGET_DISTANCE: {
      uint32_t m = (uint32_t)health_service_sum_today(HealthMetricWalkedDistanceMeters);
      if (m>=1000) snprintf(val,sizeof(val),"%lu.%lukm",(unsigned long)(m/1000),(unsigned long)((m%1000)/100));
      else         snprintf(val,sizeof(val),"%lum",(unsigned long)m);
      break; }
  }
  snprintf(s_widget_bufs[slot],sizeof(s_widget_bufs[slot]),"%s %s",widget_label(s_widgets[slot]),val);
  text_layer_set_text(s_widget_layers[slot], s_widget_bufs[slot]);
}

static void update_all_widgets(void) {
  for (int i=0;i<3;i++) update_widget(i);
}

/* ================================================================ */
/*  SERVICES                                                         */
/* ================================================================ */

static void tick_handler(struct tm *t, TimeUnits changed) {
  update_all_widgets();
  layer_mark_dirty(s_canvas_layer);
  layer_mark_dirty(s_clock_layer);
}

static void battery_handler(BatteryChargeState state) { update_widget(2); }

/* ================================================================ */
/*  APPMESSAGE                                                       */
/* ================================================================ */

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *tp;
  tp = dict_find(iter, MSG_CLOCKSTYLE);
  if (tp) {
    int v = (int)tp->value->int32;
    if (v>=0&&v<=2) { s_clockstyle=v; persist_write_int(PERSIST_CLOCKSTYLE,v); layer_mark_dirty(s_clock_layer); }
  }
  int keys[3] = {MSG_WIDGET0,MSG_WIDGET1,MSG_WIDGET2};
  int pkeys[3]= {PERSIST_WIDGET0,PERSIST_WIDGET1,PERSIST_WIDGET2};
  for (int i=0;i<3;i++) {
    tp = dict_find(iter, keys[i]);
    if (tp) {
      int v=(int)tp->value->int32;
      if (v>=0&&v<=4){ s_widgets[i]=v; persist_write_int(pkeys[i],v); update_widget(i); }
    }
  }
}

/* ================================================================ */
/*  WINDOW                                                           */
/* ================================================================ */

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);

  s_canvas_layer = layer_create(GRect(0,0,SCREEN_W,SCREEN_H));
  layer_set_update_proc(s_canvas_layer, canvas_update);
  layer_add_child(root, s_canvas_layer);

  GFont wfont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  for (int i=0;i<3;i++) {
    s_widget_layers[i] = text_layer_create(GRect(WIDGET_X, WIDGET_Y+i*17, 80, 18));
    text_layer_set_background_color(s_widget_layers[i], GColorClear);
    text_layer_set_text_color(s_widget_layers[i], GColorLightGray);
    text_layer_set_font(s_widget_layers[i], wfont);
    layer_add_child(root, text_layer_get_layer(s_widget_layers[i]));
  }

  s_clock_layer = layer_create(GRect(CLOCK_X,CLOCK_Y,CLOCK_SIZE,CLOCK_SIZE));
  layer_set_update_proc(s_clock_layer, clock_layer_update);
  layer_add_child(root, s_clock_layer);

  update_all_widgets();
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_clock_layer);
  for (int i=0;i<3;i++) text_layer_destroy(s_widget_layers[i]);
}

/* ================================================================ */
/*  INIT / DEINIT                                                    */
/* ================================================================ */

static void init(void) {
  s_clockstyle = persist_exists(PERSIST_CLOCKSTYLE) ? persist_read_int(PERSIST_CLOCKSTYLE) : CLOCK_DOT;
  s_widgets[0] = persist_exists(PERSIST_WIDGET0)    ? persist_read_int(PERSIST_WIDGET0)    : WIDGET_STEPS;
  s_widgets[1] = persist_exists(PERSIST_WIDGET1)    ? persist_read_int(PERSIST_WIDGET1)    : WIDGET_BPM;
  s_widgets[2] = persist_exists(PERSIST_WIDGET2)    ? persist_read_int(PERSIST_WIDGET2)    : WIDGET_BATTERY;
  if (s_clockstyle<0||s_clockstyle>2) s_clockstyle=CLOCK_DOT;
  for (int i=0;i<3;i++) if (s_widgets[i]<0||s_widgets[i]>4) s_widgets[i]=i;

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window,(WindowHandlers){.load=window_load,.unload=window_unload});
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  app_message_register_inbox_received(inbox_received);
  app_message_open(128, 64);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) { init(); app_event_loop(); deinit(); }
