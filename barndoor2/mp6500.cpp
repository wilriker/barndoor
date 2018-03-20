#include "mp6500.h"

MP6500::MP6500(uint8_t pinDir, uint8_t pinStep, uint8_t pinSleep, uint8_t pinI2, uint8_t pinI1, uint8_t pinMS2, uint8_t pinMS1, uint8_t pinEnable = 0xff) {
	_pinDir = pinDir;
	_pinStep = pinStep;
	_pinSleep = pinSleep;
	_pinI1 = pinI1;
	_pinI2 = pinI2;
	_pinMS1 = pinMS1;
	_pinMS2 = pinMS2;
	_pinEnable = pinEnable;
}

void MP6500::init(MP6500::Microsteps steps, float current) {
	if (_pinEnable != 0xff) {
		pinMode(_pinEnable, OUTPUT);
	}
	pinMode(_pinMS1, OUTPUT);
	pinMode(_pinMS2, OUTPUT);
	pinMode(_pinSleep, OUTPUT);
	pinMode(_pinStep, OUTPUT);
	pinMode(_pinDir, OUTPUT);
	sleeping = true;

	setStepping(steps);
	if (current == -1.0) {
		setCoarseCurrentLimit(POINT_FIVE);
	} else {
		setCurrentLimit(current);
	}
}

void MP6500::setStepping(MP6500::Microsteps steps) {
	uint8_t ms1 = LOW;
	uint8_t ms2 = LOW;
	switch(steps) {
		case HALF_STEP:
			ms1 = HIGH;
			break;
		case QUARTER_STEP:
			ms2 = HIGH;
			break;
		case EIGHTH_STEP:
			ms1 = HIGH;
			ms2 = HIGH;
			break;
	}
	digitalWrite(_pinMS1, ms1);
	digitalWrite(_pinMS2, ms2);
	delay(5);
	currentStepping = steps;
}

void MP6500::sleep() {
	if (!sleeping) {
		digitalWrite(_pinSleep, LOW);
		sleeping = true;
	}
}

void MP6500::wakeup() {
	if (sleeping) {
		digitalWrite(_pinSleep, HIGH);
		// Driver requires approximately 1ms to wake-up - be safe and wait
		delay(5);
		sleeping = false;
	}
}

void MP6500::setCurrentLimit(float limit) {
	pinMode(_pinI1, OUTPUT);
	setIPin(_pinI2, false);
	float pwmVoltage = (2.2 - limit) / 0.63; // Voltage on PWM out pin (check driver's manual to get the correct value)
	int pwmValue = ceil((255.0 / 5.0) * pwmVoltage); // Ceil'd PWM value
	analogWrite(_pinI1, pwmValue);
}

void MP6500::setCoarseCurrentLimit(MP6500::CoarseCurrentLimit limit) {
	switch (limit) {
		case POINT_FIVE:
			setIPin(_pinI1, true);
			setIPin(_pinI2, true);
			break;
		case ONE:
			setIPin(_pinI1, false);
			setIPin(_pinI2, true);
			break;
		case ONE_POINT_FIVE:
			setIPin(_pinI1, true);
			setIPin(_pinI2, false);
			break;
		case TWO:
			setIPin(_pinI1, false);
			setIPin(_pinI2, false);
			break;
	}
}

void MP6500::setIPin(uint8_t iPin, bool highImpedance) {
	digitalWrite(iPin, LOW);
	if (highImpedance) {
		pinMode(iPin, INPUT);
	} else {
		pinMode(iPin, OUTPUT);
	}
}
