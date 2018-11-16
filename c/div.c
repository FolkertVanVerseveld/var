#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

struct mydiv_t {
	uint8_t quot;
	uint8_t rem;
};

struct mydiv_t mydiv(uint8_t numerator, uint8_t denominator)
{
	struct mydiv_t d;
	uint8_t quotient;
	uint16_t divisor, remainder, parity_mask;

	quotient = 0;
	divisor = denominator << (sizeof(denominator) * 8);
	remainder = numerator;
	parity_mask = ~(((uint16_t)-1) >> 1);

	//printf("%2" PRIX8 ", %4" PRIX16 ", %4" PRIX16 ", %4" PRIX16 "\n", quotient, divisor, remainder, parity_mask);

	for (unsigned i = 0; i <= sizeof(numerator) * 8; ++i) {
		uint16_t remainder_next = remainder - divisor;

		//printf("next: %4" PRIX16 "\n", remainder_next);

		// Restore if negative.
		if (remainder_next & parity_mask) {
			//puts("restore");
			remainder_next = remainder;
			quotient <<= 1;
		} else {
			//puts("sll");
			quotient = (quotient << 1) | 1;
		}

		divisor >>= 1;
		remainder = remainder_next;
	}

	d.quot = quotient;
	d.rem = remainder;

	return d;
}

void mydiv_dump(struct mydiv_t *d)
{
	printf("%" PRIu8 " rest %" PRIu8 "\n", d->quot, d->rem);
}

int main(void)
{
	for (unsigned numerator = 0; numerator <= INT8_MAX; ++numerator) {
		for (unsigned denominator = 0; denominator <= INT8_MAX; ++denominator) {
			if (denominator == 0)
				continue;

			struct mydiv_t d = mydiv(numerator, denominator);
			div_t d2 = div(numerator, denominator);

			if (d.quot != d2.quot || d.rem != d2.rem) {
				fprintf(
					stderr,
					"Div fails for divisor = %u, dividend = %u: got: %" PRIu8 ", %" PRIu8 " but expected: %u, %u\n",
					numerator, denominator, d.quot, d.rem, d2.quot, d2.rem
				);
				//return 1;
			}
		}
	}

	return 0;
}
