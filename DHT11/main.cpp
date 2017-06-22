#include "bcm2835.h"
#include <iostream>
#include <locale.h>

using namespace std;

int data[5] = { 0, 0, 0, 0, 0 };

void dht11_read(int pin)
{
	uint8_t laststate = HIGH;
	uint8_t j = 0;

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(pin, LOW);
	delay(18);

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

	// Max timing: 85
	for (uint8_t i = 0; i < 85; i++)
	{
		uint8_t counter = 0;
		while (bcm2835_gpio_lev(pin) == laststate)
		{
			counter++;
			delayMicroseconds(1);

			if (counter == 255)
				break;
		}
		laststate = bcm2835_gpio_lev(pin);

		if (counter == 255)
			break;

		if (i >= 4 && i % 2 == 0)
		{
			data[j / 8] <<= 1;

			if (counter > 16)
				data[j / 8] |= 1;
			
			j++;
		}
	}

	if (j >= 40 && data[4] == (data[0] + data[1] + data[2] + data[3] & 0xFF))
	{
		auto h = static_cast<float>((data[0] << 8) + data[1]) / 10;
		
		if (h > 100)
			h = data[0];
		
		auto c = static_cast<float>(((data[2] & 0x7F) << 8) + data[3]) / 10;
		
		if (c > 125)
			c = data[2];
		
		if (data[2] & 0x80)
			c = -c;

		printf("Nem: %.1f%% - Sıcaklık: %.1f *C\n", h, c);
	}
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "Turkish");

	if (!bcm2835_init())
		exit(1);

	int pin = argc > 1 ? atoi(argv[1]) : 22;

	cout << pin << ". Pin Üzerinden Sıcaklık Ve Nem Okunuyor" << endl;

	while (true)
	{
		dht11_read(pin);
		delay(2000);
	}
}