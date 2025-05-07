#include <stdbool.h>
#include <stdint.h>
#include <xil_printf.h>

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
#define DIST_THRESHOLD 13 //cm
#define TIMEOUT_TICKS (DIST_THRESHOLD*58)
#define PRE_TURN_CORR 7 //inches
#define POST_TURN_CORR 8 //inches
#define CNT_PER_REV 340
#define CNT_PER_INCH 45
#define HW_TIME_PER_SEC 565001
#define US_PER_TICK  1.77f
#define USS_READ_INTERVAL 0.060f // 60ms delay (4m max range)
#define MED_FILT_WINDOW 5
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
typedef enum {
  left,
  right,
  straight,
  stop,
  idle,
} motion_type;

typedef enum {
  wait_to_start,
  delay_3s,
  update_uss,
  initialize_drive,
  drive,
  left_only,
  left_and_front,
  front_only,
  no_left_or_front,
  turn_state,
  pause_half_sec,
  win,
} maze_state;

typedef enum {
  init_drive,
  driving,
  stop_driving
} drive_state;

typedef enum {
  send_trig,  
  clear_trig,
  count_echo_duration, 
  median_filter,
  calculate_distance, 
  cooldown
  } uss_state;

typedef struct {
  uint8_t trig_offset;
  uint8_t echo_offset; 
  uint8_t hw_timer_channel;
  uint32_t raw_echo_high_time;
  uint32_t med_echo_high_time;
} UltrasonicSensor;

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
void set_trig_pin(UltrasonicSensor uss);
void clear_trig_pin(UltrasonicSensor uss);
_Bool read_echo_pin(UltrasonicSensor uss);
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
static inline uint8_t scale_correction(int32_t raw_correction);
static inline void PID_Controller(_Bool reset, uint32_t L1, uint32_t R1);
void drive_straight_distance(uint32_t inches);
void drive_straight(drive_state cmd);
void turn(uint32_t degrees);
void read_2_uss_fsm(UltrasonicSensor * uss1, 
                    UltrasonicSensor * uss2, 
                    // float * dist_1, 
                    // float * dist_2,
                    uint32_t buf1[MED_FILT_WINDOW],
                    uint32_t buf2[MED_FILT_WINDOW]);
void selection_sort(uint32_t intArray[], uint8_t arrayLength);
static inline void swap(uint32_t * pFirst, uint32_t * pSecond);
void celebration();

// Global Variables:
uint8_t g_LeftDutyCycle = 0x00;
uint8_t g_RightDutyCycle = 0x00;
uint32_t g_FrontDist = 0;
uint32_t g_LeftDist = 0;
_Bool g_NewReading = false;

// Median Filtering
static uint32_t front_buf[MED_FILT_WINDOW] = {14, 14, 14, 14, 14};
static uint32_t left_buf[MED_FILT_WINDOW] = {14, 14, 14, 14, 14};
uint8_t buf_write_index = 0; // Used for both front and left, updated simultaneously

// Initialize Ultrasonic Sensors
UltrasonicSensor FrontUSS = {0, 3, 3, 0, 0};
UltrasonicSensor LeftUSS = {1, 2, 4, 0, 0};

// ###########################################################################################################

int main() {
  // One time initialization
  init_program();

  // set up PMOD DDRs
  // 1 = Input, 0 = Output
  JA_DDR = 0x03;
  JB_DDR = 0x00;
  JB_DDR |= ((1<<FrontUSS.echo_offset) | (1<<LeftUSS.echo_offset));
  JC_DDR = 0x00;

  ANODES = 0x00;
  // while (1) {
  //   g_NewReading = false; // Reset new reading flag so that it will only be high if uss fsm sets it
  //   read_2_uss_fsm(&FrontUSS, &LeftUSS, 
  //                 //  &g_FrontDist, &g_LeftDist,
  //                  front_buf, left_buf);
  //   if (g_NewReading) {
  //     // xil_printf("Front: %5d    Left: %5d\n", g_FrontDist, g_LeftDist);
  //   //   xil_printf("Left sensor reading: %d\n",  g_LeftDist);
  //   //   while (!delay_half_sec());
  //   }
    
  //   if (g_FrontDist < 8) {
  //     set_motion_type(stop);
  //   }
  //   else if (g_LeftDist < 13) {
  //     set_motion_type(straight);
  //     drive_straight(driving);
  //   }
  // }
  _Bool btnU = false, btnD = false, btnL = false, btnR = false;
  maze_state state = wait_to_start;
  maze_state next_state;
  maze_state last_state = wait_to_start;
  maze_state ultrasonic_state = left_only;
  maze_state last_ultrasonic = left_only;
  motion_type turn_dir;
  uint8_t pre_turn_corr;

  while (1) {  
    next_state = state; // Ensure we never accidentally leave state without checking
    btnU = UpButton_pressed();
    btnD = DownButton_pressed();
    btnL = LeftButton_pressed();
    btnR = RightButton_pressed();
    g_NewReading = false; // Reset new reading flag so that it will only be high if uss fsm sets it
    read_2_uss_fsm(&FrontUSS, &LeftUSS, 
                //    &g_FrontDist, &g_LeftDist, 
                   front_buf, left_buf);
    if (g_NewReading) {          
      // xil_printf("Front: %5d    Left: %5d\n", g_FrontDist, g_LeftDist); // Debug Prints       
      if (g_FrontDist >= DIST_THRESHOLD && g_LeftDist < DIST_THRESHOLD) {ultrasonic_state = left_only;}
      else if (g_FrontDist < DIST_THRESHOLD && g_LeftDist < DIST_THRESHOLD) {ultrasonic_state = left_and_front;}
      else if (g_FrontDist < DIST_THRESHOLD && g_LeftDist >= DIST_THRESHOLD) {ultrasonic_state = front_only;}
      else if (g_FrontDist >= DIST_THRESHOLD && g_LeftDist >= DIST_THRESHOLD) {ultrasonic_state = no_left_or_front;}
    }
    switch (state) {
    case wait_to_start:
      if (btnU) {
        next_state = delay_3s;
        start_stopwatch(6);
      }
      break;

    case delay_3s:
      if (read_stopwatch(6) >= 3000000) {next_state = initialize_drive;}
      break;

    case update_uss:
      next_state = ultrasonic_state;
      break;

    case initialize_drive:
      set_motion_type(straight);
      drive_straight(init_drive);
      next_state = update_uss;
      break;
    
    case left_only:
      last_state = state;
      drive_straight(driving);
      if (g_NewReading && (ultrasonic_state != last_ultrasonic)) {next_state = update_uss;}
      break;
    
    case left_and_front:
      drive_straight(stop_driving);
      set_motion_type(stop);
      turn_dir = right;
      if (last_state == left_and_front) {
        next_state = win;
        break;
      }
      last_state = state;
      next_state = turn_state;
      break;

    case front_only:
      drive_straight(stop_driving);
      set_motion_type(stop);
      turn_dir = right;
      last_state = state;
      next_state = turn_state;
      break;

    case no_left_or_front:
      drive_straight(stop_driving);
      set_motion_type(stop);
      turn_dir = left;
      last_state = state;
      next_state = turn_state;
      break;
    
    case turn_state:
      pre_turn_corr = (turn_dir == left) ? PRE_TURN_CORR : 0;
      set_motion_type(straight);
      drive_straight_distance(pre_turn_corr);
      
      set_motion_type(turn_dir);
      turn(90);
      set_motion_type(straight);
      drive_straight_distance(POST_TURN_CORR);
      
      next_state = pause_half_sec;
      break;

    case pause_half_sec:
      // set_motion_type(stop);
      if (delay_half_sec()) {
        next_state = initialize_drive;
      }
      break;
      
    case win:
      set_motion_type(stop);
      celebration();
      if (btnD) {next_state = wait_to_start;}
      break;

    default:
      next_state = initialize_drive;
      break;
    }
    state = next_state;
  }
}

// ###########################################################################################################

// Functions Initialization
void init_program() { 
  configure_timers();
  set_motion_type(straight);
}

// Functions for the Ultrasonic Sensor
void set_trig_pin(UltrasonicSensor uss) {
  JB |= (1<<uss.trig_offset); // set trig pin
}

void clear_trig_pin(UltrasonicSensor uss) {
  JB &= ~(1<<uss.trig_offset); // clear trig pin
}

_Bool read_echo_pin(UltrasonicSensor uss) {
  bool echo = JB & (1<<uss.echo_offset); // Read echo signal from pin
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

static inline uint8_t scale_correction(int32_t raw_correction) {
  float raw_correction_mag = (raw_correction >= 0) ? raw_correction : -raw_correction;
  if (raw_correction_mag >= 255.00f) return 0xFF;
  else return (uint8_t) raw_correction_mag;
}

static inline void PID_Controller(_Bool reset, uint32_t L1, uint32_t R1) {
  static int32_t error_sum = 0, error_prev = 0;
  if (reset) {
    error_sum = 0;
    error_prev = 0;
  }
  int32_t error = (int32_t)L1 - (int32_t)R1;
  error_sum += error;
	
  int32_t error_diff = error - error_prev;
	
  float correction = KP*error + KI*error_sum + KD*error_diff;
  uint8_t correction_scaled = scale_correction(correction);

  if (error > 0) {
    if (g_RightDutyCycle + correction_scaled > 0xFF) {g_RightDutyCycle = 0xFF;}
    else {g_RightDutyCycle += correction_scaled;}

    if (g_LeftDutyCycle - correction_scaled < 0xA0) {g_LeftDutyCycle = 0xA0;}
    else {g_LeftDutyCycle -= correction_scaled;}
  } 
  else if (error < 0) { 
    if (g_LeftDutyCycle + correction_scaled > 0xFF) {g_LeftDutyCycle = 0xFF;}
    else {g_LeftDutyCycle += correction_scaled;}

    if (g_RightDutyCycle - correction_scaled < 0xA0) {g_RightDutyCycle = 0xA0;}
    else {g_RightDutyCycle -= correction_scaled;}
  }
	
	error_prev = error;
}

// Functions for navigation
void drive_straight_distance(uint32_t inches) {
  PID_Controller(true, 0, 0);
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

    PID_Controller(false, read_L1_quad_enc(0), read_R1_quad_enc(0));
    LEDS = (g_LeftDutyCycle << 8) | g_RightDutyCycle;
  } 
}

void drive_straight(drive_state cmd) {
  static uint8_t pwmCnt = 0;
  uint32_t L1, R1;
  switch (cmd) {
    case init_drive:
      // Reset variables and states for driving
      L1 = read_L1_quad_enc(1);
      R1 = read_R1_quad_enc(1);
      PID_Controller(true, L1, R1); // 1 is rst, 0s to not start with imaginary error
      pwmCnt = 0;
      g_LeftDutyCycle = 0xCF;
      g_RightDutyCycle = 0xCF;
      break;

    case driving:
      if (pwmCnt <= g_LeftDutyCycle) {JC |= (1 << L_PWM_OFFSET);}
      else {JC &= ~(1 << L_PWM_OFFSET);}
    
      if (pwmCnt <= g_RightDutyCycle) {JC |= (1 << R_PWM_OFFSET);}
      else {JC &= ~(1 << R_PWM_OFFSET);}
    
      if (++pwmCnt == PWM_TOP) {pwmCnt = 0;}
      PID_Controller(0, read_L1_quad_enc(0), read_R1_quad_enc(0));
      break;

    case stop_driving:
      g_LeftDutyCycle = 0;
      g_RightDutyCycle = 0;
      JC &= ~(1 << L_PWM_OFFSET);
      JC &= ~(1 << R_PWM_OFFSET);
  }
  
}

void turn(uint32_t degrees) {   
    // PID_Controller(true, 0, 0);
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

        // PID_Controller(false, read_L1_quad_enc(0), read_R1_quad_enc(0));
        LEDS = (g_LeftDutyCycle << 8) | g_RightDutyCycle;
    }
}

void read_2_uss_fsm(UltrasonicSensor * uss1, 
                    UltrasonicSensor * uss2, 
                    // float * dist_1, 
                    // float * dist_2,
                    uint32_t buf1[MED_FILT_WINDOW],
                    uint32_t buf2[MED_FILT_WINDOW]) {
  static uss_state state = send_trig;
  uss_state next_state = state;
  static _Bool last_echo_1 = false, last_echo_2 = false;
  static _Bool curr_echo_1 = false, curr_echo_2 = false;
  static _Bool seen_echo_1 = false, seen_echo_2 = false;
  static _Bool echo1_read = false, echo2_read = false;
  uint32_t curr_ticks_1 = 0, curr_ticks_2 = 0;
  uint32_t temp_buf1[MED_FILT_WINDOW], temp_buf2[MED_FILT_WINDOW];

  switch (state)
  {
  case send_trig:
    // Ensure trig is cleared
    clear_trig_pin(*uss1);
    clear_trig_pin(*uss2);
    // Start the 10us pulse by setting both trig pins to high then moving state
    set_trig_pin(*uss1);
    set_trig_pin(*uss2);
    start_stopwatch(5);
    next_state = clear_trig;
    break;

  case clear_trig:
    // Wait until 10us have passed before clearing trig
    // 10us/(1.77us/tick) gives ticks, cast converts to uint32_t, '+ 0.05f' ensures the cast rounds correcly. 
    if (read_stopwatch(5) >= 10) {
      clear_trig_pin(*uss1);
      clear_trig_pin(*uss2);
      next_state = count_echo_duration;

      // Ensure flags are cleared
      echo1_read = false;
      echo2_read = false;
      last_echo_1 = false;
      last_echo_2 = false;
      seen_echo_1 = false;
      seen_echo_2 = false;
    }
    break;

  case count_echo_duration:
    // Start each ultrasonic's hw timer when their echo pin goes high,
    // read it on the falling edge.
    curr_echo_1 = read_echo_pin(*uss1);
    curr_echo_2 = read_echo_pin(*uss2);
    if (!echo1_read) {
      if (seen_echo_1) curr_ticks_1 = read_stopwatch(uss1->hw_timer_channel);
      else curr_ticks_1 = 0;
    }
    if (!echo2_read) {
      if (seen_echo_2) curr_ticks_2 = read_stopwatch(uss2->hw_timer_channel);
      else curr_ticks_2 = 0;
    }

    if (curr_echo_1 && !last_echo_1) {  // 1 echo rising edge
      start_stopwatch(uss1->hw_timer_channel);
      seen_echo_1 = true;
    } 
    if (curr_echo_2 && !last_echo_2) {  // 2 echo rising edge
      start_stopwatch(uss2->hw_timer_channel);
      seen_echo_2 = true;
    } 
    if (!curr_echo_1 && last_echo_1) {  // 1 falling edge
      uss1->raw_echo_high_time = curr_ticks_1;
      echo1_read = true;
    }
    if (!curr_echo_2 && last_echo_2) { // 2 falling edge
      uss2->raw_echo_high_time = curr_ticks_2;
      echo2_read = true;
    }

    // Timeout if distance is more than DIST_THRESHOLD
    // We don't care what the actual value is as long as we know
    // whether its +/- our threshold, so grab the current value to put into buffer
    if ((curr_ticks_1 >= TIMEOUT_TICKS) && seen_echo_1) {
      echo1_read = true;
      uss1->raw_echo_high_time = curr_ticks_1;
    }
    if ((curr_ticks_2 >= TIMEOUT_TICKS) && seen_echo_2) {
      echo2_read = true;
      uss2->raw_echo_high_time = curr_ticks_2;
    }

    if (echo1_read && echo2_read) {next_state = median_filter;}

    last_echo_1 = curr_echo_1;
    last_echo_2 = curr_echo_2;
    break;

  case median_filter:
    // Median filter:
      // Take the last five readings from the uss and sort them using selection sort (from cs211)
      // This will sort outliers (erroneously high or low readings) to the extrema
      // taking the median ensures that we have a more consistent value
      //
      // For example, the burst hitting a wire and returning very quicjly could cause us to turn:
      // With this filter, we need at least 3 measurements below the turn threshold before we believe them
      // Add new readings to buffers
    buf1[buf_write_index] = uss1->raw_echo_high_time;
    buf2[buf_write_index] = uss2->raw_echo_high_time;
    if (++buf_write_index == MED_FILT_WINDOW) {buf_write_index = 0;} // Reset back to index 0 if at max

    // Create local copies of buffers
    for (int i = 0; i < MED_FILT_WINDOW; i++) {
        temp_buf1[i] = buf1[i];
        temp_buf2[i] = buf2[i];
    }

    selection_sort(temp_buf1, MED_FILT_WINDOW);
    selection_sort(temp_buf2, MED_FILT_WINDOW);

    uss1->med_echo_high_time = temp_buf1[MED_FILT_WINDOW/2]; // Integer division (e.g. 5/2 = 2)
    uss2->med_echo_high_time = temp_buf2[MED_FILT_WINDOW/2];

    next_state = calculate_distance;
    break;

  case calculate_distance:
    // Use echo high time to calculate distance:
    //    hw_ticks*micros per ticks / 58 micros per cm = cm
    // Dereference pointers to update both distance readings
    g_FrontDist = (uss1->med_echo_high_time) / 58;
    g_LeftDist = (uss2->med_echo_high_time) / 58;
    g_NewReading = true;
    start_stopwatch(5);
    next_state = cooldown;
    break;

  case cooldown:
    if (read_stopwatch(5) >= USS_READ_INTERVAL*HW_TIME_PER_SEC)
    {
      next_state = send_trig;
    }
    break;

  default:
    next_state = send_trig;
  }
  state = next_state;
}

void selection_sort(uint32_t intArray[], uint8_t arrayLength)
{
    uint8_t smallest;

    // Go through all array elements up to the second to last elemnt. On the last 
    // iteration, there are only two elements left in the unsorted array to compare
    for (uint8_t currentElement = 0; currentElement < arrayLength - 1; currentElement++)
    {
        // For the sub-array of currentElement to the end of the array, find the
        // position of the smallest element

        // Initialize the smallest element to the first element in the sub array
        smallest = currentElement;

        // Check ALL other elements in the sub array against the current smallest
        for (int index = currentElement + 1; index < arrayLength; index++)
        {
            // If the current element is smaller than the smallest element, update the smallest
            if (intArray[index] < intArray[smallest])
            {
                // We found a new smallest element
                smallest = index;
            }
        }

        // Now that the sub-array has been searched, we have the index of the smallest
        // value. Now we can swap the values contained in the current element and the
        // smallest element
        swap(&intArray[currentElement], &intArray[smallest]);
    }
}

static inline void swap(uint32_t * pFirst, uint32_t * pSecond)
{
    // Store first in temp
	uint32_t temp = *pFirst;

    // Swap
	*pFirst = *pSecond;
	*pSecond = temp;
}

void celebration() {
    static uint16_t led_state = 0xAAAA;
    if (read_stopwatch(2) >= HW_TIME_PER_SEC/2) {
        led_state ^= 0xFFFF;
        start_stopwatch(2);
    }
    LEDS = led_state;    
}
