#include "simpletools.h"
#include "servo.h"
#include "fdserial.h"
#include "wavplayer.h"
#include "servodiffdrive.h"
#include "ping.h"                             // Include ping header

#define PIN_XBEE_RX      0
#define PIN_XBEE_TX      1
#define PIN_SERVO_LEFT  14
#define PIN_SERVO_RIGHT 15
#define PIN_SERVO_MID 16

volatile int ch;
volatile int XeeChange=0;
volatile int cmDist=0;
volatile int turn=0;
int * cog_ptr[8];
int pinLeft, pinRight, rampLeft, rampRight;   // Variables shared by functions
volatile fdserial *xbee;

void XBee();
char char2lowercase (char c);

int main (void)
{
  xbee = fdserial_open(PIN_XBEE_RX, PIN_XBEE_TX, 0, 9600);
  drive_pins(PIN_SERVO_LEFT, PIN_SERVO_RIGHT);    // Set the Left and Right servos 
  cog_ptr[1] = cog_run(&XBee,32);

  while(1)
  {   
    cmDist = ping_cm(17);
    print("cmDist = %d\n", cmDist); 
    if(XeeChange)
    {   
      ch = char2lowercase(ch);
      if (cmDist <=20 && ch=='f'){
       servo_speed(14,0);
       servo_speed(15,0);
       pause(500);
     }      
     else {
      switch ((char) ch)
      {
      case 'f':
        servo_speed(14,-30);
        servo_speed(15,30);
        pause(500);
        break;
      case 'b':
        servo_speed(14,30);
        servo_speed(15,-30);
        pause(500);
        break;
      case 'r' :
        servo_speed(14,25);
        servo_speed(15,25);
        pause(500); 
        break;
      case 'l':
        servo_speed(14,-25);
        servo_speed(15,-25);
        pause(500);
        break;  
      case 's' :
        servo_speed(14,0);
        servo_speed(15,0);
        pause(500); 
        break;
      default:
        servo_speed(14,0);
        servo_speed(15,0);
        pause(500);
        break;           
      }     
      XeeChange = 0;
     }      
    }                          
    pause(1);
  }    

  return 0;
}


void XBee(){
   
   putchar(CLS);
   fdserial_rxFlush(xbee);
   while(1){
     ch = fdserial_rxChar(xbee);
     XeeChange = 1;
     fdserial_txChar(xbee, ch);
     fdserial_txFlush(xbee);
   }
}  

char char2lowercase (char c) {
	return ('A'<=c && c<='Z') ? c+32 : c;
}
