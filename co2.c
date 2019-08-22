#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

float level_from_pwm(int length_0, int length_1) {
  float co2_max = 5000;

  if (length_0 > 0 && length_1 > 0 && length_0 + length_1 > 900*1000 && length_0 + length_1 < 1100*1000) {
	  float level = (float)(length_1 - 2000)/(float)(length_0 + length_1 - 4000) * co2_max;

	  if (level > 0 && level < co2_max) {
		  return level;
	  }
  }

  return 0;
}

int main(int argc, char** argv) {

  if (wiringPiSetup() == -1) {
    exit(1);
  }

  int delay = 500;
  int max_samples = 10;
  int gpio = 5;

  int debug = 0;

  int opt;

  // put ':' in the starting of the
  // string so that program can
  //distinguish between '?' and ':'
  while((opt = getopt(argc, argv, "d:n:g:v")) != -1) {
	  switch(opt) {
		  case 'v':
			  debug = 1;
			  break;
		  case 'd':
			  delay = atoi(optarg);
			  break;
		  case 'n':
			  max_samples = atoi(optarg);
			  break;
		  case 'g':
			  gpio = atoi(optarg);
			  break;
		  case '?':
			 printf("unknown option: %c\n", optopt);
			 return 1;
	  }
  }

  pinMode(gpio, INPUT);

  int status = -1;
  int newstatus = 0;
  
  int last_change = 0;
  int length_0 = 0;
  int length_1 = 0;

  int discarding = 1;

  int samples = 0;
  float sum = 0;

  while (samples < max_samples) {
	newstatus = digitalRead(gpio);
	int now = micros();
	if (newstatus != status) {
		if (debug) printf("\n");

		if (last_change > 0) {
			if (status == 0 && newstatus == 1) {
				length_0 = now - last_change;
				discarding = 0;
			} else if (status == 1 && newstatus == 0) {
				length_1 = now - last_change;
			}

			status = newstatus;

			float level = level_from_pwm(length_0, length_1);
			if (level > 0 && !discarding) {
				samples++;
				sum += level;

				if (debug) printf("\n~%f ppm\n\n", level);

				length_0 = 0;
				length_1 = 0;
			}
		}

		last_change = now;
		if (debug) {
			if (discarding) printf("[%i]", newstatus);
			else printf("%i", newstatus);
		}
	} else if (now - last_change > 1000 * 1000) {
		return 1;
	} else {
		if (debug) {
			if (discarding) printf("(%i)", newstatus);
			else printf("%i", newstatus);
		}
	}

	delayMicroseconds(delay);
  }

  if (debug) printf("\n\nsum: %f, samples: %i\n", sum, samples);
  printf("%f ppm\n", sum/samples);

  return 0;
}
