#include <stdio.h>
#include <math.h>

#define INDEX_A4 49
#define FREQ_A4 440.0
#define NOTES_PER_OCTAVE 12

double freq(int note)
{
	return pow(2, (note - INDEX_A4) / (double)NOTES_PER_OCTAVE) * FREQ_A4;
}

double tone(double freq)
{
	return (double)NOTES_PER_OCTAVE * (log(freq / FREQ_A4) / log(2.0)) + INDEX_A4;
}

unsigned utone(double freq)
{
	return (unsigned)tone(freq);
}

int main(void)
{
	for (int i = INDEX_A4; i <= INDEX_A4 + 2 * NOTES_PER_OCTAVE; ++i) {
		printf("%2d: %g\n", i, freq(i));
	}

	for (double f = 880; f >= FREQ_A4; f -= 40) {
		printf("%g: %g\n", f, tone(f));
	}

	return 0;
}
