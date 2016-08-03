#include <M5.h>
#include <Keypad.h> 
#include <EEPROM.h>
#include <Time.h>

#include <TrueRandom.h>

#include <Password.h>


#define TRACE_DEBUG
#define MySerial Serial

#define MAX_PWD_LENGTH 12
char custom_pwd_code[MAX_PWD_LENGTH] = "8088";
Password current_pwd = Password( (char*)custom_pwd_code );

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
const char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 12, 11, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 4, 10}; //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const char pansen_hint[]= {0xB7, 0xBF, 0xBC, 0xE4, 0xC3, 0xC5, 0xCB, 0xF8, 0x00, 0x00};
const char error_hint[]={0xB4, 0xED, 0xCE, 0xF3, 0x20};
const char contact_hint[]={0xC1, 0xAA, 0xCF, 0xB5, 0xBF, 0xCD, 0xB7, 0xFE};
const char config_hint[]={0xC9, 0xE8, 0xD6, 0xC3, 0xC4, 0xA3, 0xCA, 0xBD};
const char correct_hint[]={0x20, 0x20, 0xD5, 0xFD, 0xC8, 0xB7, 0x20, 0x20};

char g_Ringbell_Level = 1;
const unsigned char Pin_Lock =  8; 
const unsigned char Pin_Ring =  9;
const unsigned char Pin_Key_Backlight =  13;

char g_times = 0;
unsigned char g_Key_Backlight_Timer = 0;
unsigned char g_Report_Handle_Timer = 0;

enum
{
  STATE_IDLE,
  STATE_REPORT,
  STATE_CONFIG,
};

unsigned char g_CurStatus = STATE_IDLE;
unsigned long previousMillis = 0;
const int interval = 2000;
unsigned char interval_count = 0;

void checkPassword()
{
  bool pwd_pass = 0;
  int index = 0;

  current_pwd.set((char*)custom_pwd_code);

  if (current_pwd.evaluate()){
    M5.ClearScreen();
    M5.PutS_2X(2, 2, (char*)correct_hint);    

    digitalWrite(Pin_Lock, LOW);
    g_CurStatus = STATE_REPORT;
    g_Report_Handle_Timer = 3;    

  }
  else{
    M5.ClearScreen();
    M5.PutS_2X(2, 2, (char*)pansen_hint);      
  }
}

void keypadEvent(KeypadEvent eKey)
{
  switch (customKeypad.getState()){
    case PRESSED:
    digitalWrite(Pin_Key_Backlight, HIGH);
    M5.Light(255);
    g_Key_Backlight_Timer = 2;
#if defined(TRACE_DEBUG)   
    MySerial.print(eKey);
#endif       
    switch (eKey){
      case '#': 
        checkPassword(); 
        current_pwd.reset();
      break;
      case '*': 
        ////------Ring Bell-----------
        g_Ringbell_Level = 0;
        digitalWrite(Pin_Ring, g_Ringbell_Level);
        current_pwd.reset();
    M5.ClearScreen();
    M5.PutS_2X(2, 2, (char*)pansen_hint);    
      break;
      default: 
        current_pwd.append(eKey);
        unsigned char input_len = strlen(current_pwd.getGuess());
        for(char index = 0; index < input_len; index++){
          M5.PutS_2X(2+index*16, 36, "*");
        }          
      break;
     }
 
  }
}

void setup() {
  Serial1.begin(9600); 
  MySerial.begin(9600);
    
  pinMode(Pin_Lock, OUTPUT);    
  pinMode(Pin_Ring, OUTPUT);
  pinMode(Pin_Key_Backlight, OUTPUT);
  digitalWrite(Pin_Lock, HIGH);
  digitalWrite(Pin_Ring, g_Ringbell_Level);
  digitalWrite(Pin_Key_Backlight, LOW);

pinMode(A0, OUTPUT);
digitalWrite(A0, HIGH);      

  customKeypad.addEventListener(keypadEvent);
  
  // put your setup code here, to run once:
  //--------M5 ------------
  M5.Init();
  M5.ClearScreen();
  M5.Light(100);
  M5.SetKeyLightTime(20);
  M5.HideRunLogo();
  M5.HideM5Logo();
  M5.PutS_2X(2, 2, (char*)pansen_hint);
}

void loop() {
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    switch(g_CurStatus){
      case STATE_REPORT:{
        if(g_Report_Handle_Timer > 0){
          g_Report_Handle_Timer--;
        }
        else{
          g_CurStatus = STATE_IDLE;
          digitalWrite(Pin_Lock, HIGH);
          M5.ClearScreen();
          M5.PutS_2X(2, 2, (char*)pansen_hint);          
        }
      }
      break;
    }   

    if(g_Ringbell_Level==0){
      g_Ringbell_Level = 1;
      digitalWrite(Pin_Ring, g_Ringbell_Level);
    }
    
    if(g_Key_Backlight_Timer > 0){
      g_Key_Backlight_Timer--;
    }
    else{
      digitalWrite(Pin_Key_Backlight, LOW);
      M5.Light(0);
    }
     
  }
  
  switch(g_CurStatus){
    case STATE_IDLE:{
      char customKey = customKeypad.getKey();
      if (customKey){
        ///g_CurStatus = STATE_REPORT;
      }
    }
    break;
    
    case STATE_CONFIG:{
      // put your main code here, to run repeatedly:
      if(g_times==0){
        M5.ClearScreen();
        M5.PutS_2X(2, 2, (char*)pansen_hint);   
    
        g_times = 1;
      }
      else{
        M5.ClearScreen();
        M5.PutS_2X(2, 2, (char*)contact_hint);   
    
        g_times = 0;
      }
    }
    break; 
  }


  delay(100);
}
