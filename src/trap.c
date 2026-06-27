#include <kernel/trap.h>
#include <kernel/panic.h>
#include <arch/csr.h>

/* defined in src/trap_entry.S */
extern void trap_entry();

void handle_irq()
{
	u64 scause;

	scause = csr_read(CSR_SCAUSE);

	switch (scause) {
		case TRAP_TIMER_IRQ:
			timer_irq();
			break;
		case TRAP_EXTERNAL_IRQ:
			// TODO: implementar
			break;		
		default:
			break;
	}
}

void handle_exception()
{
	u64 scause, stval;

	scause = csr_read(CSR_SCAUSE);
	stval = csr_read(CSR_STVAL);
	
	switch (scause) {
		case EXCEPTION_INST_ACCESS_FAULT:
			error("instruction access fault at address 0x%x\n", stval);
			break;
		case EXCEPTION_LOAD_ACCESS_FAULT:
			error("load access fault at address 0x%x\n", stval);
			break;
		case EXCEPTION_STORE_ACCESS_FAULT:
			error("store access fault at address 0x%x\n", stval);
			break;
		case EXCEPTION_INST_PAGE_FAULT:
			error("instruction page fault at address 0x%x\n", stval);
			break;
		case EXCEPTION_LOAD_PAGE_FAULT:
			error("load page fault at address 0x%x\n", stval);
			break;
		case EXCEPTION_STORE_PAGE_FAULT:
			error("store page fault at address 0x%x\n", stval);
			break;
		default:
			error("uncaught exception! cause: 0x%x\n", scause);
			break;
	}
}

void trap_setup()
{
	csr_write(CSR_STVEC, trap_entry);
}

void handle_trap()
{
	u64 scause;

	scause = csr_read(CSR_SCAUSE);

	bool isInterrupt = scause & TRAP_IRQ_BIT;

	isInterrupt
		? handle_irq()
		: handle_exception();
}

void hart_irq_enable()
{
	csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

u64 hart_irq_save()
{
	u64 sstatus = csr_read(CSR_SSTATUS);
	hart_irq_disable();
	return sstatus & CSR_SSTATUS_SIE;
}

void hart_irq_restore(u64 flags)
{
	flags
		? hart_irq_enable()
		: hart_irq_disable();
}

void hart_irq_disable()
{
	csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}
