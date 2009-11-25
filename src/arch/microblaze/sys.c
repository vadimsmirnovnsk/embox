/**
 * @file arch/microblaze/sys.c
 *
 * @date 23.11.2009
 * @author: Anton Bondarev
 */

#include "types.h"
#include "kernel/sys.h"


void context_restore(CPU_CONTEXT * pcontext) {

}

static CPU_CONTEXT context;
volatile static BOOL started = FALSE;

int sys_exec_start(EXEC_FUNC f, int argc, char **argv) {
	if (started) {
		return -3;
	}

	int ret = -2;

	//context_save(&context);

	if (!started) {
		started = TRUE;
		ret = f(argc, argv);
	}
	started = FALSE;

	return ret;
}

int sys_exec_is_started(){
	return 0;
}

void sys_exec_stop() {
	if (!started) {
		return;
	}
	context_restore(&context);
}

void sys_halt() {
	//todo must be trap
}


/**
 * disable interrupt for atomic operation
 * return old psr reg
 */
unsigned long local_irq_save(void){
}

/**
 * set PSR_PIL in 0xF
 */
void local_irq_enable(void){
}

/**
 * restore PSR_PIL after __local_irq_save
 * param psr which was returned __local_irq_save
 */
void local_irq_restore(unsigned long old_psr){

}

