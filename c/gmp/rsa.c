// Licensed under Lesser General Public License version 3
/*
Small demo with rsa encryption/decryption
Copyright Folkert van Verseveld

Note that the example is just for illustration purposes and not suitable for
real use, because the encrypted message is stored per byte, which means that
even though the stored numbers are really large, there are only 256 distinct
ones which greatly reduces security.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#define GK_TRIES 5
#define KP_TRIES 100

static const unsigned char atohex[256] = {
	['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4, ['5'] = 5, ['6'] = 6, ['7'] = 7,
	['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,
	['8'] = 8, ['9'] = 9, ['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15,
};

int gen_keypair(mpz_t *pub, mpz_t *priv, mpz_t *k, gmp_randstate_t *prng, const mpz_t p, const mpz_t q)
{
	int err = 1;
	unsigned i, runs = GK_TRIES;
	mpz_t a, b, g, e, e2, e0, i0, i1, phi, phi2, rem, x, y;
	mpz_inits(a, b, g, e, e2, e0, i0, i1, phi, phi2, rem, x, y, NULL);

again:
	mpz_sub_ui(i0, p, 1);
	mpz_sub_ui(i1, q, 1);

	if (!runs--)
		goto fail;

	// compute totient of n: phi = (p-1) * (q-1)
	mpz_mul(phi, i0, i1);

	// try to find random e in range [1, phi) such that e and phi are coprime
	for (i = 0; mpz_cmp_ui(g, 1) && i < KP_TRIES; ++i) {
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

	// compute multiplicative_inverse
	mpz_set(e2, e);
	mpz_set(phi2, phi);

	mpz_set_ui(g, 0);
	mpz_set_ui(a, 0);
	mpz_set_ui(b, 1);
	mpz_set_ui(e0, 1);
	mpz_set(rem, phi2);

	while (mpz_cmp_ui(e2, 0) > 0) {
		// temp1 = temp_phi / e
		mpz_tdiv_q(i0, rem, e2);
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
		gmp_fprintf(stderr, "bad temp_phi: %Zd\n", rem);
		goto fail;
	}

	// store result
	mpz_add(*priv, g, phi2);
	mpz_mul(*k, p, q);
	mpz_set(*pub, e);

	// ensure the private and public key are distinct
	if (mpz_cmp(*priv, *pub) == 0)
		goto again;

	err = 0;
fail:
	mpz_clears(y, x, rem, phi2, phi, i1, i0, e0, e2, e, g, b, a, NULL);

	if (err)
		fputs("fail\n", stderr);

	return err;
}

// XXX use int (malloc may fail)
// FIXME compute number of bytes that can fit in modulo
void rsa_encrypt(char *dst, const void *src, size_t size, const mpz_t pub, const mpz_t m)
{
	size_t elemsz;
	mpz_t base, elem;
	const unsigned char *data;
	char *basebuf;
	mpz_init(base);
	mpz_init(elem);

	size_t items = 0;

	mpz_set(base, m);
	mpz_set(elem, m);

	while (mpz_cmp_ui(base, 256) > 0) {
		++items;
		mpz_tdiv_q_ui(elem, base, 256);
		if (mpz_cmp_ui(elem, 256) <= 0)
			break;
		++items;
		mpz_tdiv_q_ui(base, elem, 256);
	}

	printf("items: %zu\n", items);

	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	basebuf = malloc(elemsz + 1);
	data = src;

	for (size_t i = 0; i < size; ++i) {
		mpz_set_ui(base, data[i]);
		mpz_powm(elem, base, pub, m);

		memset(basebuf, 0, elemsz);
		mpz_get_str(basebuf, 16, elem);

		// compression step
		// determine length
		size_t nhex = strlen(basebuf);
		size_t pad = elemsz - nhex;

		// dst <- elem
		while (pad --> 0)
			*dst++ = '0';
		for (const char *dig = basebuf; *dig; ++dig)
			*dst++ = *dig;
	}

	free(basebuf);
	mpz_clear(elem);
	mpz_clear(base);
}

/* Compute number of elements and item size for encrypting and decrypting data. */
void rsa_bufstat(size_t size, const mpz_t m, size_t *count, size_t *elemsz)
{
	size_t items = 0;
	mpz_t base, elem;

	mpz_init(base);
	mpz_init(elem);

	*elemsz = mpz_sizeinbase(m, 256);

	mpz_set(base, m);
	mpz_set(elem, m);

	// count bytes pack count for m
	while (mpz_cmp_ui(base, 256) > 0) {
		++items;
		mpz_tdiv_q_ui(elem, base, 256);
		if (mpz_cmp_ui(elem, 256) <= 0)
			break;
		++items;
		mpz_tdiv_q_ui(base, elem, 256);
	}

	*count = (size - 1) / items + 1;

	mpz_clear(elem);
	mpz_clear(base);
}

int rsa_encrypt2(char *dst, const void *src, size_t size, const mpz_t pub, const mpz_t m)
{
	size_t items, elemsz, bufsz;
	mpz_t sum, base, elem, k, tmp;
	char *basebuf;
	const unsigned char *data;

	rsa_bufstat(size, m, &items, &elemsz);
	printf("count: %zu, items: %zu, elemsz: %zu\n", size, items, elemsz);
	// +1 may not be necessary, but better safe than sorry
	bufsz = 2 * elemsz + 1;

	if (!(basebuf = malloc(bufsz)))
		return 1;

	mpz_inits(sum, base, elem, k, tmp, NULL);

	data = src;

	for (size_t i = 0; i < size; i += elemsz) {
		mpz_set_ui(k, 1);
		mpz_set_ui(sum, 0);

		size_t end = i + elemsz;
		if (end > size)
			end = size;

		for (size_t j = i; j < end; ++j) {
			// compute item sum
			mpz_set_ui(base, 0);
			mpz_addmul_ui(base, k, data[j]);
			mpz_add(elem, sum, base);
			mpz_set(sum, elem);

			// update multiplier for next byte
			mpz_swap(tmp, k);
			mpz_mul_ui(k, tmp, 256);
		}
		gmp_printf("sum : %*ZX\n", 2 * elemsz, sum);
		mpz_powm(elem, sum, pub, m);
		gmp_printf("elem: %*ZX\n", 2 * elemsz, elem);

		memset(basebuf, 0, bufsz);
		mpz_get_str(basebuf, 16, elem);

		basebuf[bufsz - 1] = '\0';

		// compression step
		// determine length
		size_t nhex = strlen(basebuf);
		size_t pad = 2 * elemsz - nhex;

		printf("pad: %zu\n", pad);

		// pack basebuf and store in dst
		const char *from = dst;

		// FIXME padding
		if (pad) {
			size_t shift, k = 0, j;

			for (shift = pad >> 1; shift; --shift)
				*dst++ = 0;

			j = i + (pad >> 1);

			if (pad & 1) {
				#if 0
				*dst++ = atohex[basebuf[0] & 0xf];

				for (size_t j = i, k = 1; j < i + elemsz; ++j, k += 2)
					*dst++ = atohex[basebuf[k]] << 4 | atohex[basebuf[k + 1]];
				#else
				*dst = 0;

				// odd padding is tricker, we may overrun basebuf.
				for (; j < i + elemsz - 1; ++j, k += 2) {
					*dst++ |= atohex[(unsigned char)basebuf[k]];
					*dst = atohex[(unsigned char)basebuf[k + 1]] << 4;
				}
				*dst++ |= atohex[(unsigned char)basebuf[k] & 0xff];
				if (k + 2 < bufsz)
					*dst = atohex[(unsigned char)basebuf[k + 1]] << 4;
				#endif
			} else {
				for (; j < i + elemsz; ++j, k += 2)
					*dst++ = atohex[(unsigned char)basebuf[k]] << 4 | atohex[(unsigned char)basebuf[k + 1]];
			}
		} else {
			for (size_t j = i + (pad >> 1), k = 0; j < i + elemsz; ++j, k += 2)
				*dst++ = atohex[(unsigned char)basebuf[k]] << 4 | atohex[(unsigned char)basebuf[k + 1]];
		}

		// dump to test if it worked
		printf("elemsz: %zu\n      ", elemsz);
		for (size_t k = 0; k < elemsz; ++k)
			printf("%02X", from[k] & 0xff);
		putchar('\n');
	}

	mpz_clears(tmp, k, elem, base, sum, NULL);
	free(basebuf);

	return 0;
}

// XXX use int (malloc may fail)
// FIXME compute number of bytes that can fit in modulo
void rsa_decrypt(char *dst, const char *blk, size_t size, const mpz_t priv, const mpz_t m)
{
	size_t elemsz;
	mpz_t base, ch;
	char *basebuf;
	mpz_init(base);
	mpz_init(ch);

	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	basebuf = malloc(elemsz + 1);

	for (size_t i = 0; i < size; i += elemsz) {
		strncpy(basebuf, &blk[i], elemsz);
		basebuf[elemsz] = '\0';

		mpz_set_str(base, basebuf, 16);

		mpz_powm(ch, base, priv, m);

		*dst++ = mpz_get_ui(ch);
	}

	free(basebuf);
	mpz_clear(ch);
	mpz_clear(base);
}

int test_rsa(const char *msg, const mpz_t pub, const mpz_t priv, const mpz_t m)
{
	int err = 1;
	size_t msglen, elemsz, bufsz, items;
	char *basebuf = NULL, *buf = NULL, *orig = NULL;
	mpz_t base, elem;
	mpz_init(base);
	mpz_init(elem);

	msglen = strlen(msg);
	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	bufsz = (msglen + 1) * elemsz;

	if (!(buf = malloc(bufsz)) || !(orig = malloc(msglen + 1)))
		goto fail;

	printf("blk len: %zu (orig: %zu, elem: %zu)\n", bufsz, msglen, elemsz);

	rsa_encrypt(buf, msg, msglen + 1, pub, m);
	rsa_decrypt(orig, buf, bufsz, priv, m);
	printf("decrypted: %s\n", orig);

	puts(strcmp(msg, orig) ? "fail" : "ok");

	free(orig);
	rsa_bufstat(msglen + 1, m, &items, &elemsz);
	bufsz = items * elemsz + 1;
	printf("bufsz: %zu\n", bufsz);
	if (!(orig = malloc(bufsz)))
		goto fail;

	rsa_encrypt2(orig, msg, msglen + 1, pub, m);

	err = 0;
fail:
	free(orig);
	free(basebuf);
	free(buf);
	mpz_clear(elem);
	mpz_clear(base);
	return err;
}

int main(void)
{
	int err;
	gmp_randstate_t prng;
	unsigned bits, reps;
	mpz_t p, q, pub, priv, m;

	mpz_inits(p, q, pub, priv, m, NULL);
	gmp_randinit_mt(prng);
	srand(time(NULL));

	bits = 256 + (unsigned)(16.0f * rand() / RAND_MAX);
	reps = 40 + (unsigned)(10.0f * rand() / RAND_MAX);

	while (1) {
		mpz_urandomb(p, prng, bits);
		if (!mpz_probab_prime_p(p, reps))
			continue;
		mpz_urandomb(q, prng, bits);
		if (!mpz_probab_prime_p(q, reps))
			continue;
		break;
	}

	gmp_printf("p: %ZX\nq: %ZX\n", p, q);

	err = gen_keypair(&pub, &priv, &m, &prng, p, q);
	if (err)
		goto fail;
	// dump keys
	gmp_printf("public key: (%ZX, %ZX)\n", pub, m);
	gmp_printf("private key: (%ZX, %ZX)\n", priv, m);

	err = test_rsa("whoah", pub, priv, m);
	if (err)
		goto fail;
	err = test_rsa("Mah BOI, this is what all true warriors strive FOR! WhoaHaHahaHaHAHA", pub, priv, m);
	if (err)
		goto fail;
	err = test_rsa("Je kan lekker niet verslaan!", pub, priv, m);
	if (err)
		goto fail;

	err = 0;
fail:
	mpz_clears(m, priv, pub, q, p, NULL);
	gmp_randclear(prng);
	return err;
}
