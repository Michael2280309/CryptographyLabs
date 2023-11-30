#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define LZ64 0x00000000FFFFFFFF
#include "rsa.h"

uint64_t rsa_prime();
int rsa_miller_rabin_test(uint64_t n, int k);

uint64_t rsa_pow_mod(uint64_t a, uint64_t b, uint64_t c)
{
	uint64_t power = b, d = a, res = 1;
	while (power) {
		if (power & 1)
			res = (uint64_t)(res * d) % c;
		d = (d * d) % c;
		power = power >> 1;
	}
	return res % c;
}

uint64_t rsa_gcd(uint64_t u, uint64_t v)
{
	// Обчислення найбільшого спільного дільника 
	// за допомогою двійкового метода Кнута.

	int shift;

	/* нСД(0,v) == v; НСД(u,0) == u, НСД(0,0) == 0 */
	if (u == 0) return v;
	if (v == 0) return u;

	/* Нехай shift := lg K, де K - найбільша степінь двійки, що ділить u і v */
	for (shift = 0; ((u | v) & 1) == 0; ++shift) {
		u >>= 1;
		v >>= 1;
	}

	while ((u & 1) == 0)
		u >>= 1;

	/* Починаючи звідси, u завжди непарне. */
	do {
		/* позбудемось всіх дільників 2 в v -- вони не спільні */
		/*   зауваження: v не 0, тож цикл завершиться */
		while ((v & 1) == 0)  /* Loop X */
			v >>= 1;
		/* Тепер u і v - непарні. Якщо потрібно обміняємо їх, щоб u <= v,
		   тоді встановимо v = v - u (парне число). Для довгих чисел
		   обмін - всього переміщення вказівників, і віднімання
		   можна зробити на місці */
		if (u > v) {
			uint64_t t = v; v = u; u = t; // Обмін u і v.
		}

		v = v - u; // Тут v >= u.
	} while (v != 0);

	/* Відновлюємо спільні дільники 2 */
	return u << shift;
}

uint64_t rsa_gen_E(uint64_t fi) {

	uint64_t e;

	for (e = 2; e < fi; e++)
	{
		if (rsa_gcd(e, fi) == 1)
		{
			return e;
		}
	}

	return -1;
}

uint64_t rsa_D_calculate(uint64_t E, uint64_t fi) {

	// Розширена теорема Евкліда												 
	//uint64_t D = 2;
	//while (((E * D) % fi) != 1) ++D;
	//return D;

	uint64_t d;
	uint64_t k = 1;

	while (1)
	{
		k = k + fi;

		if (k % E == 0)
		{
			d = (k / E);
			return d;
		}
	}
}

void rsa_key_gen(uint64_t* E, uint64_t* D, uint64_t* n) {
	uint64_t p = rsa_prime();
	uint64_t q = rsa_prime();
	// Перевіка нерівності p та q
	while (p == q) { p = rsa_prime(); q = rsa_prime(); }
	*n = p * q;
	uint64_t fi = (p - 1) * (q - 1);
	*E = rsa_gen_E(fi);
	*D = rsa_D_calculate(*E, fi);
}

uint64_t rsa_encrypt(uint64_t a, uint64_t E, uint64_t n) {
	uint64_t b = rsa_pow_mod(a, E, n);
	return b;
}

uint64_t rsa_decrypt(uint64_t b, uint64_t D, uint64_t n) {
	uint64_t c = rsa_pow_mod(b, D, n);
	return c;
}

uint64_t rsa_prime() {
	uint64_t nr = die();

	// Ряд перших простих чисел, використовується для відсіювання 
	// майже 80% непарних чисел, що не є простими, перш ніж обробляти їх
	// алгоритмом Міллера-Рабіна. Але оскільки числа не є настільки великими,
	// ми могли б використати звичайний алгоритм Евкліда, не зважаючи на  
	// кількість машинних інструкцій.

	uint64_t fewPrimes[] = { 2, 3,	5,	7,	11,	13,	17,	19,	23,	29,
31,	37,	41,	43,	47,	53,	59,	61,	67,	71, 73,	79,	83,	89,	97,	101,
103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167,
173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
233, 239, 241, 251
	};

	int div = 0;
	while (1) {
		div = 0;
		for (uint64_t i = 0; i < sizeof(fewPrimes) / sizeof(fewPrimes[0]); i++)
			if (nr % fewPrimes[i] == 0) div = 1;

		// 16 циклів перевіки тестом Міллера
		if (!div && rsa_miller_rabin_test(nr, 16)) return nr;
		nr = die();
	}
}

int rsa_miller_rabin_test(uint64_t n, int k) {
	// Тест Міллера-Рабіна є більш еффективним, ніж
	// звичайний алгоритм Евкліда. З іншої сторони,
	// точність цього алгоритму не є стопроцентною.
	// 0 - не є простим числом, 1 - з великою 
	// імовірністю є простим числом.

	uint64_t t = n - 1;
	while (t % 2 == 0) { t /= 2; }

	uint64_t a, t1;
	while (k--) {
		a = (rand() % (n - 1 - 2)) + 2;
		uint64_t x = rsa_pow_mod(a, t, n);
		if (x == 1 || x == n - 1) continue;
		t1 = t;
		while (1) {
			if (t1 == n - 1) break;
			if (x == 1) break;
			if (x == n - 1) break;

			x = rsa_pow_mod(x, 2, n);
			t1 *= 2;
		}
		if (x != n - 1 && t1 % 2 == 0)
		{
			return 0;
		}
	}
	// Якщо усі етапи перевірки пройдені, число просте.

	return 1;
}



	


