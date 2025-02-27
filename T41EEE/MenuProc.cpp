
#include "SDT.h"

int micChoice;
int micGainChoice;
int splitOn = 0;

// Updates by KF5N to CalibrateOptions() function. July 20, 2023
// Updated receive calibration code to clean up graphics.  KF5N August 3, 2023
// ==============  AFP 10-22-22 ==================
/*****
  Purpose: Present the Calibrate options available and return the selection.
  This function is embedded in the mail receiver loop.  It gets called repeatedly during calibration.

  Parameter list:
    void

  Return value
   void
*****/
int CalibrateOptions() {
  int val;
  int freqCorrectionFactorOld = 0;
  int32_t increment = 100L;
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 30, CHAR_HEIGHT, RA8875_BLACK);

  // Select the type of calibration, and then skip this during the loop() function.
  if (calibrateFlag == 0) {
    const char *IQOptions[9]{ "Freq Cal", "CW PA Cal", "Rec Cal", "Xmit Cal", "SSB PA Cal", "Set Tone", "Btn Cal", "Btn Repeat", "Cancel" };  //AFP 10-21-22
    IQChoice = SubmenuSelect(IQOptions, 9, 0);                                                                                                //AFP 10-21-22
  }
  calibrateFlag = 1;
  switch (IQChoice) {

    case 0:  // Calibrate Frequency  - uses WWV
      EEPROMData.freqCorrectionFactor = GetEncoderValueLive(-200000, 200000, EEPROMData.freqCorrectionFactor, increment, (char *)"Freq Cal: ");
      if (EEPROMData.freqCorrectionFactor != freqCorrectionFactorOld) {
        si5351.set_correction(EEPROMData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
        freqCorrectionFactorOld = EEPROMData.freqCorrectionFactor;
      }
      val = ReadSelectedPushButton();
      if (val != BOGUS_PIN_READ) {        // Any button press??
        val = ProcessButtonPress(val);    // Use ladder value to get menu choice
        if (val == MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          EEPROMWrite();
          calibrateFlag = 0;
          IQChoice = 5;
          return IQChoice;
        }
      }
      break;

    case 1:  // CW PA Cal
      EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand], 0.001, (char *)"CW PA Cal: ");
      EEPROMData.powerOutCW[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand];
      val = ReadSelectedPushButton();
      if (val != BOGUS_PIN_READ) {        // Any button press??
        val = ProcessButtonPress(val);    // Use ladder value to get menu choice
        if (val == MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          EEPROMWrite();
          calibrateFlag = 0;
          IQChoice = 5;
          return IQChoice;
        }
      }
      break;
    case 2:                  // IQ Receive Cal - Gain and Phase
      DoReceiveCalibrate();  // This function was significantly revised.  KF5N August 16, 2023
      IQChoice = 5;
      break;
    case 3:                                 // IQ Transmit Cal - Gain and Phase  //AFP 2-21-23
      DoXmitCalibrate(EEPROMData.calFreq);  // This function was significantly revised.  KF5N August 16, 2023
      IQChoice = 5;
      break;
    case 4:  // SSB PA Cal
      EEPROMData.SSBPowerCalibrationFactor[EEPROMData.currentBand] = GetEncoderValueLive(-2.0, 2.0, EEPROMData.SSBPowerCalibrationFactor[EEPROMData.currentBand], 0.001, (char *)"SSB PA Cal: ");
      EEPROMData.powerOutSSB[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.SSBPowerCalibrationFactor[EEPROMData.currentBand];  // AFP 10-21-22
      val = ReadSelectedPushButton();
      if (val != BOGUS_PIN_READ) {        // Any button press??
        val = ProcessButtonPress(val);    // Use ladder value to get menu choice
        if (val == MENU_OPTION_SELECT) {  // Yep. Make a choice??
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          val = ReadSelectedPushButton();
          EEPROMWrite();
          calibrateFlag = 0;
          IQChoice = 5;
          return IQChoice;
        }
      }
      break;  // Missing break.  KF5N August 12, 2023

    case 5:  // Choose CW calibration tone frequency.
      SelectCalFreq();
      Serial.printf("End of case 5 in IQOptions\n");
      calibrateFlag = 0;
      return 0;
      break;

    case 6:  // Calibrate buttons
      SaveAnalogSwitchValues();
      calibrateFlag = 0;
      RedrawDisplayScreen();
      ShowFrequency();
      DrawFrequencyBarValue();
      IQChoice = 6;
      return 6;

    case 7:  // Set button repeat rate
      EEPROMData.buttonRepeatDelay = 1000 * GetEncoderValueLive(0, 5000, EEPROMData.buttonRepeatDelay / 1000, 1, (char *)"Btn Repeat:  ");
      val = ReadSelectedPushButton();
      if (val != BOGUS_PIN_READ) {
        val = ProcessButtonPress(val);
        if (val == MENU_OPTION_SELECT) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 35, CHAR_HEIGHT, RA8875_BLACK);
          EEPROMWrite();
          calibrateFlag = 0;
          IQChoice = 7;
          return IQChoice;
        }
      }
      break;

    case 8:  // Cancelled choice
      //EraseMenus();
      RedrawDisplayScreen();
      currentFreq = TxRxFreq = EEPROMData.centerFreq + NCOFreq;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
      ShowFrequency();
      BandInformation();

      calibrateFlag = 0;
      // centerTuneFlag = 1;  Not used in revised tuning scheme.  July 22, 2023
      modeSelectOutExL.gain(0, 0);
      modeSelectOutExR.gain(0, 0);
      //   ShowSpectrum();  KF5N
      break;

    default:
      micChoice = -1;
      break;
  }
  UpdateEqualizerField(EEPROMData.receiveEQFlag, EEPROMData.xmitEQFlag);
  return 1;
}
// ==============  AFP 10-22-22 ==================
/*****
  Purpose: Present the CW options available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int CWOptions()  // new option for Sidetone and Delay JJP 9/1/22
{
  const char *cwChoices[]{ "WPM", "Key Type", "CW Filter", "Paddle Flip", "CW Offset", "Sidetone Volume", "Transmit Delay", "Cancel" };  // AFP 10-18-22
  int CWChoice = 0;

  CWChoice = SubmenuSelect(cwChoices, 8, 0);

  switch (CWChoice) {
    case 0:  // WPM
      SetWPM();
      SetTransmitDitLength(EEPROMData.currentWPM);  //Afp 09-22-22     // JJP 8/19/23
      break;

    case 1:          // Type of key:
      SetKeyType();  // Straight key or keyer? Stored in EEPROMData.EEPROMData.keyType; no heap/stack variable
      SetKeyPowerUp();
      UpdateWPMField();
      break;

    case 2:              // CW Filter BW:      // AFP 10-18-22
      SelectCWFilter();  // in CWProcessing    // AFP 10-18-22
      break;             // AFP 10-18-22

    case 3:            // Flip paddles
      DoPaddleFlip();  // Stored in EEPROM; variables EEPROMData.paddleDit and EEPROMData.paddleDah
      break;

    case 4:              // Select a preferred CW offset frequency.
      SelectCWOffset();  //  Located in CWProcessing.cpp
      break;

    case 5:  // Sidetone volume
             //  SetEEPROMData.sidetoneVolume();
      SetSideToneVolume();
      break;

    case 6:                // new function JJP 9/1/22
      SetTransmitDelay();  // Transmit relay hold delay
      break;

    default:  // Cancel
      break;
  }
  return CWChoice;
}


/*****
  Purpose: Show the list of scales for the spectrum divisions

  Parameter list:
    void

  Return value
    int           an index into displayScale[] array, or -1 on cancel
*****/
int SpectrumOptions() { /*
  dispSc displayScale[] =  //r *dbText,dBScale, pixelsPerDB, baseOffset, offsetIncrement
  {
    {"20 dB/", 10.0,   2,  24, 1.00},
    {"10 dB/", 20.0,   4,  10, 0.50},  // JJP 7/14/23
    {"5 dB/",  40.0,   8,  58, 0.25},
    {"2 dB/",  100.0, 20, 120, 0.10},
    {"1 dB/",  200.0, 40, 200, 0.05}
  };
  */
  const char *spectrumChoices[] = { "20 dB/unit", "10 dB/unit", "5 dB/unit", "2 dB/unit", "1 dB/unit", "Cancel" };
  int spectrumSet = EEPROMData.currentScale;  // JJP 7/14/23

  spectrumSet = SubmenuSelect(spectrumChoices, 6, spectrumSet);
  if (strcmp(spectrumChoices[spectrumSet], "Cancel") == 0) {
    return EEPROMData.currentScale;  // Nope.
  }
  EEPROMData.currentScale = spectrumSet;  // Yep...
  //EEPROMData.currentScale = EEPROMData.currentScale;
  EEPROMWrite();
  RedrawDisplayScreen();
  ShowSpectrumdBScale();
  return spectrumSet;
}
/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int AGCOptions() {
  const char *AGCChoices[] = { "Off", "Long", "Slow", "Medium", "Fast", "Cancel" };  // G0ORX (Added Long) September 5, 2023

  EEPROMData.AGCMode = SubmenuSelect(AGCChoices, 6, EEPROMData.AGCMode);  // G0ORX
  if (EEPROMData.AGCMode == 5) {
    return EEPROMData.AGCMode;  // Nope.
  }

  AGCLoadValues();  // G0ORX September 5, 2023

  EEPROMData.AGCMode = EEPROMData.AGCMode;          // Store in EEPROM and...
  EEPROM.put(EEPROM_BASE_ADDRESS + 4, EEPROMData);  // ...save it
  UpdateAGCField();
  return EEPROMData.AGCMode;
}
/*****
  Purpose: IQ Options

  Parameter list:
    void

  Return value
****
int IQOptions()  //============================== AFP 10-22-22  All new
{
  calibrateFlag = 1;
  const char* IQOptions[7] { "Freq Cal", "CW PA Cal", "Rec Cal", "Xmit Cal", "SSB PA Cal", "Set Tone", "Cancel" };  //AFP 10-21-22
  //const char *IQOptions[] = {"Rec Cal", "Xmit Cal", "Freq Cal", "SSB PA Cal", "CW PA Cal", "Cancel"}; //AFP 10-21-22
  IQChoice = SubmenuSelect(IQOptions, 7, 0);  //AFP 10-21-22
  Serial.printf("IQOptions returns %d\n", IQChoice);
  CalibrateOptions(IQChoice);
  return IQChoice;
}
*/


/*****
  Purpose: To process the graphics for the 14 chan equalizer option

  Parameter list:
    int array[]         The array to fill in.  0 is receive, 1 is transmit.
    char *title         The equalizer being set.
  Return value
    void
*****/
void ProcessEqualizerChoices(int EQType, char *title) {
  for (int i = 0; i < EQUALIZER_CELL_COUNT; i++) {
  }
  const char *eqFreq[] = { " 200", " 250", " 315", " 400", " 500", " 630", " 800",
                           "1000", "1250", "1600", "2000", "2500", "3150", "4000" };
  int yLevel[EQUALIZER_CELL_COUNT];  // EQUALIZER_CELL_COUNT 14

  int columnIndex;
  int iFreq;
  int newValue;
  int xOrigin = 50;
  int xOffset;
  int yOrigin = 50;
  int wide = 700;
  int high = 300;
  int barWidth = 46;
  int barTopY;
  int barBottomY;
  int val;

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    if (EQType == 0) {
      yLevel[iFreq] = EEPROMData.equalizerRec[iFreq];
    } else {
      if (EQType == 1) {
        yLevel[iFreq] = EEPROMData.equalizerXmt[iFreq];
      }
    }
  }
  tft.writeTo(L2);
  tft.clearMemory();
  tft.writeTo(L1);
  tft.fillWindow(RA8875_BLACK);

  tft.fillRect(xOrigin - 50, yOrigin - 25, wide + 50, high + 50, RA8875_BLACK);  // Clear data area
  tft.setTextColor(RA8875_GREEN);
  tft.setFontScale((enum RA8875tsize)1);
  tft.setCursor(200, 0);
  tft.print(title);

  tft.drawRect(xOrigin - 4, yOrigin, wide + 4, high, RA8875_BLUE);
  tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED);  // Print center zero line center
  tft.setFontScale((enum RA8875tsize)0);

  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + tft.getFontHeight());
  tft.print("+12");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + (high / 2) - tft.getFontHeight());
  tft.print(" 0");
  tft.setCursor(xOrigin - 4 - tft.getFontWidth() * 3, yOrigin + high - tft.getFontHeight() * 2);
  tft.print("-12");

  barTopY = yOrigin + (high / 2);                // 50 + (300 / 2) = 200
  barBottomY = barTopY + DEFAULT_EQUALIZER_BAR;  // Default 200 + 100

  for (iFreq = 0; iFreq < EQUALIZER_CELL_COUNT; iFreq++) {
    tft.fillRect(xOrigin + (barWidth + 4) * iFreq, barTopY - (yLevel[iFreq] - DEFAULT_EQUALIZER_BAR), barWidth, yLevel[iFreq], RA8875_CYAN);
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq, yOrigin + high - tft.getFontHeight() * 2);
    tft.print(eqFreq[iFreq]);
    tft.setCursor(xOrigin + (barWidth + 4) * iFreq + tft.getFontWidth() * 1.5, yOrigin + high + tft.getFontHeight() * 2);
    tft.print(yLevel[iFreq]);
  }

  columnIndex = 0;  // Get ready to set values for columns
  newValue = 0;
  while (columnIndex < EQUALIZER_CELL_COUNT) {
    xOffset = xOrigin + (barWidth + 4) * columnIndex;  // Just do the math once
                                                       //    tft.fillRect(xOffset,                               // Indent to proper bar... Removed this rectangle.  Seems unnecessary.  KF5N November 12, 2023
                                                       //                 barBottomY - yLevel[columnIndex] - 1,  // Start at red line
                                                       //                 barBottomY - 1,
                                                       //                 barWidth,                              // Set bar width
                                                       //                 newValue + 1,                          // Erase old bar
                                                       //                 -100,
                                                       //                 RA8875_BLACK);

    tft.fillRect(xOffset,                           // Indent to proper bar...
                 barBottomY - yLevel[columnIndex],  // Start at red line
                 barWidth,                          // Set bar width
                 yLevel[columnIndex],               // Draw new bar
                 RA8875_MAGENTA);
    while (true) {
      newValue = yLevel[columnIndex];  // Get current value
      if (filterEncoderMove != 0) {

        tft.fillRect(xOffset,                    // Indent to proper bar...
                     barBottomY - newValue - 1,  // Start at red line
                     barWidth,                   // Set bar width
                     newValue + 1,               // Erase old bar
                     RA8875_BLACK);
        newValue += (PIXELS_PER_EQUALIZER_DELTA * filterEncoderMove);  // Find new bar height. OK since filterEncoderMove equals 1 or -1
        tft.fillRect(xOffset,                                          // Indent to proper bar...
                     barBottomY - newValue,                            // Start at red line
                     barWidth,                                         // Set bar width
                     newValue,                                         // Draw new bar
                     RA8875_MAGENTA);
        yLevel[columnIndex] = newValue;

        tft.fillRect(xOffset + tft.getFontWidth() * 1.5 - 1, yOrigin + high + tft.getFontHeight() * 2,  // Update bottom number
                     barWidth, CHAR_HEIGHT, RA8875_BLACK);
        tft.setCursor(xOffset + tft.getFontWidth() * 1.5, yOrigin + high + tft.getFontHeight() * 2);
        tft.print(yLevel[columnIndex]);
        if (newValue < DEFAULT_EQUALIZER_BAR) {  // Repaint red center line if erased
          tft.drawFastHLine(xOrigin - 4, yOrigin + (high / 2), wide + 4, RA8875_RED);
          ;  // Clear hole in display center
        }
      }
      filterEncoderMove = 0;
      MyDelay(200L);

      val = ReadSelectedPushButton();  // Read the ladder value

      if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
        val = ProcessButtonPress(val);  // Use ladder value to get menu choice
        MyDelay(100L);

        tft.fillRect(xOffset,                // Indent to proper bar...
                     barBottomY - newValue,  // Start at red line
                     barWidth,               // Set bar width
                     newValue,               // Draw new bar
                     RA8875_GREEN);

        if (EQType == 0) {
          EEPROMData.equalizerRec[columnIndex] = newValue;
          //       EEPROMData.equalizerRec[columnIndex] = recEQ_Level[columnIndex];
        } else {
          if (EQType == 1) {
            EEPROMData.equalizerXmt[columnIndex] = newValue;
            //      EEPROMData.equalizerXmt[columnIndex] = EEPROMData.xmtEQ_Level[columnIndex];
          }
        }

        filterEncoderMove = 0;
        columnIndex++;
        break;
      }
      //recEQ_Level[columnIndex] = (float)array[columnIndex];  //AFP 08-09-22
      //EEPROMData.equalizerRec[columnIndex] = recEQ_Level[columnIndex];
    }  // end inner while
  }    // end outer while

  EEPROMWrite();
}

/*****
  Purpose: Receive EQ set

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int EqualizerRecOptions() {
  const char *RecEQChoices[] = { "On", "Off", "EQSet", "Cancel" };  // Add code practice oscillator
  int EQChoice = 0;

  EQChoice = SubmenuSelect(RecEQChoices, 4, 0);

  switch (EQChoice) {
    case 0:
      EEPROMData.receiveEQFlag = true;
      break;
    case 1:
      EEPROMData.receiveEQFlag = false;
      break;
    case 2:
      ProcessEqualizerChoices(0, (char *)"Receive Equalizer");
      break;
    case 3:
      return 0;  // Do nothing and return.
      break;
  }
  EEPROMWrite();
  RedrawDisplayScreen();
  UpdateEqualizerField(EEPROMData.receiveEQFlag, EEPROMData.xmitEQFlag);
  return 0;
}

/*****
  Purpose: Xmit EQ options

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int EqualizerXmtOptions() {
  const char *XmtEQChoices[] = { "On", "Off", "EQSet", "Cancel" };  // Add code practice oscillator
  int EQChoice = 0;

  EQChoice = SubmenuSelect(XmtEQChoices, 4, 0);

  switch (EQChoice) {
    case 0:
      EEPROMData.xmitEQFlag = true;
      break;
    case 1:
      EEPROMData.xmitEQFlag = false;
      break;
    case 2:
      ProcessEqualizerChoices(1, (char *)"Transmit Equalizer");
      break;
    case 3:  // Do nothing and exit.
      return 0;
      break;
  }
  EEPROMWrite();
  RedrawDisplayScreen();
  UpdateEqualizerField(EEPROMData.receiveEQFlag, EEPROMData.xmitEQFlag);
  return 0;
}


/*****
  Purpose: Set Mic level

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int MicGainSet() {
  //=====
  const char *micGainChoices[] = { "Set Mic Gain", "Cancel" };
  micGainChoice = SubmenuSelect(micGainChoices, 2, micGainChoice);
  switch (micGainChoice) {
    case 0:
      int val;
      // EEPROMData.currentMicGain = EEPROMData.EEPROMData.currentMicGain;  // AFP 09-22-22
      tft.setFontScale((enum RA8875tsize)1);
      tft.fillRect(SECONDARY_MENU_X - 50, MENUS_Y, EACH_MENU_WIDTH + 50, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setTextColor(RA8875_WHITE);
      tft.setCursor(SECONDARY_MENU_X - 48, MENUS_Y + 1);
      tft.print("Mic Gain:");
      tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
      tft.print(EEPROMData.currentMicGain);
      while (true) {
        if (filterEncoderMove != 0) {
          EEPROMData.currentMicGain += ((float)filterEncoderMove);
          if (EEPROMData.currentMicGain < -40)
            EEPROMData.currentMicGain = -40;
          else if (EEPROMData.currentMicGain > 30)  // 100% max
            EEPROMData.currentMicGain = 30;
          tft.fillRect(SECONDARY_MENU_X + 180, MENUS_Y, 80, CHAR_HEIGHT, RA8875_MAGENTA);
          tft.setCursor(SECONDARY_MENU_X + 180, MENUS_Y + 1);
          tft.print(EEPROMData.currentMicGain);
          filterEncoderMove = 0;
        }
        val = ReadSelectedPushButton();  // Read pin that controls all switches
        val = ProcessButtonPress(val);
        //MyDelay(150L);
        if (val == MENU_OPTION_SELECT) {  // Make a choice??
                                          //         EEPROMData.EEPROMData.currentMicGain = EEPROMData.currentMicGain;
          EEPROMWrite();
          break;
        }
      }
    case 1:
      break;
  }
  return micGainChoice;
  //  EraseMenus();
}
/*****
  Purpose: Turn mic compression on and set the level

  Parameter list:
    void

  Return value
    int           an index into the band array
*****/
int MicOptions()  // AFP 09-22-22 All new
{
  const char *micChoices[] = { "On", "Off", "Set Threshold", "Set Comp_Ratio", "Set Attack", "Set Decay", "Cancel" };

  micChoice = SubmenuSelect(micChoices, 7, micChoice);
  switch (micChoice) {
    case 0:                           // On
      EEPROMData.compressorFlag = 1;  // AFP 09-22-22
      UpdateCompressionField();       // JJP 8/26/2023
      break;
    case 1:  // Off
      EEPROMData.compressorFlag = 0;
      UpdateCompressionField();  // JJP 8/26/2023
      break;
    case 2:
      SetCompressionLevel();
      break;
    case 3:
      SetCompressionRatio();
      break;
    case 4:
      SetCompressionAttack();
      break;
    case 5:
      SetCompressionRelease();
      break;
    case 6:
      break;
    default:  // Cancelled choice
      micChoice = -1;
      break;
  }
  secondaryMenuIndex = -1;
  return micChoice;
}

/*****
  Purpose: Present the bands available and return the selection

  Parameter list:
    void

  Return value12
    int           an index into the band array
*****/
int RFOptions() {
  const char *rfOptions[] = { "Power level", "Gain", "Auto-Gain On", "Auto-Gain Off", "Cancel" };
  int rfSet = 0;
  int returnValue = 0;

  rfSet = SubmenuSelect(rfOptions, 5, rfSet);

  switch (rfSet) {
    case 0:  // AFP 10-21-22
      EEPROMData.transmitPowerLevel = (float)GetEncoderValue(1, 20, EEPROMData.transmitPowerLevel, 1, (char *)"Power: ");
      if (EEPROMData.xmtMode == CW_MODE) {                                                                                                                                                                                                        //AFP 10-13-22
        EEPROMData.powerOutCW[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.CWPowerCalibrationFactor[EEPROMData.currentBand];  //  afp 10-21-22
      } else {                                                                                                                                                                                                                                    //AFP 10-13-22
        if (EEPROMData.xmtMode == SSB_MODE) {
          EEPROMData.powerOutSSB[EEPROMData.currentBand] = (-.0133 * EEPROMData.transmitPowerLevel * EEPROMData.transmitPowerLevel + .7884 * EEPROMData.transmitPowerLevel + 4.5146) * EEPROMData.SSBPowerCalibrationFactor[EEPROMData.currentBand];  // afp 10-21-22                                                                                                                                                                                                                                                      //         EEPROMData.EEPROMData.powerOutSSB[EEPROMData.currentBand] = EEPROMData.powerOutSSB[EEPROMData.currentBand];                                                                                                //AFP 10-21-22
        }
      }
      EEPROMData.transmitPowerLevel = EEPROMData.transmitPowerLevel;  //AFP 10-21-22
      EEPROMWrite();                                                  //AFP 10-21-22

      BandInformation();
      break;

    case 1:                                                                                                                                        // Gain
      EEPROMData.rfGain[EEPROMData.currentBand] = GetEncoderValue(-60, 10, EEPROMData.rfGain[EEPROMData.currentBand], 5, (char *)"RF Gain dB: ");  // Argument: min, max, start, increment
      //EEPROMData.rfGainAllBands = EEPROMData.rfGainAllBands;
      EEPROMWrite();
      returnValue = EEPROMData.rfGain[EEPROMData.currentBand];
      break;

    case 2:                        // Auto-Gain On                                                                                                  // Gain
      EEPROMData.autoGain = true;  // Argument: min, max, start, increment
      EEPROMWrite();
      returnValue = true;
      break;

    case 3:  // Auto-Gain Off
      EEPROMData.autoGain = false;
      EEPROMWrite();
      returnValue = false;
      break;

      // Where is the 3rd option and default???
  }
  return returnValue;
}


/*****
  Purpose: This option reverses the dit and dah paddles on the keyer

  Parameter list:
    void

  Return value
    void
*****/
void DoPaddleFlip() {
  const char *paddleState[] = { "Right paddle = dah", "Right paddle = dit" };
  int choice, lastChoice;
  int pushButtonSwitchIndex;
  int valPin;

  EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;  // Defaults
  EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
  choice = lastChoice = 0;

  tft.setTextColor(RA8875_BLACK);
  tft.fillRect(SECONDARY_MENU_X - 100, MENUS_Y, EACH_MENU_WIDTH + 100, CHAR_HEIGHT, RA8875_GREEN);
  tft.setCursor(SECONDARY_MENU_X - 93, MENUS_Y + 1);
  tft.print(paddleState[choice]);  // Show the default (right paddle = dah

  while (true) {
    MyDelay(150L);
    valPin = ReadSelectedPushButton();                     // Poll buttons
    if (valPin != -1) {                                    // button was pushed
      pushButtonSwitchIndex = ProcessButtonPress(valPin);  // Winner, winner...chicken dinner!
      if (pushButtonSwitchIndex == MAIN_MENU_UP || pushButtonSwitchIndex == MAIN_MENU_DN) {
        choice = !choice;  // Reverse the last choice
        tft.fillRect(SECONDARY_MENU_X - 100, MENUS_Y, EACH_MENU_WIDTH + 100, CHAR_HEIGHT, RA8875_GREEN);
        tft.setCursor(SECONDARY_MENU_X - 93, MENUS_Y + 1);
        tft.print(paddleState[choice]);
      }
      if (pushButtonSwitchIndex == MENU_OPTION_SELECT) {  // Made a choice??
        if (choice) {                                     // Means right-paddle dit
          EEPROMData.paddleDit = KEYER_DAH_INPUT_RING;
          EEPROMData.paddleDah = KEYER_DIT_INPUT_TIP;
          EEPROMData.paddleFlip = 1;  // KD0RC
        } else {
          EEPROMData.paddleDit = KEYER_DIT_INPUT_TIP;
          EEPROMData.paddleDah = KEYER_DAH_INPUT_RING;
          EEPROMData.paddleFlip = 0;  // KD0RC
        }
        //EEPROMEEPROMData.paddleDit = EEPROMData.paddleDit;
        //EEPROMData.EEPROMData.paddleDah = EEPROMData.paddleDah;
        EraseMenus();
        UpdateWPMField();  // KD0RC
        return;
      }
    }
  }
  EEPROMWrite();
}


/*****
  Purpose: Used to change the currently active VFO

  Parameter list:
    void

  Return value
    int             // the currently active VFO, A = 1, B = 0
*****/
int VFOSelect() {
  const char *VFOOptions[] = { "VFO A", "VFO B", "Split", "Cancel" };
  int toggle;
  int choice, lastChoice;

  choice = lastChoice = toggle = EEPROMData.activeVFO;
  splitOn = 0;

  tft.setTextColor(RA8875_BLACK);
  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);
  tft.setCursor(SECONDARY_MENU_X + 7, MENUS_Y + 1);
  tft.print(VFOOptions[choice]);  // Show the default (right paddle = dah

  choice = SubmenuSelect(VFOOptions, 4, 0);
  MyDelay(10);
  NCOFreq = 0L;
  switch (choice) {
    case VFO_A:  // VFO A
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqA;
      EEPROMData.activeVFO = VFO_A;
      EEPROMData.currentBand = EEPROMData.currentBandA;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);  // Erase split message
      splitOn = 0;
      break;

    case VFO_B:  // VFO B
      EEPROMData.centerFreq = TxRxFreq = EEPROMData.currentFreqB;
      EEPROMData.activeVFO = VFO_B;
      EEPROMData.currentBand = EEPROMData.currentBandB;
      tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);  // Erase split message
      splitOn = 0;
      break;

    case VFO_SPLIT:  // Split
      DoSplitVFO();
      splitOn = 1;
      break;

    default:  // Cancel
      return EEPROMData.activeVFO;
      break;
  }
  bands[EEPROMData.currentBand].freq = TxRxFreq;
  SetBand();           // KF5N July 12, 2023
  SetBandRelay(HIGH);  // Required when switching VFOs. KF5N July 12, 2023
  SetFreq();
  RedrawDisplayScreen();
  BandInformation();
  ShowBandwidth();
  FilterBandwidth();
  tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);  // delete old digit
  tft.fillRect(FREQUENCY_X, FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);        // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
  ShowFrequency();
  // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
  tft.writeTo(L2);
  tft.clearMemory();
  if (EEPROMData.xmtMode == CW_MODE) BandInformation();
  DrawBandWidthIndicatorBar();
  DrawFrequencyBarValue();
  UpdateDecoderField();
  return EEPROMData.activeVFO;
}

/*****
  Purpose: Allow user to set current EEPROM values or restore default settings

  Parameter list:
    void

  Return value
    int           the user's choice
*****/
int EEPROMOptions() {  // 0               1                2               3               4                  5                  6                  7                   8                  9
  const char *EEPROMOpts[] = { "Save Current", "Load Defaults", "Get Favorite", "Set Favorite", "Copy EEPROM->SD", "Copy SD->EEPROM", "EEPROM->Serial", "Default->Serial", "Stack->Serial", "SD->Serial", "Cancel" };
  int defaultOpt = 0;
  config_t tempConfig;     // A temporary config_t struct to copy EEPROM data into.
  config_t defaultConfig;  // The configuration defaults.
  defaultOpt = SubmenuSelect(EEPROMOpts, 11, defaultOpt);
  switch (defaultOpt) {
    case 0:  // Save current EEPROMData struct to EEPROM non-volatile memory.
      EEPROMWrite();
      break;

    case 1:
      EEPROMDataDefaults();  // Restore defaults to EEPROMData struct and refresh display.
      break;

    case 2:
      GetFavoriteFrequency();  // Get a stored frequency and store in active VFO
      break;

    case 3:
      SetFavoriteFrequency();  // Set favorites
      break;

    case 4:                                             // Copy EEPROM->SD.
      EEPROM.get(EEPROM_BASE_ADDRESS + 4, tempConfig);  // Read as one large chunk
      saveConfiguration(filename, tempConfig, true);    // Save EEPROM struct to SD
      break;

    case 5:                                     // Copy SD->EEPROM
      loadConfiguration(filename, EEPROMData);  // Copy from SD to struct in active memory (on the stack) EEPROMData.
      EEPROMWrite();                            // Write to EEPROM non-volatile memory.
      tft.writeTo(L2);                          // This is specifically to clear the bandwidth indicator bar.  KF5N August 7, 2023
      tft.clearMemory();
      tft.writeTo(L1);
      //  Read the revised struct into active memory.
      //EEPROMRead();
      // Initialize the frequency setting based on the last used frequency stored to EEPROM.
      TxRxFreq = EEPROMData.centerFreq = EEPROMData.lastFrequencies[EEPROMData.currentBand][EEPROMData.activeVFO];
      // Set the frequency correction of the Si5351:
      si5351.set_correction(EEPROMData.freqCorrectionFactor, SI5351_PLL_INPUT_XO);
      RedrawDisplayScreen();  // Assume there are lots of changes and do a heavy-duty refresh.  KF5N August 7, 2023
      break;

    case 6:  // EEPROM->Serial
      {
        Serial.println(F("\nBegin EEPROMData from EEPROM"));
        // Don't want to overwrite the stack.  Need a temporary struct, read the EEPROM data into that.
        config_t EEPROMData_temp;
        EEPROM.get(EEPROM_BASE_ADDRESS + 4, EEPROMData_temp);
        saveConfiguration(filename, EEPROMData_temp, false);  // Write the temporary struct to the serial monitor.
        Serial.println(F("\nEnd EEPROMData from EEPROM\n"));
      }
      break;

    case 7:  // Defaults->Serial
      Serial.println(F("\nBegin EEPROMData defaults"));
      saveConfiguration(filename, defaultConfig, false);  // Write default EEPROMData struct to the Serial monitor.
      Serial.println(F("\nEnd EEPROMData defaults\n"));
      break;

    case 8:  // Current->Serial
      Serial.println(F("Begin EEPROMData on the stack"));
      saveConfiguration(filename, EEPROMData, false);  // Write current EEPROMData struct to the Serial monitor.
      Serial.println(F("\nEnd EEPROMData on the stack\n"));
      break;

    case 9:  // SDEEPROMData->Serial
      Serial.println(F("Begin EEPROMData on the SD card"));
      printFile(filename);  // Write SD card EEPROMData struct to the Serial monitor.
      Serial.println(F("End EEPROMData on the SD card\n"));
      break;

    default:
      defaultOpt = -1;  // No choice made
      break;
  }
  return defaultOpt;
}


/*****
  Purpose: To select an option from a submenu

  Parameter list:
    char *options[]           submenus
    int numberOfChoices       choices available
    int defaultState          the starting option

  Return value
    int           an index into the band array
const char *topMenus[] = { "Bearing", "CW Options", "RF Set", "VFO Select",
                           "EEPROM", "AGC", "Spectrum Options",
                           "Noise Floor", "Mic Gain", "Mic Comp",
                           "EQ Rec Set", "EQ Xmt Set", "Calibrate" };

int (*functionPtr[])() = { &BearingMaps, &CWOptions, &RFOptions, &VFOSelect,
                           &EEPROMOptions, &AGCOptions, &SpectrumOptions,
                           &ButtonSetNoiseFloor, &MicGainSet, &MicOptions,
                           &EqualizerRecOptions, &EqualizerXmtOptions, &IQOptions

};
  const char *labels[]        = {"Select",       "Menu Up",  "Band Up",
                               "Zoom",         "Menu Dn",  "Band Dn",
                               "Filter",       "DeMod",    "Mode",
                               "NR",           "Notch",    "Noise Floor",
                               "Fine Tune",    "Decoder",  "Tune Increment",
                               "User 1",       "User 2",   "User 3"
                              };

*****/
int SubmenuSelect(const char *options[], int numberOfChoices, int defaultStart) {
  int refreshFlag = 0;
  int val;
  int encoderReturnValue;

  tft.setTextColor(RA8875_BLACK);
  encoderReturnValue = defaultStart;  // Start the options using this option

  tft.setFontScale((enum RA8875tsize)1);
  if (refreshFlag == 0) {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
    tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
    tft.print(options[encoderReturnValue]);  // Secondary Menu
    refreshFlag = 1;
  }
  MyDelay(150L);

  while (true) {
    val = ReadSelectedPushButton();  // Read the ladder value
    MyDelay(150L);
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);  // Use ladder value to get menu choice
      if (val > -1) {                 // Valid choice?
        switch (val) {
          case MENU_OPTION_SELECT:  // They made a choice
            tft.setTextColor(RA8875_WHITE);
            EraseMenus();
            return encoderReturnValue;
            break;

          case MAIN_MENU_UP:
            encoderReturnValue++;
            if (encoderReturnValue >= numberOfChoices)
              encoderReturnValue = 0;
            break;

          case MAIN_MENU_DN:
            encoderReturnValue--;
            if (encoderReturnValue < 0)
              encoderReturnValue = numberOfChoices - 1;
            break;

          default:
            encoderReturnValue = -1;  // An error selection
            break;
        }
        if (encoderReturnValue != -1) {
          tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_GREEN);  // Show the option in the second field
          tft.setTextColor(RA8875_BLACK);
          tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
          tft.print(options[encoderReturnValue]);
          MyDelay(50L);
          refreshFlag = 0;
        }
      }
    }
  }
}
