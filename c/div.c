#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

struct mydiv_t {
	uint16_t quot;
	uint16_t rem;
};

struct mydiv_t mydiv(uint16_t numerator, uint16_t denominator)
{
	struct mydiv_t d;
	uint16_t quotient;
	uint32_t divisor, remainder, parity_mask;

	quotient = 0;
	divisor = denominator << (sizeof(denominator) * 8);
	remainder = numerator;
	parity_mask = ~(((uint32_t)-1) >> 1);

	//printf("%4" PRIX16 ", %8" PRIX32 ", %8" PRIX32 ", %8" PRIX32 "\n", quotient, divisor, remainder, parity_mask);

	for (unsigned i = 0; i <= sizeof(numerator) * 8; ++i) {
		uint32_t remainder_next = remainder - divisor;

		//printf("next: %8" PRIX32 "\n", remainder_next);

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
	printf("%" PRIu16 " rest %" PRIu16 "\n", d->quot, d->rem);
}

int main(void)
{
	/* NOTE: this algorithm does not work for signed division! And either
	 * the numerator or denominator may not have their most significant bit
	 * set (I don't know the specifics for sure, but it does not work
	 * otherwise.
	 */

	/*
	 * This test takes about half a minute to run on an Intel Core
	 * i7-7700HQ. It tests all possible values in the range [0, 32767] and
	 * [1, 32767] for the numerator and denominator respectively.
	 */
	for (unsigned numerator = 0; numerator <= INT16_MAX; ++numerator) {
		for (unsigned denominator = 1; denominator <= INT16_MAX; ++denominator) {
			struct mydiv_t d = mydiv(numerator, denominator);
			div_t d2 = div(numerator, denominator);

			if (d.quot != d2.quot || d.rem != d2.rem) {
				fprintf(
					stderr,
					"Div fails for divisor = %u, dividend = %u: got: %" PRIu16 ", %" PRIu16 " but expected: %u, %u\n",
					numerator, denominator, d.quot, d.rem, d2.quot, d2.rem
				);
				//return 1;
			}
		}
	}

	return 0;
}
