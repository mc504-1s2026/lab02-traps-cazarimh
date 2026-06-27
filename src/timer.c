#include <arch/timer.h>
#include <kernel/panic.h>
#include <arch/csr.h>

u64 timer_read()
{	
	return csr_read(CSR_TIME);
}

void timer_irq_enable()
{
	csr_set(CSR_SIE, CSR_SIE_STIE);
}

void timer_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
	timer_irq_enable();
	csr_write(CSR_STIMECMP, timer_read() + secs*TIMER_FREQ);
}

void timer_done()
{
	csr_write(CSR_STIMECMP, -1ULL);
}

void timer_irq()
{
	info("alarm\n");
	timer_done();
}
