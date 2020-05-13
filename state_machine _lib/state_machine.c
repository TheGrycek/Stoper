		#include "gpio.h"
		#include "state_machine.h"
		#include "main.h"
		enum states{IDLE, COUNT, PAUSE};
		
		void state_machine(char *tab, uint32_t n, uint8_t start,char state)
		{
		switch (state)
		{
			case IDLE:
			{
				start = 0;
				n = 0;
				tab[0] = 0;
				tab[1] = 0;
				tab[2] = 0;
				tab[3] = 0;		
					break;
			}
			case COUNT:
			{
				start = 1;
					break;
			}
				case PAUSE:
			{
				start = 0;
					break;
			}	
		}
	}
		