#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>

#define CMD_BUF_SIZE 256
char cmd_buffer[CMD_BUF_SIZE];
size_t cmd_idx = 0;

void help() {
	info("  help          - Mostra a lista de comandos\n");
	info("  echo [msg]    - Exibe a mensagem digitada\n");
	info("  uptime        - Mostra o tempo de execução do sistema\n");
	info("  alarm [secs]  - Agenda um alarme para o futuro\n");
}

void uptime() {
	info("%llus\n", timer_read() / TIMER_FREQ);
}

void alarm(char *secs) {
	u64 seconds = 0;
	for (int i = 0; secs[i] != '\0'; i++) {
		if (secs[i] < '0' || secs[i] > '9') {
			error("%s não é um número válido\n", secs);
			return;
		}

		seconds *= 10;
		seconds += secs[i] - '0';
	}

	timer_set_alarm(seconds);
}

void echo(char *msg) {
	info("%s\n", msg);
}

void execute(char *command) {
	size_t len = strlen(command);

	while (len > 0 && (command[len-1] == '\n' || command[len-1] == '\r' || command[len-1] == ' '))
		command[--len] = '\0';
	
	if (len == 0) return;

	if (strcmp(command, "help") == 0) help();
	else if (strcmp(command, "uptime") == 0) uptime();
	else if (strncmp(command, "alarm ", 6) == 0) alarm(command + 6);
	else if (strncmp(command, "echo ", 5) == 0) echo(command + 5);
	else error("%s: comando não encontrado. Digite 'help' para verificar os comandos existentes\n", command);
}

void shell() {
  char aux_buffer[128];
  serial_puts("> ");

  while (1) {
    size_t n_chars = serial_read(aux_buffer);

    for (size_t i = 0; i < n_chars; i++) {
      char c = aux_buffer[i];

			switch (c) {
				case '\r':
				case '\n':
					serial_putc('\n');
					cmd_buffer[cmd_idx] = '\0';

					execute(cmd_buffer);

					cmd_idx = 0;
					serial_puts("> ");
					break;

				case '\177':
				case '\b':
					if (cmd_idx > 0) {
						cmd_idx--;
						serial_puts("\b \b");
					}
					break;

				default:
					if (cmd_idx < CMD_BUF_SIZE - 1) {
						cmd_buffer[cmd_idx++] = c;
						serial_putc(c);
					}
					break;
			}
    }
  }
}

extern int _hartid[];
void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	hart_irq_enable();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();

	shell();
}
