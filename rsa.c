/*
TODO rsa encryption/decryption demo
TODO rip java biginteger prime generation
*/
#include <stdio.h>
#include <gmp.h>

#define KP_TRIES 100

int gen_keypair(gmp_randstate_t *prng, mpz_t *p, mpz_t *q)
{
	int err = 1;
	unsigned i;
	mpz_t a, b, g, e, e2, e0, i0, i1, phi, phi2, rem, x, y;
	mpz_inits(a, b, g, e, e2, e0, i0, i1, phi, phi2, rem, x, y, NULL);

	mpz_sub_ui(i0, *p, 1);
	mpz_sub_ui(i1, *q, 1);

	// compute totient of n: phi = (p-1) * (q-1)
	mpz_mul(phi, i0, i1);

	for (i = 0; mpz_cmp_ui(g, 1) && i < KP_TRIES; ++i) {
		// choose integer e such that e and phi(n) are coprime
		mpz_urandomm(e0, *prng, phi);
		mpz_add_ui(e, e0, 1);

		mpz_set(e2, e);
		mpz_set(phi2, phi);

		// g = gcd(e, phi)
		while (mpz_cmp_ui(phi2, 0)) {
			mpz_mod(rem, e2, phi2);
			mpz_set(e2, phi2);
			mpz_set(phi2, rem);
		}
		mpz_set(g, e2);
	}
	if (i == KP_TRIES)
		goto fail;

	// TODO compute multiplicative_inverse
	// alias d to g
	// alias x1 to a
	// alias x2 to b
	// alias y1 to e0
	// alias rem to temp_phi
	// alias i0 to temp1
	// alias i1 to temp2
	// alias e2 to e
	// alias phi2 to phi
	mpz_set(e2, e);
	mpz_set(phi2, phi);

	mpz_set_ui(g, 0);
	mpz_set_ui(a, 0);
	mpz_set_ui(b, 1);
	mpz_set_ui(e0, 1);
	mpz_set(rem, phi2);

	while (mpz_cmp_ui(e2, 0) > 0) {
		// temp1 = temp_phi / e
		mpz_fdiv_q(i0, rem, e2);
		// x = temp1 * e
		mpz_mul(x, i0, e2);
		mpz_sub(i1, rem, x);
		mpz_set(rem, e2);
		mpz_set(e2, i1);

		// thrash x to compute y first
		mpz_mul(x, i0, e0);
		mpz_sub(y, g, x);
		// we can recycle g to compute the rest
		mpz_mul(g, i0, a);
		mpz_sub(x, b, g);

		mpz_set(b, a);
		mpz_set(a, x);
		mpz_set(g, e0);
		mpz_set(e0, y);
	}
	if (mpz_cmp_ui(rem, 1)) {
		fputs("bad temp_phi:", stderr);
		mpz_out_str(stderr, 10, rem);
		fputc('\n', stderr);
		goto fail;
	}

	// alias x to d
	mpz_add(x, g, phi2);
	mpz_mul(a, *p, *q);

	fputs("public key: (", stdout);
	mpz_out_str(stdout, 10, e);
	fputs(", ", stdout);
	mpz_out_str(stdout, 10, a);
	fputs(")\n", stdout);

	mpz_out_str(stdout, 10, x);

	err = 0;
fail:
	mpz_clears(y, x, rem, phi2, phi, i1, i0, e0, e2, e, g, b, a, NULL);

	if (err)
		fputs("fail\n", stderr);

	return err;
}

int main(void)
{
	gmp_randstate_t prng;
	gmp_randinit_mt(prng);

	mpz_t p, q;
	mpz_inits(p, q, NULL);

	mpz_set_ui(p, 23);
	mpz_set_ui(q, 19);

	gen_keypair(&prng, &p, &q);

	mpz_clears(p, q, NULL);
	gmp_randclear(prng);
	return 0;
}
