/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*----------------------------setAvgSamples()--------------------------------------------
set samples options
measure - samples register size, default & alternate
calibrate - samples register size
*/
void setAvgSamples()
{
	int x, y;
	int n;
	int samplesStat;									// samplesAvg status

	if (avgSamples == optAvgCal.val)				// save current samplesAvg setting
		samplesStat = 1;								// restore on exit
	if (avgSamples == optAvgDefault.val)
		samplesStat = 2;
	if (avgSamples == optAvgAlt.val)
		samplesStat = 3;

	tft.fillScreen(BG_COLOUR);
	tft.setTextColor(WHITE);

	// screen header, centred
	tft.setFont(FONT12);
	char txt[] = "Fast - Slow Averaging Options (1-100%)";
	tft.setCursor((320 - tft.strPixelLen(txt)) / 2, 1);
	tft.printf(txt);

	// touch area index
	int index = 0;
	drawPlusMinusOpts(samplesSlowOpt, "Calibrate Sampling", index);			// returns new i
	index += 2;
	displayValue(samplesSlowOpt, optAvgCal.val);

	drawPlusMinusOpts(samplesMedOpt, "Default Sampling", index);		// returns new i
	index += 2;
	displayValue(samplesMedOpt, optAvgDefault.val);

	drawPlusMinusOpts(samplesFastOpt, "Aternate Sampling", index);
	index += 2;
	displayValue(samplesFastOpt, optAvgAlt.val);

	drawPlusMinusOpts(weighting, "Decay Weight (.01-1.0)", index);
	index += 2;
	displayValue(weighting, (float)optAvgWeight.val / 1000);

	x = 135; y = 210;
	drawTextBoxOpts(x, y, "Exit", index);


	// touch screen options
	do
	{
		n = chkTouchOption(index);						// check which box touched
		switch (n)
		{
		case 0:										// increment sample size, limit to max
			if (optAvgCal.val == 1)
				optAvgCal.val += SAMPLES_CHANGE - 1;
			else
				optAvgCal.val += SAMPLES_CHANGE;
			if (optAvgCal.val >= 100)
				optAvgCal.val = 100;
			break;
		case 1:										// decrement sample size, min = 1
			optAvgCal.val -= SAMPLES_CHANGE;
			if (optAvgCal.val <= 1)
				optAvgCal.val = 1;
			break;
		case 2:										// increment sample size, limit to max
			if (optAvgDefault.val == 1)
				optAvgDefault.val += SAMPLES_CHANGE - 1;
			else optAvgDefault.val += SAMPLES_CHANGE;
			if (optAvgDefault.val >= 100)
				optAvgDefault.val = 100;
			break;
		case 3:										// decrement sample size, min = 1
			optAvgDefault.val -= SAMPLES_CHANGE;
			if (optAvgDefault.val <= 1)
				optAvgDefault.val = 1;
			break;
		case 4:										// increment calibrate sample size
			if (optAvgAlt.val == 1)
				optAvgAlt.val += SAMPLES_CHANGE - 1;
			else optAvgAlt.val += SAMPLES_CHANGE;
			if (optAvgAlt.val >= 100)
				optAvgAlt.val = 100;
			break;
		case 5:										// decrement calibrate sample size, min = 1
			optAvgAlt.val -= SAMPLES_CHANGE;
			if (optAvgAlt.val <= 1)
				optAvgAlt.val = 1;
			break;
		case 6:										// exponential weight for averaging
			optAvgWeight.val += 10;					// max: 1000/1000 = 1.0
			if (optAvgWeight.val >= 1000)
				optAvgWeight.val = 1000;
			break;
		case 7:										// decrement calibrate sample size, min = 1

			if (optAvgWeight.val > 100)				// 0.1
			{
				optAvgWeight.val -= 50;
				if (optAvgWeight.val <= 50)				// min: 50/1000 = .05
					optAvgWeight.val = 50;
			}
			else
			{
				optAvgWeight.val -= 10;
				if (optAvgWeight.val <= 5)				// min: 50/1000 = .005
					optAvgWeight.val = 5;
			}
			break;
		default:
			break;
		}

		// re-display parameters
		displayValue(samplesSlowOpt, optAvgCal.val);
		displayValue(samplesMedOpt, optAvgDefault.val);
		displayValue(samplesFastOpt, optAvgAlt.val);
		displayValue(weighting, (float)optAvgWeight.val / 1000);


		// save to EEPROM
		EEPROM.put(optAvgCal.eeAddr, optAvgCal);;
		EEPROM.put(optAvgDefault.eeAddr, optAvgDefault);;
		EEPROM.put(optAvgAlt.eeAddr, optAvgAlt);;
		EEPROM.put(optAvgWeight.eeAddr, optAvgWeight);;
	} while (n < index);								// do while touched item is less than toatl items


	switch (samplesStat)							// reset samplesAvg using new values
	{
	case 1:
		avgSamples = optAvgCal.val;
		break;
	case 2:
		avgSamples = optAvgDefault.val;
		break;
	case 3:
		avgSamples = optAvgAlt.val;
	}
	// clean up
	initDisplay();
}

/*-------------------------drawPlusMinusOpts()--------------------------------------
	draw text and value, +/- buttons
	returns touch index = original i+2
*/
void drawPlusMinusOpts(int posn, const char* txt, int tIndex)
{
	int x = 30, y;
	int offSet = 10;
	frame* fPtr = &fr[posn];

	ILI9341_t3_font_t fnt = FONT14;
	tft.setFont(fnt);
	tft.setTextColor(WHITE);
	y = fr[posn].y + fnt.cap_height;
	tft.setCursor(x, y);
	tft.printf(txt);
	restoreFrame(posn);

	// font for Plus - minus symbols
	tft.setFont(FONT_PM);
	tft.setTextColor(WHITE);

	// draw +, save + touch index, x,y
	x = fPtr->x + fPtr->w + 5;
	y = fPtr->y;
	tft.setCursor(x, y);
	tft.write(PLUS_SYMBOL);
	x += offSet;
	y += offSet;
	tb[tIndex].x = x;
	tb[tIndex].y = y;
	tIndex++;

	// draw -, save touch index, x,y
	x = fPtr->x + fPtr->w + 5;
	y = y + 10;
	tft.setCursor(x, y);
	tft.write(MINUS_SYMBOL);
	x += offSet;
	y += offSet;
	tb[tIndex].x = x;
	tb[tIndex].y = y;
}



/*------------------------drawTextBoxOpts()----------------------------
draws touch box with txt with touch index

args: x,y, text in box, drawTouchBoxOpts[num]
returns:	box width
*/
int drawTextBoxOpts(int x, int y, const char* txt, int tIndex)
{
	int r = 5;
	int s, w, h;						//  string length, box width, height adjustment
	ILI9341_t3_font_t fnt = FONT16;
	char t[10];

	strcpy(t, txt);
	// set font, get width and height from font & text
	tft.setFont(fnt);
	s = tft.strPixelLen(t);							// width of string
	w = s + 20;										// box width
	h = fnt.cap_height + 10;
	tft.drawRoundRect(x, y, w, h, r, WHITE);
	tft.setCursor(x + (w - s) / 2, y + 5);
	tft.print(txt);

	tb[tIndex].x = x + w / 2;
	tb[tIndex].y = y + 10;

	// return width
	return w;
}

/*------------------drawCircleOpts()---------------------------------------
darw filled / blank circle at x, y position
flag: 1 = filled circle
tiindex: touch box index
*/
void drawCircleOpts(int x, int y, bool isFlg, int tIndex)
{
	int r = 10;

	if (isFlg)
	{
		// normal fill
		tft.fillCircle(x, y, r, WHITE);
		tft.drawCircle(x, y, r, WHITE);
	}
	else
	{
		// alternate B/Ground fill
		tft.fillCircle(x, y, r, BG_COLOUR);
		tft.drawCircle(x, y, r, WHITE);
	}
	// save touch index, x&y
	tb[tIndex].x = x;
	tb[tIndex].y = y;
}


