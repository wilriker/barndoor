#ifndef MP6500_H
#define MP6500_H

#include <stdlib.h>
#include <Arduino.h>

class MP6500 {
	public:
		typedef enum {
			FULL_STEP = 1,
			HALF_STEP = 2,
			QUARTER_STEP = 4,
			EIGHTH_STEP = 8
		} Microsteps;
		typedef enum {
			POINT_FIVE,
			ONE,
			ONE_POINT_FIVE,
			TWO
		} CoarseCurrentLimit;
		MP6500(uint8_t pinDir, uint8_t pinStep, uint8_t pinSleep, uint8_t pinI2, uint8_t pinI1, uint8_t pinMS2, uint8_t pinMS1, uint8_t pinEnable = 0xff);
		void init(MP6500::Microsteps steps = EIGHTH_STEP, float current = -1.0);
		void sleep();
		void wakeup();
		void setStepping(MP6500::Microsteps steps);
		void setCurrentLimit(float limit);
		void setCoarseCurrentLimit(MP6500::CoarseCurrentLimit limit);
	private:
		uint8_t _pinDir;
		uint8_t _pinStep;
		uint8_t _pinSleep;
		uint8_t _pinI1;
		uint8_t _pinI2;
		uint8_t _pinMS1;
		uint8_t _pinMS2;
		uint8_t _pinEnable;
		bool sleeping;
		Microsteps currentStepping;

		void setIPin(uint8_t iPin, bool highImpedance);
};
#endif
