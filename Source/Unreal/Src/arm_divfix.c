// Software integer division for ARM Cortex-A7.
// The FunKey SDK's libgcc uses the 'udiv'/'sdiv' instructions in ARM mode,
// but Cortex-A7 only supports them in Thumb mode. Override with software
// implementations to avoid SIGILL.

typedef unsigned int uint32_t;
typedef int int32_t;

uint32_t __udivsi3(uint32_t num, uint32_t den)
{
	if (den == 0 || den > num)
		return 0;

	uint32_t quot = 0;
	uint32_t bit = 1;

	while (den < num && !(den & (1U << 31)))
	{
		den <<= 1;
		bit <<= 1;
	}

	while (bit)
	{
		if (num >= den)
		{
			num -= den;
			quot |= bit;
		}
		den >>= 1;
		bit >>= 1;
	}

	return quot;
}

int32_t __divsi3(int32_t num, int32_t den)
{
	int neg = 0;
	if (num < 0) { num = -num; neg = !neg; }
	if (den < 0) { den = -den; neg = !neg; }
	int32_t result = (int32_t)__udivsi3((uint32_t)num, (uint32_t)den);
	return neg ? -result : result;
}

uint32_t __aeabi_uidiv(uint32_t num, uint32_t den)
{
	return __udivsi3(num, den);
}

int32_t __aeabi_idiv(int32_t num, int32_t den)
{
	return __divsi3(num, den);
}

typedef struct { uint32_t quot; uint32_t rem; } uidiv_result;
typedef struct { int32_t quot; int32_t rem; } idiv_result;

uidiv_result __aeabi_uidivmod(uint32_t num, uint32_t den)
{
	uidiv_result r;
	r.quot = __udivsi3(num, den);
	r.rem = num - r.quot * den;
	return r;
}

idiv_result __aeabi_idivmod(int32_t num, int32_t den)
{
	idiv_result r;
	r.quot = __divsi3(num, den);
	r.rem = num - r.quot * den;
	return r;
}

uint32_t __umodsi3(uint32_t num, uint32_t den)
{
	return num - __udivsi3(num, den) * den;
}

int32_t __modsi3(int32_t num, int32_t den)
{
	return num - __divsi3(num, den) * den;
}
