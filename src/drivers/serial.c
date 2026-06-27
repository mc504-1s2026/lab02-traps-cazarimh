#include <kernel/serial.h>
#include <kernel/panic.h>
#include <kernel/mm.h>
#include <arch/plic.h>
#include <arch/spinlock.h>
#include <arch/csr.h>

#define SERIAL_BASE_VA ((void*)((u64)SERIAL_BASE + KERNEL_DIRECT_MAP_START))
#define BUF_SIZE 512
char serial_buffer[BUF_SIZE];
size_t serial_len = 0;

struct spinlock serial_lock;

void serial_init()
{
	serial_len = 0;

	spin_init(&serial_lock);

	iowrite8(0x3, SERIAL_BASE_VA + SERIAL_LCR);
	iowrite8(0x1, SERIAL_BASE_VA + SERIAL_FCR);
}

void serial_irq_enable()
{
	csr_set(CSR_SIE, CSR_SIE_SEIE);

	plic_irq_set_priority(IRQ_SERIAL, 1);
	plic_hart_set_threshold(0, 0);
	plic_hart_enable_irq(0, IRQ_SERIAL);

	iowrite8(SERIAL_IER_ERBFI, SERIAL_BASE_VA + SERIAL_IER);
}

void serial_irq_disable()
{
	iowrite8(0x0, SERIAL_BASE_VA + SERIAL_IER);
	csr_clear(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq()
{
	spin_lock(&serial_lock);
	
	if (ioread8(SERIAL_BASE_VA + SERIAL_LSR) & SERIAL_LSR_DTR) {
		char c = (char)ioread8(SERIAL_BASE_VA + SERIAL_RBR);

		if (serial_len < BUF_SIZE) serial_buffer[serial_len++] = c;
		else error("serial buffer run out of space\n");
	}

	spin_unlock(&serial_lock);
}

size_t serial_read(char *buf)
{
	if (serial_len == 0) return 0;

	u64 flags = spin_lock_irqsave(&serial_lock);

	size_t len = serial_len;

	for (size_t i = 0; i < len; i++) buf[i] = serial_buffer[i];

	serial_len = 0;

	spin_unlock_irqrestore(&serial_lock, flags);
	
	return len;
}

void serial_puts(char *str)
{
	for (int i = 0; str[i] != '\0'; i++) serial_putc(str[i]);
}

void serial_putc(char c)
{
	while (!(ioread8(SERIAL_BASE_VA + SERIAL_LSR) & SERIAL_LSR_THRE));

	iowrite8((u8)c, SERIAL_BASE_VA + SERIAL_THR);
}
