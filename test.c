/*
gmp demo that computes pow(2, 20000)
compile and run with: cc -o test test -lgmp && ./test
*/
#include <stdio.h>
#include <gmp.h>

int main(void)
{
	mpz_t result, base;
	mpz_inits(result, base, NULL);
	// base = "2" (using base 10)
	mpz_set_str(base, "2", 10);
	// result = pow(base, 20000)
	mpz_pow_ui(result, base, 20000);
	// print result using base10 to stdout
	mpz_out_str(stdout, 10, result);

	mpz_clears(result, base, NULL);
	return 0;
}
