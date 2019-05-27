/********************************************************
 * PID RelayOutput Example
 * Same as basic example, except that this time, the output
 * is going to a digital pin which (we presume) is controlling
 * a relay.  the pid is designed to Output an analog value,
 * but the relay can only be On/Off.
 *
 *   to connect them together we use "time proportioning
 * control"  it's essentially a really slow version of PWM.
 * first we decide on a window size (5000mS say.) we then
 * set the pid to adjust its output between 0 and that window
 * size.  lastly, we add some logic that translates the PID
 * output into "Relay On Time" with the remainder of the
 * window being "Relay Off Time"
 ********************************************************/
#include <stdio.h>
#include "../pid.h"
// (!) SAMPLE TIME AND MINIMAL WINDOW SHOULD BE IDENTICAL
// Minimal window size in second for on/off state
#define MINIMAL_TIME_WINDOW_SEC (5) 

bool relay_state(
    PidType* pid, 
    unsigned long* windowStartTime,
    unsigned long windowTimeInSec
    ){
    //printf("\n\rRTC = %lu", *pid->rtc);
    // Locals
    bool state = false;
    unsigned long rtc = *pid->rtc ;
    unsigned long w, ws;
    // Compute     
    PID_Compute(pid);

    // Define state of relay
    if ((rtc - *windowStartTime) > (windowTimeInSec * 1000)){
        *windowStartTime += (windowTimeInSec * 1000);
        //printf("*windowStartTime:%lu\n\r", *windowStartTime);
    }

  
    if (((*pid->myOutput < (FloatType)(rtc - *windowStartTime)) && (*pid->myOutput > pid->outMin)) || (*pid->myOutput >= pid->outMax)) {
        state = true;
    }else{
        state = false;
    } 
        

    printf("rtc:%06lu setpoint:%f input:%02f output:%02f w:%lu relay:%s\n\r", *pid->rtc, *pid->mySetpoint,*pid->myInput, *pid->myOutput, (rtc - *windowStartTime), ((state == true)?"1":"0"));

    // return result 
    return state;


}



int main(){
    printf("\n\rPID Controller Test running...\n\r");
//
//  I   N   I   T
//
    // Define Variables we'll be connecting to
    FloatType Setpoint, Input, Output;
    // Real Time counter in msec
    unsigned long rtc = 0; 
    // Specify the links and initial tuning parameters
    double Kp=1, Ki=2, Kd=1;
    PidType myPID;
    // Init
    Setpoint = 25; // Goal of external parameter, you can change it runtime, algorithm accept it
    PID_init(
        &myPID,     // Pid structure 
        &Input,     // Input value
        &Output,    // Outputvariable
        &Setpoint,  // Target pont
        Kp,         // Proportional coeff
        Ki,         // Integral coeff
        Kd,         // Derivative coeff
        P_ON_E,     // Proportional component on error
        PID_Direction_Reverse, // The deriction is direct (heater, humidifier, etc) 
        &rtc        // Tie with RTC variable
        );

    //tell the PID to range between 0 and the full window size
     PID_SetOutputLimits(&myPID, 0, MINIMAL_TIME_WINDOW_SEC*1000);
    // Set Sample time in msec (!) should be identical to window size
     PID_SetSampleTime(&myPID, (unsigned long)(MINIMAL_TIME_WINDOW_SEC * 1000)); 

     unsigned long windowStartTime = rtc; // defined individually for each PID in a system

//
//  W   O   R   K   
//

    // Make sure PID set in auto
    PID_SetMode(&myPID, PID_Mode_Automatic);

    for(int i=0; i<100; i++){// in reality while(1){
        rtc += 1000;
        Input += 3;
        if(Input > 30) {Input = 20;}
        //printf("\n\rRTC = %lu", rtc);
        relay_state(&myPID, &windowStartTime, MINIMAL_TIME_WINDOW_SEC);
    }

    // If you want manually change output stop PID
    PID_SetMode(&myPID, PID_Mode_Manual);
    // for manual change of coefficients use PID_SetTunings
   

    return 0;
}