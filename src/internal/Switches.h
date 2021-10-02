#ifndef __DCSBIOS_SWITCHES_H
#define __DCSBIOS_SWITCHES_H

#include <math.h>
#include "Arduino.h"

namespace DcsBios {
	template <unsigned long pollIntervalMs = POLL_EVERY_TIME>
	class Switch2PosT : PollingInput, public ResettableInput
	{
	private:
		const char* msg_;
		char pin_;
		char debounceSteadyState_;
		char lastState_;
		bool reverse_;
		unsigned long debounceDelay_;
		unsigned long lastDebounceTime = 0;

		void resetState()
		{
			lastState_ = (lastState_==0)?-1:0;
		}

		void pollInput() {
			char state = digitalRead(pin_);
			if (reverse_) state = !state;

			unsigned long now = millis();

			if (state != debounceSteadyState_) {
				lastDebounceTime = now;
				debounceSteadyState_ = state;
			}
			
			if ((now - lastDebounceTime) >= debounceDelay_) {
				if (debounceSteadyState_ != lastState_) {
					if (tryToSendDcsBiosMessage(msg_, state == HIGH ? "0" : "1")) {
						lastState_ = debounceSteadyState_;
					}
				}
			}			
		}
	public:
		Switch2PosT(const char* msg, char pin, bool reverse = false, unsigned long debounceDelay = 50) :
			PollingInput(pollIntervalMs)
		{ 
			msg_ = msg;
			pin_ = pin;
			pinMode(pin_, INPUT_PULLUP);
			debounceDelay_ = debounceDelay;
			reverse_ = reverse;

			lastState_ = digitalRead(pin_);
			if (reverse_) lastState_ = !lastState_;			
		}
				
		void SetControl( const char* msg )
		{
			msg_ = msg;
		}
        
		void resetThisState()
		{
			this->resetState();
		}
	};
	typedef Switch2PosT<> Switch2Pos;
	
	template <unsigned long pollIntervalMs = POLL_EVERY_TIME>
	class Switch3PosT : PollingInput, public ResettableInput
	{
	private:
		const char* msg_;
		char pinA_;
		char pinB_;
		char lastState_;
		char debounceSteadyState_;
		unsigned long debounceDelay_;
		unsigned long lastDebounceTime = 0;

		char readState() {
			if (digitalRead(pinA_) == LOW) return 0;
			if (digitalRead(pinB_) == LOW) return 2;
			return 1;
		}
		void resetState()
		{
			lastState_ = (lastState_==0)?-1:0;
		}
		void pollInput() {
			char state = readState();
			unsigned long now = millis();
			if (state != debounceSteadyState_) {
				lastDebounceTime = now;
				debounceSteadyState_ = state;
			}

			if ((now - lastDebounceTime) >= debounceDelay_)
			{
				if (state != lastState_) {
					if (state == 0)
					{
						if (tryToSendDcsBiosMessage(msg_, "0"))
							lastState_ = state;
					}
					else if (state == 1)
					{
						if (tryToSendDcsBiosMessage(msg_, "1"))
							lastState_ = state;
					}
					else if (state == 2)
					{
						if(tryToSendDcsBiosMessage(msg_, "2"))
							lastState_ = state;
					}
				}
			}
		}
	public:
		Switch3PosT(const char* msg, char pinA, char pinB, unsigned long debounceDelay = 50) :
			PollingInput(pollIntervalMs)
		{
			msg_ = msg;
			pinA_ = pinA;
			pinB_ = pinB;
			pinMode(pinA_, INPUT_PULLUP);
			pinMode(pinB_, INPUT_PULLUP);
			lastState_ = readState();
			debounceSteadyState_ = lastState_;
			debounceDelay_ = debounceDelay;
		}
		
		void SetControl( const char* msg )
		{
			msg_ = msg;
		}

		void resetThisState()
		{
			this->resetState();
		}
	};
	typedef Switch3PosT<> Switch3Pos;

	template <unsigned long pollIntervalMs = POLL_EVERY_TIME>
	class SwitchMultiPosT : PollingInput, public ResettableInput
	{
	private:
		const char* msg_;
		const byte* pins_;
		char numberOfPins_;
		char lastState_;
		bool reverse_;
		char readState() {
			unsigned char ncPinIdx = lastState_;
			for (unsigned char i=0; i<numberOfPins_; i++) {
				if( pins_[i] == PIN_NC)
					ncPinIdx = i;
				else
				{
					if (digitalRead(pins_[i]) == LOW && reverse_ == false) return i;
					else if (digitalRead(pins_[i]) == HIGH && reverse_ == true) return i;
				}
			}
			return ncPinIdx;
		}
		void resetState()
		{
			lastState_ = (lastState_==0)?-1:0;
		}
		void pollInput() {
			char state = readState();
			if (state != lastState_)
			{
				char buf[7];
				utoa(state, buf, 10);
				if (tryToSendDcsBiosMessage(msg_, buf))
					lastState_ = state;
			}
		}
	public:
		SwitchMultiPosT(const char* msg, const byte* pins, char numberOfPins, bool reverse = false) :
			PollingInput(pollIntervalMs),
			lastState_(0)
		{
			msg_ = msg;
			pins_ = pins;
			reverse_ = reverse;
			numberOfPins_ = numberOfPins;
			unsigned char i;
			for (i=0; i<numberOfPins; i++) {
				if( pins[i] != PIN_NC)
					pinMode(pins[i], INPUT_PULLUP);
			}
			lastState_ = readState();
		}
		
		void SetControl( const char* msg )
		{
			msg_ = msg;
		}
        
		void resetThisState()
		{
			this->resetState();
		}
	};
	typedef SwitchMultiPosT<> SwitchMultiPos;
}

#endif	