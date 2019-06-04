#include <Adafruit_NeoPixel.h>

#define NeoP     10
#define SHOW_BUTTON_TIME 1000 // in mS
#define GAMETIMEOUT 5000      // in mS
#define SEQUENCE_SIZE 6
#define NeoPin 6
#define BOUNCE_GAP 10         // in mS

#define BT1      7//2  
#define BT2      8//3  
#define BT3      9//4  
#define BT4      3//5  
#define BT5      4//7
#define BT6      5//8
#define BTR      2 

#define BT1_LED      2//0
#define BT2_LED      1//1
#define BT3_LED      0//2
#define BT4_LED      5//3
#define BT5_LED      4//4
#define BT6_LED      3//5
#define BTR_LED      9 

#define S1_LED      6
#define S2_LED      7
#define S3_LED      8 

#define SgnIn    11
#define SgnOut    12



byte menu;
#define MENU_INIT            1
#define MENU_INIT_LOOP       2
#define MENU_GAME            3
#define MENU_WIN             4
#define MENU_WAIT            5
#define MENU_WAIT_LOOP       6
#define MENU_RESET           7
#define MENU_BEGIN           8
#define MENU_FAIL            9
#define MENU_IDLE           10






#define Green  0x00FF00
#define White  0xFFFFFF
#define Red  0xFF0000
#define Blue  0x0000FF
#define Yellow  0xFFFF00
#define Black 0x000000

int game_buttons[7]={BT1,BT2,BT3,BT4,BT5,BT6,BTR};
int game_leds[NeoP]={BT1_LED,BT2_LED,BT3_LED,BT4_LED,BT5_LED,BT6_LED,BTR_LED,S1_LED,S2_LED,S3_LED};

int button_sequence[SEQUENCE_SIZE]={4,2,5,3,6,1};   // Number of the button 1 to 6
int color_sequence[SEQUENCE_SIZE]={1,3,1,3,2,1};    // Number of the LED 1, 2 or 3 
int led_number_pushes_relation[3]={1,0,2};
uint32_t color_led_SX[3]={Green,Red,Blue};

enum single_step_stat{IDLE,SHOW,WAIT_PRESS,WAIT_RELEASE,FAIL,WIN,RESET};

int countWin = 0;
int game_stat=0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NeoP, NeoPin, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  
  turn_all_off();
  Serial.begin(9600);
  Serial.println("Begin");
//  delay(2000);
  
  for (int i=0;i<7;i++){
    pinMode(game_buttons[i], INPUT_PULLUP);
  }
  
  //pinMode(SgnIn, INPUT);
  pinMode(SgnIn, INPUT_PULLUP);
  
  pinMode(SgnOut, OUTPUT);
  digitalWrite(SgnOut, HIGH);
  //Serial.println("Strip start");
  strip.show();
  delay(250);
  
  menu = MENU_IDLE;
  turn_all_off();
}

void turn_all_off(){
  for (int i=0; i<NeoP;i++){
    strip.setPixelColor(game_leds[i], Black);
  }
  strip.show();
}

void turn_leds_off(){
  //Serial.println("Turn leds off");
  for (int i=0; i<NeoP;i++){
    strip.setPixelColor(game_leds[i], Black);
  }
  strip.setPixelColor(game_leds[6], Red);
  strip.show();     
}

void turn_all_color(uint32_t color){
  //Serial.println("Turn all in one color");
  for (int i=0; i<NeoP;i++){
    strip.setPixelColor(game_leds[i], color);
  }
  strip.show();     
}


void blink_all(uint32_t color1,uint32_t color2, int times){
  //Serial.println("Blinking");
   for (int j = 0; j<times ; j++){
          for (int i=0; i<NeoP;i++){
            strip.setPixelColor(game_leds[i], color1);
          }
          strip.show();
          delay(100);
          for (int i=0; i<NeoP;i++){
            strip.setPixelColor(game_leds[i], color2);
          }
          strip.show();
          delay(100);
    }
}

void loop() {
 // boolean InStatus = digitalRead(SgnIn);
  static int time_stamp_last_BTR=0;
  static boolean InStatus=true;
  Serial.println(InStatus);
  if (millis()-time_stamp_last_BTR>BOUNCE_GAP){
    InStatus = digitalRead(BTR);
    time_stamp_last_BTR=millis();
  }
  
  switch(menu) {  
     case MENU_IDLE:
        if(InStatus == false){
       // turn_leds_off();
        menu = MENU_RESET;
        turn_all_off();
        //Serial.println("Game starts");
     }   
     break;
     case MENU_RESET:
      Serial.println("Menu reset");
      digitalWrite(SgnOut, HIGH);

      if(InStatus == true){
        menu = MENU_INIT;
      }

     break;
      
     case MENU_INIT:
      Serial.println("Menu init");
      menu = MENU_INIT_LOOP;

      break;
       
     case MENU_INIT_LOOP:
       Serial.println("Menu init loop");

        menu = MENU_GAME;
        
        break;
     
     case MENU_GAME:
        Serial.println("Menu game");
        game_stat=game_loop();
        if (game_stat==FAIL){
          Serial.print("Faileoss");
          menu = MENU_FAIL;
          
        }else if (game_stat==WIN){
          menu = MENU_WIN;
        }else if (game_stat==RESET){
          menu = MENU_INIT;
        }

        break;


      case MENU_FAIL:      
        Serial.println("Menu fail");

        menu = MENU_IDLE;
        delay(100);
        blink_all(Red,Black,5);
        delay(100);
        break;
      case MENU_WIN:
        Serial.println("Menu win");

        digitalWrite(SgnOut, LOW);
        
        blink_all(Green,Black,20);

        menu = MENU_WAIT;
        
        break;
        
      case MENU_WAIT:
        Serial.println("Menu wait");

        turn_all_color(Green);
            
        break;        
   }
}

void set_strip_next_color(int index, uint32_t color){
  //Serial.println("Set pixel color");

  strip.setPixelColor(index, color);
}

int game_loop(){
  static int current_sequence_step=0;
  static int single_status=IDLE;
  static long timestamp=0;
  static long timestamp_show=0;
  static int press_counter=0;
  static int last_pressed=-1;
  switch (single_status){
    case IDLE:
    //Serial.println("IDLE");

      timestamp_show=millis();
      timestamp=millis();
      single_status=WAIT_PRESS;
      set_strip_next_color(game_leds[button_sequence[current_sequence_step]-1],White);
      set_strip_next_color(game_leds[color_sequence[current_sequence_step]+6],color_led_SX[color_sequence[current_sequence_step]-1]);
      strip.setPixelColor(game_leds[6], Red);
      strip.show();
    break;
    case SHOW:
      //Serial.println("SHOW");

     
      

    break;
    case WAIT_PRESS:
    //Serial.println("WAIT PRESS");
 /*   
    Serial.print("Current sequence step: ");
    Serial.print(current_sequence_step);
    Serial.print("Button to press: ");
    Serial.print(button_sequence[current_sequence_step]);
    Serial.print("  Goal press: ");
    Serial.print(led_number_pushes_relation[color_sequence[current_sequence_step]-1]);
    Serial.print("  Actual pressings: ");
    *///Serial.println(press_counter);
   
      if (millis()-timestamp_show>SHOW_BUTTON_TIME){
        //Serial.println("TIMEOUT");
     //   single_status=WAIT_PRESS;
        turn_leds_off();
  //      timestamp=millis();
      }
      
      last_pressed=detect_button_pressed();
      Serial.println(last_pressed);
      if (last_pressed==button_sequence[current_sequence_step]){
        single_status=WAIT_RELEASE;
        press_counter++;
      }else if(last_pressed==7){
          single_status=WAIT_RELEASE;
      }else if(last_pressed!=-1){
        single_status=FAIL;
        turn_leds_off();
      }

      if (millis()-timestamp>GAMETIMEOUT){
        single_status=FAIL;
        Serial.println("TIMEOUT ewwe");
        turn_leds_off();
        timestamp=millis();
        Serial.println(led_number_pushes_relation[color_sequence[current_sequence_step]-1]);
        if(press_counter==led_number_pushes_relation[color_sequence[current_sequence_step]-1]){
          if (current_sequence_step+1>=SEQUENCE_SIZE){
            single_status = WIN;
          }else{
            single_status=IDLE;
            press_counter=0;
            current_sequence_step++;
          }
        }else{
          Serial.println("PORACAs");
          single_status=FAIL;
          current_sequence_step=0;
        }
     //   
      }
    break;
    case WAIT_RELEASE:
    //Serial.println("WAIT RELEASE");

      if (detect_button_released(last_pressed)){
        single_status=WAIT_PRESS;
        if(last_pressed==7){
          single_status=RESET;
          turn_leds_off();
        }
        Serial.println("WAIT press");
      }
      if (millis()-timestamp>GAMETIMEOUT){
        Serial.println("TIMEOUT RELEASE");
        turn_leds_off();
        timestamp=millis();
        single_status=FAIL;
      }
    break;

    case FAIL:
      Serial.println("FAIL");

      current_sequence_step=0;
      single_status=IDLE;
      timestamp=0;
      timestamp_show=0;
      press_counter=0;
      last_pressed=-1;
      return (FAIL);
      Serial.println("Nuca apacrece");
    break;
    case RESET:
      Serial.println("FAIL");

      current_sequence_step=0;
      single_status=IDLE;
      timestamp=0;
      timestamp_show=0;
      press_counter=0;
      last_pressed=-1;
      return (RESET);
      Serial.println("Nuca apacrece");
    break;
    case WIN:
      current_sequence_step=0;
      single_status=IDLE;
      timestamp=0;
      timestamp_show=0;
      press_counter=0;
      last_pressed=-1;
      //Serial.println("WIN");
      return (WIN);
    break;
  }
  return(0);
}

int detect_button_pressed(){
//  Serial.println("Detect button pressed");
  static int last_button_timestamp;
  for (int i=0;i<7;i++){
    if(digitalRead(game_buttons[i])==0 && (millis()-last_button_timestamp)>BOUNCE_GAP){
      last_button_timestamp=millis();
    // Serial.println(i+1);
      return(i+1);
    }
  }
  return(-1); //nothing was pressed
}

bool detect_button_released(int index){
  //Serial.println("Detect button released");

  if(digitalRead(game_buttons[index-1])==1){
    //Serial.println("true");

    return(true);
  }else{
    //Serial.println("false");

    return(false);
  }
}
