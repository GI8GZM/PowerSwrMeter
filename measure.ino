/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerSwrMeter

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/* -------------------------- netPwrProcess ------------------------------
process net power - immediate fast attack, exponentailly decayed
*/
float netPwrProcess(float currPwr, float weight)
{
	static float prevPwr = 0;
	float pwr = sigProcess(currPwr, prevPwr, weight);
	prevPwr = pwr;
	return pwr;
}

/* ------------------pkPwrProcess -----------------------------------
process pkpower - immediate fast attack, exponentailly decayed
*/
float pkPwrProcess(float currPwr, float weight)
{
	static float prevPwr = 0;
	float pwr = sigProcess(currPwr, prevPwr, weight);
	prevPwr = pwr;
	return pwr;
}


/*----------------- sigProcess --------------------------------------
signal processing - fast attack, exponential decay
weight controls decay
*/

float sigProcess(float currSig, float prevSig, float weight)
{
	float sig;
	// rising power processing - fast attack - immediate step up to higher value and hold
	if (currSig > prevSig)
		sig = currSig;

	// exponential decay
	//if (currSig <= prevSig)
	else
		sig = weight * currSig + (1 - weight) * prevSig; // exponential decay
	return sig;
}

/*--------------------------- measure() ------------------------------------------
   reads ADC values recorded by ADC using interrupt timer - getADC().
   calculates forward, reflected power, net power, peak envelope power
   and peak power calculated and held for 3-4 secs
   swr calculated from fwd and reflected power
   Calls: pwrCalc(), displayValue(), restoreFrame(), resetDimmer()
*/
void measure()
{
	unsigned long fA, rA, fPk, rPk;										// variables from adc interrupts
	float  fwdV, refV, fwdPkV, refPkV;									// calulated ADC voltages
	float fwdPwr, refPwr, fwdPkPwr, refPkPwr;							// calculated powers
	float netPwr, pep, dB, swr = 1.0;
	static float netPwrHold = 0, pkPwr = 0;
	//static float netPwrPkHold = 0;
	//static float pepHold = 0;

	float w = (float)optAvgWeight.val / 1000;								// weighting for exponential  smoothing

	float adcConvert = 3.3 / adc->adc0->getMaxValue();					// 3.3 (max volts) / adc max value, varies with resolution+
	static float fwdVPrev = 0, refVPrev = 0;							// variables for exponential smoothing
	static float fwdPkVPrev = 0, refPkVPrev = 0;						// variables for exponential smoothing


	// set true for netPower display
	lab[netPower].stat = true;

	// main measuring / display loop
	// do at least once
	// continue while power > threshold
	do
	{
		//digitalWrite(TEST_PIN, !digitalRead(TEST_PIN));					// toggle test pin to HALF frequency

		// get ACD results - stop / restart interrupts while copying interrupt data
		noInterrupts();
		fA = fwdAvg;
		rA = refAvg;
		fPk = fwdPk;
		rPk = refPk;
		interrupts();

		// calculate voltages
		fwdV = fA * adcConvert + FV_ZEROADJ;
		refV = rA * adcConvert + RV_ZEROADJ;
		fwdPkV = fPk * adcConvert + FV_ZEROADJ;
		refPkV = rPk * adcConvert + RV_ZEROADJ;

		float weight = (float)optAvgWeight.val/1000;
		//// apply exponential voltages smoothing
		//fwdV = w * fwdV + (1 - w) * fwdVPrev;
		fwdV = sigProcess(fwdV, fwdVPrev, weight);
		fwdVPrev = fwdV;
		//refV = w * refV + (1 - w) * refVPrev;
		refV = sigProcess(refV, refVPrev, weight);
		refVPrev = refV;
		//fwdPkV = w * fwdPkV + (1 - w) * fwdPkVPrev;
		fwdPkV = sigProcess(fwdPkV, fwdPkVPrev, weight);
		fwdPkVPrev = fwdPkV;
		//refPkV = w * refPkV + (1 - w) * refPkVPrev;
		refPkV = sigProcess(refPkV, refPkVPrev, weight);
		refPkVPrev = refPkV;

		//calculate power(watts) directly from voltages
		fwdPwr = pwrCalc(fwdV);
		refPwr = pwrCalc(refV);
		fwdPkPwr = pwrCalc(fwdPkV);
		refPkPwr = pwrCalc(refPkV);

		// net power (watts)
		netPwr = fwdPwr - refPwr;
		if (netPwr < 0)
			netPwr = 0.0;

		// peak power (watts), default pep
		//short touch to select pep or netPwrPeak
		if (lab[peakPower].stat)
		{
			// average net power peak
			if (pkPwr < netPwr)
			{
				pkPwr = netPwr;
				netPwrPkTimer.reset();
			}
			else if (netPwrPkTimer.check())
				pkPwr = netPwr;
		}
		else
		{
			// pep - peak envelope power, cannot be less than netpwr
			pep = fwdPkPwr - refPkPwr;
			if (pep < netPwr)
				pep = netPwr;
			// get hold
			if (pkPwr < pep)
			{
				pkPwr = pep;
				pepTimer.reset();
			}
			else if (pepTimer.check())
				pkPwr = pep;
		}


		// dBm - select by netPower longtouch
		// =  10* log10 (1000 * watts) cannot be less than 0
		dB = 10 * log10(netPwr * 1000);
		if (dB < 0)
			dB = 0.0;

		// swr calculation - only calculate if power on. do not use netPwr as signal processed
		// use power, not volts, as curve linearity compensated
		// peak power preferred - stops SWR changes on power off
		//if ((fwdPwr-refPwr)> PWR_THRESHOLD && fPk > rPk)
		if (netPwr > PWR_THRESHOLD*10 && fwdPkPwr > refPkPwr)
		{
			// reflection coefficient
			float rc = sqrt(refPkPwr / fwdPkPwr);
			if (rc == NAN || isnan(rc))
				swr = 1.0;
			else
			{
				swr = (1 + rc) / (1 - rc);
				if (swr <= 1.0)
					swr = 1.0;
				if (swr > 999.9)
					swr = 999.9;
			}

			// swr display colour based on value
			int swrColour = GREEN;
			if (swr > 1.5)
			{
				// change through yellow to orange to red for swr > 1.5
				int grn = map(swr, 1.5, 3.0, 255, 0);
				grn = constrain(grn, 0, 255);
				swrColour = CL(255, grn, 0);
			}
			val[vswr].colour = swrColour;
			displayValue(vswr, swr);								// display swr while power on
		}

		// display net power, if power on, use RED background
		// reduce display flicker, set lab[netPower].stat = false
		if (netPwr > PWR_THRESHOLD)
		{
			if (fr[netPower].isEnable && lab[netPower].stat)
			{
				fr[netPower].bgColour = RED;
				restoreFrame(netPower);
				// ensure doesn't change to RED next time
				lab[netPower].stat = false;
			}
		}



		float test = (float)avgSamples / 100;
		float decay = (float)avgSamples / 100 * optAvgWeight.val/1000;								// weight for exponential decay
		// display all enabled frames
		decay = .1;

		//netPwr = netPwrProcess(netPwr, decay);
		displayValue(netPower, netPwr);

		displayValue(dBm, dB);
		displayValue(peakPower, pkPwr);
		displayValue(fwdPower, fwdPwr);
		displayValue(refPower, refPwr);
		displayValue(fwdVolts, fwdV);
		displayValue(refVolts, refV);

		//drawMeter(netPwrMeter, netPwr, pkPwr);
		//drawMeter(netPwrMeter, netPwr, pkPwrProcess(pep, .1));
		//drawMeter(netPwrMeter, netPwr, pkPwrProcess(pkPwr, .2));
		drawMeter(netPwrMeter, netPwr, pkPwr);
		drawMeter(swrMeter, swr, 1);

#ifdef CIV
		// get and display frequency, needed here for swr / frequency manual sweep
		// this takes 30 mSecs, so only update if SWR meter and freq display is enabled
		// useful for SWR checking vs Frequency
		if (fr[swrMeter].isEnable && fr[freq].isEnable)
			displayValue(freq, getFreq());
#endif

		// check for dimmed screen and reset
		//if (netPwr >= PWR_THRESHOLD && isDim)
		if (netPwr >= PWR_THRESHOLD)
			resetDimmer();

		// check if screen has been touched
		// netPower button can change samplesAvg during measure
		if (ts.tirqTouched())
			chkTouchFrame(NUM_FRAMES);


		// ensure measure loop slower than getADC() sample frequency
		// measure loop = 30microsecs
		// delay(>5) millisecs
		delay(5);

		// do at least once and while power applied
	} while (netPwr >= PWR_THRESHOLD);

	// power off - display exit power
	// change netPower to b/ground colour
	if (fr[netPower].isEnable && !lab[netPower].stat)
	{
		fr[netPower].bgColour = BG_COLOUR;
		restoreFrame(netPower);
		displayValue(netPower, 0, true);
	}
}



/*----------------------------------- pwrCalc() ----------------------------------------------------------------
calculates pwr in Watts directly from ADC volts
applies constants from calibration procedure
if CIV enabled, adjust for freq response of coupler
*/
float pwrCalc(float v)						//
{
	float pwr = 0.0;

	// calculate power
	{
		if (v < FWD_V_SPLIT_PWR)							// low power below split (non-linear)
			pwr = log(v) * FWD_LO_MULT_PWR + FWD_LO_ADD_PWR;
		else
			pwr = v * v * FWD_HI_MULT2_PWR + v * FWD_HI_MULT1_PWR + FWD_HI_ADD_PWR;		// high power
	}

	if (pwr < 0 || !isnormal(pwr) || isnan(pwr))			// check for division by zero and < 0
		pwr = 0.0;

#ifdef CIV	// adjust power for freq response of coupler
	if (isCivEnable)
		return (pwr * hfBand[currBand].pwrMult);			// return power (watts)
	else
#endif

		return pwr;
}