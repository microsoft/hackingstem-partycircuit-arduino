/* -------------------__ Hacking STEM PartyCircuits Arduino __-----------------===//
//   For use with the Party Circuits lesson plan 
//   available from Microsoft Education Workshop at
//   https://www.microsoft.com/en-us/education/education-workshop/party-lights.aspx
//   http://aka.ms/hackingSTEM 
//
//  Overview:
//  This project flashes one or more LEDs based on commands sent from the Excel 
//  Party Circuits workbook. Commands (intensity, flash speed, and which lights
//  are on) are send as Hex numbers in 9-element array. 
//  To test commands in the Arduino Serial Monitor, use this example:
//        0,0,3EC,2C4,364,154,374,248,3A8,2BC
//  Pins:
//  Digital Pin 3 -- LED 1
//  Digital Pin 5 -- LED 2
//  Digital Pin 6 -- LED 3
//  Digital Pin 9 -- LED 4
//  Digital Pin 10 -- LED 5
//  Digital Pin 11 -- LED 6
// 
//  This project uses an Arduino UNO microcontroller board, information at:
//  https://www.arduino.cc/en/main/arduinoBoardUno
//
//  Comments, contributions, suggestions, bug reports, and feature requests
//  are welcome! For source code and bug reports see:
//  https://github.com/microsoft/hackingstem-partycircuit-arduino
//
//  Copyright 2018 Jen Fox Microsoft EDU Workshop - HackingSTEM
//  MIT License terms detailed in LICENSE.txt 
----------------------------------------------------------------------------
*/ 

#include <String.h>
// -------------------------------------------------------------------------
//                      PROGRAM VARIABLES
// -------------------------------------------------------------------------
//Total number of LEDs (max. of 6)
const int kMaxNumberOfLeds = 6; //This is the maximum number of LEDs we can add
//  Create an array to hold the LED pins
const int kLedArray[] = {3, 5, 6, 9, 10, 11};
//  Duration that LEDs are OFF (in milliseconds)
const int kLedTimeOff = 50;

// Variables for LED intensity and Flash Speed. Can adjust as desired.
const int kLedBright = 255;
const int kLedMed = 100;
const int kLedDim = 50;
const int kFastSpeed = 250;
const int kMedSpeed = 500;
const int kSlowSpeed = 1000;

//Declare variables for Excel commands
bool priorLoopValue; 
int intensityRaw;
int intensity;
int flashSpeedRaw;
int flashSpeed;
//   LED status (0 = off, 1 = on)
int ledStatusArray[6] = {};

// Bitwise operator variables to extract Excel data. 
//   This is an advanced programming concept, 
//   leave as-is if unfamiliar with bitwise operations
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

// Command Loop Variable
bool isPatternLooped = 0;
bool newString = false; 
          
// Serial data variables ------------------------------------------------------
// IMPORTANT: This must be equal to number of channels set in Data Streamer
const byte kNumberOfChannelsFromExcel = 10; //Incoming Serial Data Array
const int kNumberOfCommands = 8; //Incoming Command Array

String ledSettingArray[kNumberOfChannelsFromExcel][10];

String incomingSerialData[kNumberOfChannelsFromExcel];

const String kDelimiter = ",";    // Data Streamer expects a comma delimeter
String inputString = "";          // String variable to hold incoming data
boolean stringComplete = false;   // Indicates complete string (newline found)
const int kSerialInterval = 50;   // Interval between serial writes
unsigned long serialPreviousTime; // Timestamp to track serial interval

// --------------------------------------------------------------------
//                               SETUP 
// --------------------------------------------------------------------
void setup() {
  // Initialize led pins as outputs! 
  for(int i; i < kMaxNumberOfLeds; i++){
    pinMode(kLedArray[i], OUTPUT);
  }
 
  //Initialize the serial port
  Serial.begin(9600);
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();

  if (isPatternLooped == 1){
    newString = false;
    loopTrack = 0;
    for (int i = 0; i < kNumberOfChannelsFromExcel; i++)
    {
      processIncomingSerial();
      if (incomingSerialData[0] == "#pause" || newString == true){
        break;
      }
      parseHexValues(i+1);
      processOutgoingSerial();
      flashLeds(i);
    }
  } else if (newString == true){
      newString = false;
      loopTrack = 0;
        for (int i = 0; i < kNumberOfChannelsFromExcel; i++)
          {
            processIncomingSerial();
            if (incomingSerialData[0] == "#pause" || newString == true){
             break;
            }
            parseHexValues(i+1);
            processOutgoingSerial();
            flashLeds(i);
          }
  }
}

//-------------------------------------------------------------------
//                      Party Circuits Functions
//-------------------------------------------------------------------
// Take Excel commands in Hex and separate into individual commands in binary string
void parseHexValues(int index)
{
    char stringCopy[incomingSerialData[index].length()+1];   // 
    incomingSerialData[index].toCharArray(stringCopy, incomingSerialData[index].length()+1); // Convert String object to char[]
    int hex = strtol(stringCopy, NULL, 16); // string to long converts str to string w/ any base
    for (int i = 0; i < kNumberOfCommands; i++){
      int val = hex & bitwiseArray[i][0]; // filter out all bits not relevant for command
      val = val >> bitwiseArray[i][1]; // shift all bits to get relevant command
      ledHexArray[i] = val;
    }
}

// Excel-driven LED Flashing Function
void flashLeds(int commandNum){
    // Separate each column of incomingSerialData into separate commands
       for(int i = 0; i < kMaxNumberOfLeds; i++){
        ledStatusArray[i] = ledHexArray[i+2];
       }

       //Determine intensity and LED flash speed from Excel Commands
       intensity = ledIntensity(ledHexArray[0]);
       flashSpeed = ledSpeed(ledHexArray[1]);

      //Determine command number based on input 
       commandNumber = commandNum;
      
       // Flash appropriate LEDs at given intensity       
       for(int i = 0; i < kMaxNumberOfLeds; i++){
        if(ledStatusArray[i] == 1){
          analogWrite(kLedArray[i], intensity);
        } 
        else(analogWrite(kLedArray[i], LOW));
       }
       
       delay(flashSpeed);

       //Turn off all LEDs
       for(int i = 0; i < kMaxNumberOfLeds; i++){
          analogWrite(kLedArray[i], LOW);
        } 
       
       delay(kLedTimeOff); //same off duration for all sequences
     
  // Keep track of number of times command sequence is repeated
  loopTrack++;
}

// Check if pattern is repeated  (Excel command 0 = infinite loop, 1 = play once
bool isLoop(int loopCommand){
  if(loopCommand == 1){
    return true;
  }
  else{
    return false; 
  } 
}

// Check and return LED intensity
int ledIntensity(int intensity){
  if(intensity == 1){
    return kLedDim;
  }
  else if(intensity == 2){
    return kLedMed;
  }
  else if(intensity == 3){
    return kLedBright;
  }
  else{
    return 0;
  }
}

// Check and return LED flash speed
int ledSpeed(int flashSpeed){
  if(flashSpeed == 1){
    return kSlowSpeed;
  }
  else if(flashSpeed == 2){
    return kMedSpeed;
  }
  else{
    return kFastSpeed;
  }
}

// 
// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
// Process serial data inputString from Data Streamer
void ParseSerialData()
{
  if (stringComplete) {     
    //Build an array of values from comma delimited string from Data Streamer
    BuildDataArray(inputString);

    // Set variables based on array index referring to columns:
    // Data Out column A5 = 0, B5 = 1, C5 = 2, etc.
    isPatternLooped = incomingSerialData[1].toInt(); // First cell in Data Out Sheet 
       
    inputString = ""; // reset inputString
    stringComplete = false; // reset stringComplete flag
  }
}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)

  Serial.print(loopTrack);    //Prints the current active column 
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
  getSerialData();
  ParseSerialData();
}

// Gathers bytes from serial port to build inputString
void getSerialData(){
  if(Serial.available()){
    inputString = Serial.readStringUntil('\n');
    stringComplete =true;
    newString = true;
  }
}

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
