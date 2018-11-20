/* ----------------------------------------------------------------------------
   Party Circuits Code for use with the Party Circuits lesson plan 
   available from Microsoft Education Workshop at http://aka.ms/hackingSTEM 
 
   This project uses an Arduino UNO microcontroller board. More information can
   be found by visiting the Arduino website: 
   https://www.arduino.cc/en/main/arduinoBoardUno 
  
  This project involves one or more LEDs connected into the Arduino Digital inputs.
  The lights are controlled via the Excel workbook.
 
  Comments, contributions, suggestions, bug reports, and feature requests 
  are welcome! For source code and bug reports see: 
  http://github.com/[TODO github path to Hacking STEM] 

  Jen Fox, 2018 Microsoft Education Workshop
  For issues with this code visit: https://aka.ms/hackingstemsupport
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
  SOFTWARE. 
----------------------------------------------------------------------------
*/ 


// 1,1;2;1;0;0;0;1;0,1;2;0;1;0;1;0;0,3;3;0;0;1;0;0;0,3;1;1;1;0;0;1;0,1;1;1;0;1;1;0;0,1;1;0;1;1;0;1;0,1;1;0;1;0;0;0;0,1;1;1;1;1;1;0;0

#include <String.h>
// Program variables ----------------------------------------------------------
//LED Pins (max. of 6)
const int ledPin1 = 3;
const int ledPin2 = 5;
const int ledPin3 = 6;
const int ledPin4 = 9;
const int ledPin5 = 10;
const int ledPin6 = 11;

//Create an array to hold the LED pins
int kNumberOfLeds = 6;
int ledArray[] = {3, 5, 6, 9, 10, 11};

//Duration that LEDs are OFF (in milliseconds)
const int ledTimeOff = 50;

//Set variables for Excel commands
bool priorLoopValue; //initialize as infinite loop
int intensityRaw;
int intensity;
int flashSpeedRaw;
int flashSpeed;
//   LED status (0 = off, 1 = on)
int led1 = 0;
int led2 = 0;
int led3 = 0;
int led4 = 0;
int led5 = 0;
int led6 = 0;
int dataArray[6] = {led1, led2, led3, led4, led5, led6};

// Variables for LED intensity and Flash Speed. Can adjust as desired.
int ledBright = 255;
int ledDim = 100;
int fastSpeed = 250;
int medSpeed = 500;
int slowSpeed = 1000;

// Bitwise operator varilables
int bitwiseArray[8][2] =
{{0x300, 8},
{0xC0, 6},
{0x20, 5},
{0x10, 4},
{0x8, 3},
{0x4, 2},
{0x2, 1},
{0x1, 0}};
int ledHexArray [8];

// Excel variables ------------------------------------------------------------
int commandNumber;
int loopTrack = 0; //variable to send to Excel to keep track of loop iterations
float currentCommand = 0; //variable to store current command
float priorCommand; //variable to store prior command

float incomingExcelFloat = 0; // Command Trigger 
//String incomingExcelString1 = ""; 
//String incomingExcelString2 = "";
          
// Serial data variables ------------------------------------------------------
// IMPORTANT: This must be equal to number of channels set in Data Streamer
const byte kNumberOfChannelsFromExcel = 10; //Incoming Serial Data Array

String ledSettingArray[kNumberOfChannelsFromExcel][10];

String incomingSerialData[kNumberOfChannelsFromExcel];

const String kDelimiter = ",";    // Data Streamer expects a comma delimeter
String inputString = "";          // String variable to hold incoming data
boolean stringComplete = false;   // Indicates complete string (newline found)
const int kSerialInterval = 50;   // Interval between serial writes
unsigned long serialPreviousTime; // Timestamp to track serial interval

// SETUP ----------------------------------------------------------------------
void setup() {
  // Initialize led pins as outputs! 
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);   
  pinMode(ledPin5, OUTPUT);
  pinMode(ledPin6, OUTPUT);

  //Initialize the serial port
  Serial.begin(9600);
  priorLoopValue = 1;  
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();
  delay(1000);
  
    // int val = parseHexValues();
    // Serial.println(val, BIN);
 
  // int hexByte = incomingSerialData[0].toInt();
  // Serial.println(hexByte, BIN);
  parseHexValues();
  for (int i = 0; i < 8; i++){
    Serial.println(ledHexArray[i], BIN);
  }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//parseLedSettings();
// for (int i = 0; i < kNumberOfChannelsFromExcel; i++)
// {
//   for (int j = 0; j < 10 ; j++){
//     Serial.print(ledSettingArray[i][j]);
//     Serial.print(",");
//   }
//   Serial.println();
// }


  currentCommand = incomingExcelFloat;
  bool loopValue = isLoop(incomingExcelFloat);
//  Serial.print("this is loop value");
//  Serial.print(loopValue);
//  Serial.println();

//Check if the command sequence has changed. If so, reset loop count
  if(currentCommand != priorCommand){
//    if(loopValue == 1){
//      flashLeds();
//      priorLoopValue = 1;
//    }
//    else if(loopValue == 0 && loopTrack < 1){
//      flashLeds();
//      priorLoopValue = 0;
//    }
    loopTrack = 0;
    priorCommand = currentCommand;
  }

  // If loop is true, loop forever. Else, play once and stop.
  if(loopValue == 1){
    for(int i = 0; i < 10000; i++){
      flashLeds();
      priorLoopValue = 1;
    }
  }
  else if(loopValue == 0 && loopTrack < 1){
      flashLeds();
      delay(10);   
      priorLoopValue = 0;
  }
  
}
//-------------------------------------------------------------------
// Party Circuits Functions
//-------------------------------------------------------------------
// Check if pattern is repeated  (Excel command 0 = infinite loop, 1 = play once
bool isLoop(int loopCommand){
  if(loopCommand == 1){
    return true;
  }
  else{
    return false; 
  } 
}
// Check value of LED intensity
int ledIntensity(int intensity){
  if(intensity == 1){
    return ledDim;
  }
  else if(intensity == 2){
    return ledBright;
  }
  else{
    return 0;
  }
}

// Check and determine speed
int ledSpeed(int flashSpeed){
  if(flashSpeed == 1){
    return slowSpeed;
  }
  else if(flashSpeed == 2){
    return medSpeed;
  }
  else if(flashSpeed == 3){
    return fastSpeed;
  }
  else{
    return fastSpeed;
  }
}

// Excel-driven LED Flashing Function
void flashLeds(){
    // Separate each column of incomingSerialData into separate commands
   for(int i = 0; i < kNumberOfChannelsFromExcel; i++)
    {
       intensityRaw= getValue(incomingSerialData[i+1], ';', 0).toInt();
       flashSpeedRaw = getValue(incomingSerialData[i+1], ';', 1).toInt();

       led1 = getValue(incomingSerialData[i+1],';',2).toInt();
       led2 = getValue(incomingSerialData[i+1],';',3).toInt();
       led3 = getValue(incomingSerialData[i+1],';',4).toInt();
       led4 = getValue(incomingSerialData[i+1],';',5).toInt();
       led5 = getValue(incomingSerialData[i+1],';',6).toInt();
       led6 = getValue(incomingSerialData[i+1],';',7).toInt();

     //  Serial.print("This is the data");
       //Serial.print(inputString);
       //Serial.println();

       //Determine intensity
       intensity = ledIntensity(intensityRaw);
       flashSpeed = ledSpeed(flashSpeedRaw);

      //Update LED data array to turn on/off approprirate LEDs   
       dataArray[0] = led1;
       dataArray[1] = led2;
       dataArray[2] = led3;
       dataArray[3] = led4;
       dataArray[4] = led5;
       dataArray[5] = led6;      
      
      // Process and send data to Excel via serial port (Data Streamer)
      // Sending: Loop Number, Command Number (Excel Column), Four LED states  
       commandNumber = i;
       Serial.print(loopTrack);
       Serial.print(kDelimiter);
       Serial.print(commandNumber);
       Serial.print(kDelimiter);
       Serial.print(dataArray[0]);
       Serial.print(kDelimiter);
       Serial.print(dataArray[1]);
       Serial.print(kDelimiter);
       Serial.print(dataArray[2]);
       Serial.print(kDelimiter);
       Serial.print(dataArray[3]);
       Serial.print(kDelimiter);
       Serial.print(dataArray[4]);
       Serial.print(kDelimiter);
       Serial.print(dataArray[5]);
       Serial.println(); // Add final line ending character only once
      
       // Flash appropriate LEDs at given intensity       
       for(int i = 0; i < kNumberOfLeds; i++){
        if(dataArray[i] == 1){
          analogWrite(ledArray[i], intensity);
        } 
        else(analogWrite(ledArray[i], LOW));
       }
       
       delay(flashSpeed);

       //Turn off all LEDs
       for(int i = 0; i < kNumberOfLeds; i++){
          analogWrite(ledArray[i], LOW);
        } 
       
       delay(ledTimeOff); //same off duration for all sequences
     }

  // Keep track of number of times command sequence is repeated
  loopTrack++;
}

void parseHexValues()
{
    char stringCopy[incomingSerialData[1].length()+1];
    incomingSerialData[1].toCharArray(stringCopy, incomingSerialData[1].length()+1);
    int hex = strtol(stringCopy, NULL, 16);
    for (int i = 0; i < 8; i++){
      int val = hex & bitwiseArray[i][0];
      val = val >> bitwiseArray[i][1];
      ledHexArray[i] = val;
    }
}
void parseLedSettings()
{
  for (int i = 0; i < kNumberOfChannelsFromExcel; i++){
    char stringCopy[incomingSerialData[i+1].length()+1];
    incomingSerialData[i+1].toCharArray(stringCopy, incomingSerialData[i+1].length()+1);
    char *token = strtok(stringCopy, ";");
    int j = 0;
    while (token != NULL){
      ledSettingArray[i][j] = token;
      token = strtok(NULL, ";");
      j++;
    }
  }
}

// void parseLedSettings()
// {
//   for (int i = 0; i < kNumberOfChannelsFromExcel; i++){
//     for (int j = 0; j < 8; i++){
//       ledSettingArray[i][j] = seperateData(incomingSerialData[i+1], ';', j).toInt();
//     }
//   }
// }

// int seperateData(String inputData, char seperater; int index)
// {
//   //int charIndex = 0;
//   arrayIndex = 0
//   //int maxIndex = inputData.length();
//   String element;
//   if (charIndex == seperater && index < arrayIndex){
//     arrayIndex++; //Increment the counter to next element in the array
//     charIndex++; //Skip the seperater
//   } 
//   else if (charIndex != seperater && charIndex < maxIndex){
//     element += inputData[charIndex];
//     charIndex++
//   } 
//   else {
//     return element.toInt();
//   }
// }


//Function for specialized string search: go through a string and pull out characters
String getValue(String dataString, char separator, int index)
{                                           // basic searching algorithm
                                            // data is the serial string, separator is a comma, index is where we want to look in the data array
  int matchingIndex = 0;                    // no match because we are starting to look
  int strIndex[] = {0, -1};
  int maxIndex = dataString.length()-1;
  for(int i=0; i<=maxIndex && matchingIndex<=index; i++){     // loop until end of array or until we find a match
    if(dataString.charAt(i)==separator || i==maxIndex){             // if we hit a comma OR we are at the end of the array
      matchingIndex++;                                        // increment matchingIndex to keep track of where we have looked
      strIndex[0] = strIndex[1]+1;                            // set substring parameters
           // ternary operator in objective c is [condition] ? [true expression] : [false expression] 
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return matchingIndex>index ? dataString.substring(strIndex[0], strIndex[1]) : ""; // if match return substring or else return ""
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
// Process serial data inputString from Data Streamer
void ParseSerialData()
{
  if (stringComplete) {     
    //Build an array of values from comma delimited string from Data Streamer
    BuildDataArray(inputString);

    // Set variables based on array index referring to columns:
    // Data Out column A5 = 0, B5 = 1, C5 = 2, etc.
    incomingExcelFloat = incomingSerialData[0].toFloat(); // Data Out column A5
    //incomingExcelString1 = incomingSerialData[1]; // Data Out column B5
    //incomingExcelString2 = incomingSerialData[2]; //Data out column C5
    //incomingExcelString3 = incomingSerialData[3]; //Data out column D5
    
    inputString = ""; // reset inputString
    stringComplete = false; // reset stringComplete flag
  }

}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  //Loop track number
  Serial.print(loopTrack);
  Serial.print(kDelimiter);
  
  // Example test for incoming Excel variables
  Serial.print(incomingExcelFloat);
  Serial.print(kDelimiter);

  //LED status (on/off)
  Serial.print(commandNumber);
  Serial.print(kDelimiter);
  Serial.print(dataArray[0]);
  Serial.print(kDelimiter);
  Serial.print(dataArray[1]);
  Serial.print(kDelimiter);
  Serial.print(dataArray[2]);
  Serial.print(kDelimiter);
  Serial.print(dataArray[3]);
  Serial.print(kDelimiter);
    
  Serial.println(); // Add final line ending character only once
}

//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    serialPreviousTime = millis(); // Reset serial interval timestamp
    sendDataToSerial(); 
  }
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  getSerialData2();
  ParseSerialData();
}

// Gathers bits from serial port to build inputString
void getSerialData2(){
  if(Serial.available()){
    inputString = Serial.readStringUntil('\n');
    stringComplete =true;
  }
}

// void getSerialData()
// {
//   while (Serial.available()) {
//     char inChar = (char)Serial.read();    // Read new character
//     inputString += inChar;                // Add it to input string
//     if (inChar == '\n') {                 // If we get a newline... 
//       stringComplete = true;              // Then we have a complete string
//     }
//   }
// }

// Takes the comma delimited string from Data Streamer
// and splits the fields into an indexed array
void BuildDataArray(String data)
{
  return ParseLine(data);
}

// Parses a single string of comma delimited values with line ending character
void ParseLine(String data)  
{
    int charIndex = 0; // Tracks the character we are looking at
    int arrayIndex = 0; // Tracks the array index to set values into
    while(arrayIndex < kNumberOfChannelsFromExcel) // Loop until full
    {
        String field = ParseNextField(data, charIndex);  // Parse next field
        incomingSerialData[arrayIndex] = field; // Add field to array
        arrayIndex++;   // Increment index
    }
}

// Parses the next value field in between the comma delimiters
String ParseNextField(String data, int &charIndex)
{
    if (charIndex >= data.length() )
    {
      return ""; //end of data
    }
    
    String field = "";
    bool hitDelimiter = false; // flag for delimiter detection 
    while (hitDelimiter == false) // loop characters until next delimiter
    {
        if (charIndex >= data.length() )
        {
          break; //end of data
        }

        if (String(data[charIndex]) == "\n") // if character is a line break
        {
          break; // end of data
        }
        
       if(String(data[charIndex]) == kDelimiter) // if we hit a delimiter
        {
          hitDelimiter = true;  // flag the delimiter hit
          charIndex++; // set iterator after delimiter so we skip next comma
          break;
        }
        else
        {        
          field += data[charIndex]; // add character to field string
          charIndex++; // increment to next character in data
        }
    }
    return field;
}
