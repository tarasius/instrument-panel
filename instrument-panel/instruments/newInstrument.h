#ifndef _NEW_INSTRUMENT_H_
#define _NEW_INSTRUMENT_H_

#include "instrument.h"

class newInstrument : public instrument
{
private:
	float scaleFactor;

	// FlightSim vars (external variables that influence this instrument)
	long instrumentVar;

	// Instrument values (caclulated from variables and needed to draw the instrument)
	float instrumentValue;

public:
	newInstrument(int xPos, int yPos, int size);
	void render();
	void update();

private:
	void resize();
	void addVars();
	bool fetchVars();
};

#endif // _NEW_INSTRUMENT_H