// Licensed under Lesser General Public License version 3
/*
Small demo with rsa encryption/decryption
Copyright Folkert van Verseveld

Note that the rsa_encrypt and rsa_decrypt are just for illustration purposes and
are not suitable for real use, because the encrypted message is stored per byte,
which means that even though the stored numbers are really large, there are only
256 distinct ones which greatly reduces security.

See test_rsa2 for a better, more efficient, more secure demonstration.
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

const char *str_hex_lo = "0123456789abcdef";

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

// FIXME compute number of bytes that can fit in modulo
/**
 * Convert \a size bytes from \a src to RSA and store in \a dst.
 * \a pub and \a m are obtained from gen_keypair.
 */
int rsa_encrypt(char *dst, const void *src, size_t size, const mpz_t pub, const mpz_t m)
{
	size_t elemsz;
	mpz_t base, elem;
	const unsigned char *data;
	char *basebuf;

	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	if (!(basebuf = malloc(elemsz + 1)))
		return 1;
	data = src;

	mpz_init(base);
	mpz_init(elem);

	mpz_set(base, m);
	mpz_set(elem, m);


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

	mpz_clear(elem);
	mpz_clear(base);
	free(basebuf);
	return 0;
}

// FIXME compute number of bytes that can fit in modulo
/**
 * Convert \a size RSA bytes from blk back to original data and store in \a dst.
 * \a priv and \a m are obtained from gen_keypair.
 */
int rsa_decrypt(char *dst, const char *blk, size_t size, const mpz_t priv, const mpz_t m)
{
	size_t elemsz;
	mpz_t base, ch;
	char *basebuf;

	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	if (!(basebuf = malloc(elemsz + 1)))
		return 1;

	mpz_init(base);
	mpz_init(ch);

	for (size_t i = 0; i < size; i += elemsz) {
		strncpy(basebuf, &blk[i], elemsz);
		basebuf[elemsz] = '\0';

		mpz_set_str(base, basebuf, 16);
		mpz_powm(ch, base, priv, m);

		*dst++ = mpz_get_ui(ch);
	}

	mpz_clear(ch);
	mpz_clear(base);
	free(basebuf);
	return 0;
}

int test_rsa1(const char *msg, const mpz_t pub, const mpz_t priv, const mpz_t m)
{
	int err = 1;
	size_t msglen, elemsz, bufsz;
	char *basebuf = NULL, *buf = NULL, *orig = NULL;
	mpz_t base, elem;
	mpz_init(base);
	mpz_init(elem);

	// setup buffers
	msglen = strlen(msg) + 1;
	elemsz = 2 * ((mpz_sizeinbase(m, 16) - 1) / 2 + 1);
	bufsz = msglen * elemsz;

	if (!(buf = malloc(bufsz)) || !(orig = calloc(msglen, 1)))
		goto fail;

	// dump buffer info
	printf("blk len: %zu (orig: %zu, elem: %zu)\n", bufsz, msglen, elemsz);

	rsa_encrypt(buf, msg, msglen, pub, m);
	rsa_decrypt(orig, buf, bufsz, priv, m);

	printf("decrypted: %s\n", orig);
	puts(strcmp(msg, orig) ? "fail" : "ok");

	err = 0;
fail:
	free(orig);
	free(basebuf);
	free(buf);
	mpz_clear(elem);
	mpz_clear(base);
	return err;
}

int rsa_enc2(void *dst, const void *src, size_t rounds, const mpz_t pub, const mpz_t m, size_t items, size_t msz)
{
	mpz_t sum, base, elem, k, tmp;
	unsigned char *to = dst;
	const unsigned char *from = src;
	// hexadecimal stringified scratch buffer for `m'
	char *basebuf;
	size_t bufsz;

	bufsz = 2 * msz + 1;
	if (!(basebuf = malloc(bufsz)))
		return 1;

	mpz_inits(sum, base, elem, k, tmp, NULL);

	for (size_t i = 0; i < rounds; ++i) {
		mpz_set_ui(k, 1);
		mpz_set_ui(sum, 0);

		for (size_t j = 0; j < items; ++j) {
			// compute item sum
			mpz_set_ui(base, 0);
			mpz_addmul_ui(base, k, *from++);
			mpz_add(elem, sum, base);
			mpz_set(sum, elem);

			// update multiplier for next byte
			mpz_swap(tmp, k);
			mpz_mul_ui(k, tmp, 256);
		}
		gmp_printf("sum : %0*ZX\n", 2 * msz, sum);
		mpz_powm(elem, sum, pub, m);

		gmp_sprintf(basebuf, "%0*ZX", 2 * msz, elem);
		printf("encr: %s\n", basebuf);

		// pack basebuf and store in dst
		for (size_t j = 0, k = 0; j < msz; ++j, k += 2)
			*to++ = atohex[(unsigned char)basebuf[k]] << 4 | atohex[(unsigned char)basebuf[k + 1]];
	}

	free(basebuf);
	mpz_clears(tmp, k, elem, base, sum, NULL);
	return 0;
}

int rsa_dec2(void *dst, const void *src, size_t rounds, const mpz_t priv, const mpz_t m, size_t msz)
{
	mpz_t base, ch;
	unsigned char *to = dst;
	const unsigned char *from = src;
	// hexadecimal stringified scratch buffer for `m'
	char *rwbuf;
	size_t bufsz;

	bufsz = 2 * msz + 1;
	if (!(rwbuf = malloc(bufsz)))
		return 1;

	mpz_init(base);
	mpz_init(ch);

	for (size_t i = 0; i < rounds; ++i) {
		size_t j, k;

		for (j = 0, k = 0; j < msz; ++j, k += 2) {
			rwbuf[k] = str_hex_lo[*from >> 4];
			rwbuf[k + 1] = str_hex_lo[*from & 0xf];
			++from;
		}
		rwbuf[k] = '\0';

		mpz_set_str(base, rwbuf, 16);
		gmp_printf("read: %0*ZX\n", 2 * msz, base);

		mpz_powm(ch, base, priv, m);
		gmp_sprintf(rwbuf, "%0*ZX", 2 * msz, ch);
		printf("decr: %s\n", rwbuf);

		// depack and write bytes in reversed order
		for (j = 2 * msz; j > 2; j -= 2)
			*to++ = atohex[(unsigned char)rwbuf[j - 1]] | atohex[(unsigned char)rwbuf[j - 2]] << 4;
	}

	mpz_clear(ch);
	mpz_clear(base);
	free(rwbuf);
	return 0;
}

int test_rsa2(const char *msg, const mpz_t pub, const mpz_t priv, const mpz_t m)
{
	int err = 1;
	size_t msglen, items = 0, msz, rwbufsz;
	char *rwbuf0 = NULL, *rwbuf1 = NULL;
	mpz_t base, elem;

	mpz_inits(base, elem, NULL);

	// compute maximum number of bytes (i.e. items) that fit in m
	mpz_set(base, m);
	mpz_set(elem, m);

	while (mpz_cmp_ui(base, 256) >= 0) {
		++items;
		mpz_tdiv_q_ui(elem, base, 256);
		if (mpz_cmp_ui(elem, 256) < 0)
			break;
		++items;
		mpz_tdiv_q_ui(base, elem, 256);
	}

	// NOTE storage size for m may be one bigger than `items'
	mpz_set(elem, base);
	msz = mpz_sgn(elem) ? items + 1 : items;

	printf("m stats: items=%zu, size=%zu\n", items, msz);

	// setup buffers
	msglen = strlen(msg) + 1;
	rwbufsz = msz * ((msglen - 1) / msz + 1);

	printf("rw stats: size=%zu, data_size=%zu\n", rwbufsz, msglen);

	if (!(rwbuf0 = calloc(rwbufsz, 1)) || !(rwbuf1 = calloc(rwbufsz, 1)))
		goto fail;

	strcpy(rwbuf0, msg);
	if ((err = rsa_enc2(rwbuf1, rwbuf0, rwbufsz / msz, pub, m, items, msz)))
		goto fail;

	memset(rwbuf0, 0, rwbufsz);
	if ((err = rsa_dec2(rwbuf0, rwbuf1, rwbufsz / msz, priv, m, msz)))
		goto fail;

	printf("decrypted: %s\n", rwbuf0);
	puts(strcmp(rwbuf0, msg) ? "fail" : "ok");
	err = 0;
fail:
	free(rwbuf1);
	free(rwbuf0);
	mpz_clears(elem, base, NULL);
	return err;
}

int test_rsa(const char *msg, const mpz_t pub, const mpz_t priv, const mpz_t m)
{
	int err;

	printf("to encrypt: \"%s\"\n", msg);
	puts("-- rsa 1.0 --");

	if ((err = test_rsa1(msg, pub, priv, m)))
		return err;
	puts("-- rsa 2.0 --");
	if ((err = test_rsa2(msg, pub, priv, m)))
		return err;

	puts("-------------");
	return 0;
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

#if 1
	bits = 256 + (unsigned)(16.0f * rand() / RAND_MAX);
	reps = 40 + (unsigned)(10.0f * rand() / RAND_MAX);

	printf("prime config: bits=%u, reps=%u\n", bits, reps);

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
#else
	// XXX reproducable regression testing
	bits = 261;
	reps = 43;

	printf("prime config: bits=%u, reps=%u\n", bits, reps);

	mpz_set_str(p, "12373DADF62BC44E605B57856CE73805315FD800A7D07E76A2D6346B8AABAAB601", 16);
	mpz_set_str(q, "15DB445277AD1EC3F81F60D7AE5C56FB74A124E76ADC8370039F51370F7129521F", 16);

	gmp_printf("p: %ZX\nq: %ZX\n", p, q);

	mpz_set_str(pub, "91260FA6137D1AC959F43DB2AD0642118B7420A4049C38C00490F9F4FB10E167122663329129E8FBD49C3A7BF00BB93E47A997A95167B2195FDDD0CA52B3ABB4E7", 16);
	mpz_set_str(priv, "CA9FFD8CC7465623721BB27EDD2D6C67383D9C833277437A4B73F06D044F7DC686E5805284F671B1B38332576FA68E9AD2DB1C9D5B2418C223DD8369FF5022CED7", 16);
	mpz_set_str(m, "18E222D939B24D8CE117089BC274250A148D7B73355CA8FFC0B631E30C2086AFEFFF3B86E60B9F88FFAFB0364E67030A5FF05F10E934AC1FA451B332A840F215C1F", 16);
#endif
	// dump keys
	gmp_printf("mod        : %ZX\npublic key : %ZX\nprivate key: %ZX\n", m, pub, priv);

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
