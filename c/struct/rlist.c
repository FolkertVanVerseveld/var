/*
speciale lijst wat mbv een stack de als leeg gemarkeerde
posities in O(1) kan reclaimen.
(beetje lastig uit te leggen :P)
en oja, het voorbeeld is best slecht :p

SMT maakt hier veel gebruik van (zie _smt.h)
alleen die functies returnen 0 bij success en 1 bij errors
(in deze file is het dus 0 bij error en 1 bij success)
*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define BANKSZ 16

struct rlist {
	int saldo[BANKSZ];
	unsigned rpop[BANKSZ];
	unsigned n, ri;
} bank;

int bank_new(unsigned *rekening, int saldo)
{
	if (bank.n >= BANKSZ)
		return 0; // lijst vol
	unsigned slot;
	// gebruik lege sloten eerst
	if (bank.ri)
		slot = bank.rpop[--bank.ri];
	else
		slot = bank.n++;
	bank.saldo[slot] = saldo;
	*rekening = slot;
	return 1;
}

int bank_saldo(unsigned rekening, int *saldo)
{
	if (rekening >= bank.n)
		return 0; // verkeerde index
	*saldo = bank.saldo[rekening];
	return 1;
}

int bank_free(unsigned rekening)
{
	if (rekening >= bank.n)
		return 0; // verkeerde index
	// markeer als leeg
	bank.rpop[bank.ri++] = rekening;
	return 1;
}

int main(void)
{
	unsigned r_a, r_b, r_c;
	int saldo;
	bank_new(&r_a, 80);
	bank_new(&r_b, 120);
	bank_saldo(r_a, &saldo);
	printf("@%02u r_a = %u\n", r_a, saldo);
	bank_free(r_a);
	bank_new(&r_c, 45);
	bank_saldo(r_c, &saldo);
	printf("@%02u r_c = %u\n", r_c, saldo);
	return 0;
}
