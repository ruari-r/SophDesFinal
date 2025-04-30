#include <stdbool.h>
#include <stdint.h>
// #include <xil_printf.h>

#define BUTTONS (*(unsigned volatile *)0x40000000)
#define JA (*(unsigned volatile *)0x40001000)
#define JA_DDR (*(unsigned volatile *)0x40001004)
#define JB (*(unsigned volatile *)0x40002000)
#define JB_DDR (*(unsigned volatile *)0x40002004)
#define JC (*(unsigned volatile *)0x40003000)
#define JC_DDR (*(unsigned volatile *)0x40003004)
#define JXADC (*(unsigned volatile *)0x40004000)
#define JXADC_DDR (*(unsigned volatile *)0x40004004)
#define LEDS (*(unsigned volatile *)0x40005000)
#define ANODES (*(unsigned volatile *)0x40006000)
#define SEVEN_SEG (*(unsigned volatile *)0x40006008)
#define SWITCHES (*(unsigned volatile *)0x40007000)
#define UART (*(unsigned volatile *)0x40008000)
#define TIMER_0 (*(unsigned volatile *)0x40009000)
#define TIMER_1 (*(unsigned volatile *)0x4000A000)
#define TIMER_2 (*(unsigned volatile *)0x4000B000)
#define TIMER_3 (*(unsigned volatile *)0x4000C000)

// Memory Access Offsets - Buttons
#define BTND_OFFSET 0 // BTN[0]
#define BTNR_OFFSET 1 // BTN[1]
#define BTNL_OFFSET 2 // BTN[2]
#define BTNU_OFFSET 3 // BTN[3]

// Memory Access Offsets - Motors
#define L_PWM_OFFSET 0  // JC[0]
#define LEFT2_OFFSET 1  // JC[1]
#define LEFT1_OFFSET 2  // JC[2]
#define R_PWM_OFFSET 3  // JC[3]
#define RIGHT2_OFFSET 4 // JC[4]
#define RIGHT1_OFFSET 5 // JC[5]

// Memory Access Offsets - Quad Encs
#define L1_QUAD_ENC_OFFSET 0 // JA[0]
#define R1_QUAD_ENC_OFFSET 1 // JA[1]

// Constants
#define PWM_TOP 255
#define INCREMENT 8
#define QUAD_ENC_TOP 10000
#define DUTY_MOTION_START 0X30
#define TIMEOUT = 10000000 // How long to wait until assuming trig was lost
#define CNT_PER_REV 340
#define CNT_PER_INCH 45
#define HW_TIME_PER_SEC 565001
#define TIME_TO_TARGET 5 // seconds
#define BASE_DUTY_CYCLE 0xBF
#define KP 0.1
#define KI 0.05
#define KD 0
#define PI 3.141592653589793
#define ITP (uint32_t *)

const volatile uint32_t TCSR_OFFEST = 0;
const volatile uint32_t TLR_OFFEST = 1;
const volatile uint32_t TCR_OFFSET = 2;

// Addresses for all the timers, ITP casts the int to uint32_t ptr
const uint32_t *TIMERS[] = {ITP 0x40009000, ITP 0x40009100, ITP 0x4000A000, ITP 0x4000A100,
                            ITP 0x4000B000, ITP 0x4000B100, ITP 0x4000C000, ITP 0x4000C100};

// Type definitions
typedef enum motion_type{
  left,
  right,
  straight,
  stop,
  idle,
} motion_type;

typedef enum state_type {
  wait_for_coords,
  delay_5,
  check_y_turn,
  turn_180,
  drive_to_y,
  x_turn,
  drive_to_x,
  hold_position
} state_type;

// Seven Segment Display LUT
uint8_t sevenSegLUT[10] = {
    0xC0, // 0 --> 1100 0000
    0xF9, // 1 --> 1111 1001
    0xA4, // 2 --> 1010 0100
    0xB0, // 3 --> 1100 0000
    0x99, // 4 --> 1001 1001
    0x92, // 5 --> 1001 0010
    0x82, // 6 --> 1000 0010
    0xF8, // 7 --> 1111 1000
    0x80, // 8 --> 1000 0000 
    0x90, // 9 --> 1001 0000
};

// Function declarations - implemented below
void init_program(); // One Time Initializations
_Bool delay_1s();
_Bool delay_5s();
_Bool delay_half_sec();
void timer_2us(unsigned t);
void set_trig_pin();
void clear_trig_pin();
_Bool read_echo_pin();
void restart_timer0();
uint32_t get_timer0_value_us();
uint32_t *convert_timer_to_hex_address(uint8_t timer_number);
void configure_timers();
void start_stopwatch(uint8_t timer_number);
uint32_t read_stopwatch(uint8_t timer_number);
void show_sseg(uint8_t *sevenSegValue);
_Bool UpButton_pressed();
_Bool DownButton_pressed();
_Bool LeftButton_pressed();
_Bool RightButton_pressed();
uint32_t read_L1_quad_enc(_Bool reset);
uint32_t read_R1_quad_enc(_Bool reset);
void set_motion_type(motion_type mode);
void PID_Controller(uint32_t L1, uint32_t R1);
void drive_straight(uint32_t inches, uint16_t coords);
void turn(uint32_t degrees, uint16_t coords);
uint16_t get_coords();
uint16_t disp_coords(uint16_t coords);
void celebration();

// Global Variables for Duty Cycles
uint8_t g_LeftDutyCycle = 0x00;
uint8_t g_RightDutyCycle = 0x00;

// ###########################################################################################################

int main() {
  // One time initialization
  init_program();

  // set up PMOD DDRs
  JA_DDR = 0x03;
  JB_DDR = 0x08;
  JC_DDR = 0x00;

  ANODES = 0x00;

  state_type state = wait_for_coords;
  state_type next_state = wait_for_coords;
  

  while (1) {  
    
  }
}

// ###########################################################################################################

// Functions Initialization
void init_program() { 
  configure_timers();
  set_motion_type(straight);
}

// Functions for the Ultrasonic Sensor
void set_trig_pin() {
  JB |= 0x01; // 0000 0001 set trig pin
}

void clear_trig_pin() {
  JB &= 0xFE; // 1111 1110 clear trig pin
}

_Bool read_echo_pin() {
  bool echo = JB & 0x08; // Read echo signal from pin JB[4]
  return echo;           // return echo pin value
}

// Function Implementation - Software Delays
_Bool delay_1s() {
  const uint32_t TOP = 2870000;
  static uint32_t count = 0;
  if (count == TOP) {
    count = 0;
    return true;
  }
  count++;
  return false;
}

_Bool delay_5s() {
  const uint32_t TOP = 14350000;
  static uint32_t count = 0;
  if (count == TOP) {
    count = 0;
    return true;
  }
  count++;
  return false;
}

_Bool delay_half_sec() {
  const uint32_t TOP = 1350000;
  static uint32_t count = 0;
  if (count == TOP) {
    count = 0;
    return true;
  }
  count++;
  return false;
}

void timer_2us(unsigned t) {
  volatile unsigned cntr1;
  while (t--)
    for (cntr1 = 0; cntr1 < 8; cntr1++)
      ;
}

// Function implementation - Hardware Timers
void restart_timer0() {
  // TCSR0 &= ~(1<<7);
  // TCSR0 |= 1<<5;
  // TCSR0 &= ~(1<<5);
  // TCSR0 |= (1<<7);
  start_stopwatch(0);
}

uint32_t get_timer0_value_us() {
  // uint32_t count = TCR0;
  // TCSR0 |= 1<<8;
  // uint32_t time_us = count/100;
  // return time_us;
  return read_stopwatch(0);
}

uint32_t *convert_timer_to_hex_address(uint8_t timer_number) {
  if (timer_number > 7)
    return 0;
  uint32_t *timer_base_address =
      (uint32_t *)0x40009000 + ((timer_number / 2) * 0x0400);
  if (timer_number & 1)
    timer_base_address += 0x40;
  return timer_base_address;
}

void configure_timers() {
  for (int i = 0; i < 8; i++) {
    uint32_t *timer_base_address = convert_timer_to_hex_address(i);
    uint32_t *tcr = timer_base_address + TCR_OFFSET;
    uint32_t *tcsr = timer_base_address + TCSR_OFFEST;
    *(tcr) = 0x00000000;
    *(tcsr) = 0b010010010001;
  }
}

void start_stopwatch(uint8_t timer_number) {
  if (timer_number > 7)
    return;
  uint32_t *timer_base_address = convert_timer_to_hex_address(timer_number);
  volatile uint32_t *tcsr = timer_base_address + TCSR_OFFEST;
  *tcsr &= ~(1 << 7);
  *tcsr |= 1 << 5;
  *tcsr &= ~(1 << 5);
  *tcsr |= 1 << 7;
}

uint32_t read_stopwatch(uint8_t timer_number) {
  if (timer_number > 7)
    return 0;
  uint32_t *timer_base_address = convert_timer_to_hex_address(timer_number);
  volatile uint32_t *tcr = timer_base_address + TCR_OFFSET;
  return (*tcr) / 100;
}

// Function implementation - SSeg
void show_sseg(uint8_t *sevenSegValue) {
  static uint8_t anodeCnt = 0;
  if (read_stopwatch(1) > 1000) {
    anodeCnt++;
    ANODES = ~(1 << (anodeCnt % 4));
    SEVEN_SEG = sevenSegValue[anodeCnt % 4];
    start_stopwatch(1);
  }
}

// Function implementation - Buttons
_Bool UpButton_pressed() {
  static _Bool current_upbtn_value = 0, previous_upbtn_value = 0;
  previous_upbtn_value = current_upbtn_value;
  current_upbtn_value = BUTTONS & (1 << BTNU_OFFSET);
  return (current_upbtn_value && !previous_upbtn_value);
}

_Bool DownButton_pressed() {
  static _Bool current_downbtn_value = 0, previous_downbtn_value = 0;
  previous_downbtn_value = current_downbtn_value;
  current_downbtn_value = BUTTONS & (1 << BTND_OFFSET);
  return (current_downbtn_value && !previous_downbtn_value);
}

_Bool LeftButton_pressed() {
  static _Bool current_leftbtn_value = 0, previous_leftbtn_value = 0;
  previous_leftbtn_value = current_leftbtn_value;
  current_leftbtn_value = BUTTONS & (1 << BTNL_OFFSET);
  return (current_leftbtn_value && !previous_leftbtn_value);
}

_Bool RightButton_pressed() {
  static _Bool current_rightbtn_value = 0, previous_rightbtn_value = 0;
  previous_rightbtn_value = current_rightbtn_value;
  current_rightbtn_value = BUTTONS & (1 << BTNR_OFFSET);
  return (current_rightbtn_value && !previous_rightbtn_value);
}

// Function implementation - Quadrature Encoders
uint32_t read_L1_quad_enc(_Bool reset)
{
    static uint32_t cnt = 0;
    static _Bool quad_enc_last_state = 0;

    if (reset)
    {
        cnt = 0;
    }

    if ((quad_enc_last_state == 0) && (JA & (1 << L1_QUAD_ENC_OFFSET)))
    {
        cnt++;
    }
    quad_enc_last_state = JA & (1 << L1_QUAD_ENC_OFFSET);
    return cnt;
}

uint32_t read_R1_quad_enc(_Bool reset)
{
    static uint32_t cnt = 0;
    static _Bool quad_enc_last_state = 0;

    if (reset)
    {
        cnt = 0;
    }
    if ((quad_enc_last_state == 0) && (JA & (1 << R1_QUAD_ENC_OFFSET)))
    {
        cnt++;
    }
    quad_enc_last_state = JA & (1 << R1_QUAD_ENC_OFFSET);
    return cnt;
}

void set_motion_type(motion_type mode) {
  switch (mode) {
    case (right):
    JC |= ((1 << LEFT1_OFFSET) | (1 << RIGHT2_OFFSET));
    JC &= ~((1 << LEFT2_OFFSET) | (1 << RIGHT1_OFFSET));
    break;

    case (left):
    JC |= ((1 << LEFT2_OFFSET) | (1 << RIGHT1_OFFSET));
    JC &= ~((1 << LEFT1_OFFSET) | (1 << RIGHT2_OFFSET));
    break;

    case (straight):
    JC |= ((1 << LEFT2_OFFSET) | (1 << RIGHT2_OFFSET));
    JC &= ~((1 << LEFT1_OFFSET) | (1 << RIGHT1_OFFSET));
    break;

    case (stop):
    JC |= ((1 << LEFT2_OFFSET) | (1 << LEFT1_OFFSET) | (1 << RIGHT2_OFFSET) | (1 << RIGHT1_OFFSET));
    break;

    case (idle):
    JC &= ~((1 << LEFT2_OFFSET) | (1 << LEFT1_OFFSET) | (1 << RIGHT2_OFFSET) | (1 << RIGHT1_OFFSET));
    break;

    default:
    JC &= ~((1 << LEFT2_OFFSET) | (1 << LEFT1_OFFSET) | (1 << RIGHT2_OFFSET) | (1 << RIGHT1_OFFSET));
  }
}

uint8_t scale_correction(int32_t raw_correction) {
  float raw_correction_mag = (raw_correction >= 0) ? raw_correction : -raw_correction;
  if (raw_correction >= 255.00f) return 0xFF;
  else return (uint8_t) raw_correction_mag;
}

void PID_Controller(uint32_t L1, uint32_t R1) {
  static int32_t error_sum = 0, error_prev = 0;
  int32_t error = (int32_t)L1 - (int32_t)R1;
  error_sum += error;
	
  int32_t error_diff = error - error_prev;
	
  float correction = KP*error + KI*error_sum + KD*error_diff;
  uint8_t correction_scaled = scale_correction(correction);

  if (error > 0) {
    if (g_RightDutyCycle + correction_scaled > 0xFF) {
      g_RightDutyCycle = 0xFF;
    }
    else {
      g_RightDutyCycle += correction_scaled;
    }

	if (g_LeftDutyCycle - correction_scaled < 0xA0) {
      g_LeftDutyCycle = 0xA0;
    }
	else {
      g_LeftDutyCycle -= correction_scaled;
    }
  } 
  else if (error < 0) { 
    if (g_LeftDutyCycle + correction_scaled > 0xFF) {
      g_LeftDutyCycle = 0xFF;
    }
    else {
      g_LeftDutyCycle += correction_scaled;
    }

	if (g_RightDutyCycle - correction_scaled < 0xA0) {
      g_RightDutyCycle = 0xA0;
    }
	else {
      g_RightDutyCycle -= correction_scaled;
    }
  }
	
	error_prev = error;
}

// Functions for navigation
void drive_straight(uint32_t inches, uint16_t coords) {
  read_L1_quad_enc(1);
  read_R1_quad_enc(1);  
  
  g_LeftDutyCycle = 0xCF;
  g_RightDutyCycle = 0xCF;

  // Inches to encoder count
  uint32_t distance_enc = inches*CNT_PER_INCH;
  uint8_t pwmCnt = 0;
  
  while (read_L1_quad_enc(0) < distance_enc || read_R1_quad_enc(0) < distance_enc) {
    if (read_L1_quad_enc(0) < distance_enc && pwmCnt <= g_LeftDutyCycle) JC |= (1 << L_PWM_OFFSET);
    else JC &= ~(1 << L_PWM_OFFSET);

    if (read_R1_quad_enc(0) < distance_enc && pwmCnt <= g_RightDutyCycle) JC |= (1 << R_PWM_OFFSET);
    else JC &= ~(1 << R_PWM_OFFSET);

    if (++pwmCnt == PWM_TOP) pwmCnt = 0;

    PID_Controller(read_L1_quad_enc(0), read_R1_quad_enc(0));
    disp_coords(coords);
    LEDS = (g_LeftDutyCycle << 8) | g_RightDutyCycle;
  } 
}

void turn(uint32_t degrees, uint16_t coords) {   
    read_L1_quad_enc(1);
    read_R1_quad_enc(1);

    g_RightDutyCycle = 0xBF;
    g_LeftDutyCycle = 0xBF;
    
    // Degrees to Arc Length
    // Correction for 180deg turns
    if (degrees == 180) degrees += 12;
    float turn_fraction = (float) degrees/360;
    float arc_length_inches = turn_fraction*PI*6.625;
    uint32_t arc_length_enc = arc_length_inches*CNT_PER_INCH;
    uint8_t pwmCnt = 0;

    while (read_L1_quad_enc(0) < arc_length_enc || read_R1_quad_enc(0) < arc_length_enc) {
        if (read_L1_quad_enc(0) < arc_length_enc && pwmCnt <= g_LeftDutyCycle) JC |= (1 << L_PWM_OFFSET);
        else JC &= ~(1 << L_PWM_OFFSET);

        if (read_R1_quad_enc(0) < arc_length_enc && pwmCnt <= g_RightDutyCycle) JC |= (1 << R_PWM_OFFSET);
        else JC &= ~(1 << R_PWM_OFFSET);

        if (++pwmCnt == PWM_TOP) pwmCnt = 0;

        PID_Controller(read_L1_quad_enc(0), read_R1_quad_enc(0));
        disp_coords(coords);
        LEDS = (g_LeftDutyCycle << 8) | g_RightDutyCycle;
    }
}

uint16_t get_coords() {
  // TODO
  // Parse out negatives and only grab enough bits for 40
  return SWITCHES & 0xbfbf;
}

uint16_t disp_coords(uint16_t coords) {
  uint8_t x_coord, y_coord, x_tens_place, x_ones_place, y_tens_place,
      y_ones_place;
  _Bool is_x_neg = false, is_y_neg = false;
  uint8_t sevenSegValue[4] = {0};

  // Extract x and y
  x_coord = (coords & 0xff00) >> 8;
  y_coord = coords & 0x00ff;

  if (x_coord & 0x80)
    is_x_neg = true;
  if (y_coord & 0x80)
    is_y_neg = true;

  if ((x_coord &= 0x7f) >= 40)
    x_coord = 40;
  if ((y_coord &= 0x7f) >= 40)
    y_coord = 40;

  // Suppose x_coord = 64,
  // 64 % 10 = 4 (ones place)
  // (64 - 4)/10 => 60/10 = 6 (tens place)
  x_ones_place = x_coord % 10;
  x_tens_place = (x_coord - x_ones_place) / 10;

  y_ones_place = y_coord % 10;
  y_tens_place = (y_coord - y_ones_place) / 10;

  // If x is negative, clear first bit of digit to turn on decimal point
  sevenSegValue[3] = (!is_x_neg) ? sevenSegLUT[x_tens_place] : (sevenSegLUT[x_tens_place] & 0x7f);
  sevenSegValue[2] = sevenSegLUT[x_ones_place];
  sevenSegValue[1] = (!is_y_neg) ? sevenSegLUT[y_tens_place] : (sevenSegLUT[y_tens_place] & 0x7f);
  sevenSegValue[0] = sevenSegLUT[y_ones_place];
  show_sseg(&sevenSegValue[0]); // Run this every while(1) iteration

  return (x_coord << 8) | (y_coord);
}

void celebration() {
    static uint16_t led_state = 0xAAAA;
    if (read_stopwatch(2) >= HW_TIME_PER_SEC/2) {
        led_state ^= 0xFFFF;
        start_stopwatch(2);
    }
    LEDS = led_state;    
}
