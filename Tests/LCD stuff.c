//include all the files necessary to use LCD display

/*
LCD commands
LCDClear(); //clears the display & brings cursor to (0,0)
LCDWriteString(“msg”); //writes whatever ‘msg’ is to LCD
LCDWriteInt(val, field_length); //writes int of ‘val’ in a space of ‘field_length’
LCDGotoXY(x,y); //takes cursor to position ‘(x,y)’
LCDWriteStringXY(x, y, msg); //writes “msg” at the location (x,y)
LCDWriteIntXY(x,y,num,field_length); //prints “num” in a specific field length
*/

int main()
{
  LCDInit(LS_BLINK|LS_ULINE); //initialize LCD subsystem

  //Running state
  LCDWriteStringXY(0, 0, “State: Running State”); 
  mTimer(1000);
  LCDClear(); 
  
  //Bucket state
  LCDWriteStringXY(0, 0, “State: Bucket State”); 
  mTimer(1000);
  LCDClear(); 
  
  //Pause state
  LCDWriteStringXY(0, 0, “State: Pause State”); 
  mTimer(1000);
  LCDClear(); 
  
  LCDWriteStringXY(0, 0, “P = Pending”);
  LCDWriteStringXY(0, 1, “S = Sorted”);
	mTimer(1000);
  LCDClear();

  LCDWriteStringXY(0, 0, “BL: WH: ST: AL: ”);
	LCDWriteStringXY(0, 1, “S   S   S   S   S   ”);
	LCDWriteIntXY(1,1,black_count, 2);
	LCDWriteIntXY(1,5,white_count, 2);
	LCDWriteIntXY(1,9,steel_count, 2);
	LCDWriteIntXY(1,13,aluminu_count, 2);
	mTimer(3000);
  LCDClear();
	
  /*
  if we need to output the number of each thing pending, we need to add a count to the ISR for the RL sensor 
  then we can know the right amount of types of objects still pending
  */
  LCDWriteStringXY(0, 0, “BL: WH: ST: AL: ”);
	LCDWriteStringXY(0, 1, “P   P   P   P   P   ”);
	LCDWriteIntXY(1,1,black_count, 2);
	LCDWriteIntXY(1,5,white_count, 2);
	LCDWriteIntXY(1,9,steel_count, 2);
	LCDWriteIntXY(1,13,aluminu_count, 2);
	mTimer(3000);
  LCDClear();
  
  
  
}
