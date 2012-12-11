#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include <string.h>
#include "counter.h"

/*
 * Function prototypes for the counter driver.
 */
FORWARD _PROTOTYPE( int counter_open,      (message *m) );
FORWARD _PROTOTYPE( int counter_close,     (message *m) );
FORWARD _PROTOTYPE( struct device * counter_prepare, (dev_t device) );
FORWARD _PROTOTYPE( int counter_transfer,  (endpoint_t endpt, int opcode,
			u64_t position, iovec_t *iov,
			unsigned int nr_req,
			endpoint_t user_endpt) );

/* SEF functions and variables. */
FORWARD _PROTOTYPE( void sef_local_startup, (void) );
FORWARD _PROTOTYPE( int sef_cb_init, (int type, sef_init_info_t *info) );
FORWARD _PROTOTYPE( int sef_cb_lu_state_save, (int) );
FORWARD _PROTOTYPE( int lu_state_restore, (void) );

/* Entry points to the counter driver. */
PRIVATE struct chardriver counter_tab =
{
	counter_open,
	counter_close,
	nop_ioctl,
	counter_prepare,
	counter_transfer,
	nop_cleanup,
	nop_alarm,
	nop_cancel,
	nop_select,
	NULL
};

/** Represents the /dev/counter device. */
PRIVATE struct device counter_device;

/** State variable to count the number of times the device has been opened. */
PRIVATE int open_counter;

PRIVATE int counter_open(message *UNUSED(m))
{
	++open_counter;
	return OK;
}

PRIVATE int counter_close(message *UNUSED(m))
{
	return OK;
}

PRIVATE struct device * counter_prepare(dev_t UNUSED(dev))
{
	counter_device.dv_base = make64(0, 0);
	counter_device.dv_size = make64(sizeof(int), 0);
	return &counter_device;
}

PRIVATE int counter_transfer(endpoint_t endpt, int opcode, u64_t position,
		iovec_t *iov, unsigned nr_req, endpoint_t UNUSED(user_endpt))
{
	int bytes, ret;

	//make open_counter into a string called str
	char str[128];
    sprintf(str,"%i",open_counter);

	if (nr_req != 1)
	{
		/* This should never trigger for character drivers at the moment. */
		printf("COUNTER: vectored transfer request, using first element only\n");
	}

	bytes = strlen(COUNTER_MESSAGE) - ex64lo(position) < iov->iov_size ? //ex64lo extracts the low 32 bits of a 64 bit number
		strlen(COUNTER_MESSAGE) - ex64lo(position) : iov->iov_size;

	if (bytes <= 0)
	{
		return OK;
	}

	switch (opcode)
	{
		case DEV_GATHER_S:
            //SYS_SAFECOPYTO: Copy data from one's own address space to a granted memory area of another process. The caller must have CPF_WRITE access to the grant.
            //int sys_safecopyto(endpoint_t dest, cp_grant_id_t grant, vir_bytes grant_offset, vir_bytes my_address, size_t bytes);
            //typedef unsigned int vir_bytes;       /* virtual addresses and lengths in bytes */
            //cp_grant_id_t is an int or a long, I'm not sure which.
            //endpoint_t is an int.
			ret = sys_safecopyto(endpt, (cp_grant_id_t) iov->iov_addr, 0,
					(vir_bytes) (&str + ex64lo(position)),
					sizeof(str), D);
			iov->iov_size -= bytes;
			break;

		default:
			return EINVAL;
	}
	return ret;
}

PRIVATE int sef_cb_lu_state_save(int UNUSED(state)) {
	/* Save the state. */
	ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);

	return OK;
}

PRIVATE int lu_state_restore() {
	/* Restore the state. */
	u32_t value;

	ds_retrieve_u32("open_counter", &value);
	ds_delete_u32("open_counter");
	open_counter = (int) value;

	return OK;
}

PRIVATE void sef_local_startup()
{
	/*
	 * Register init callbacks. Use the same function for all event types
	 */
	sef_setcb_init_fresh(sef_cb_init);
	sef_setcb_init_lu(sef_cb_init);
	sef_setcb_init_restart(sef_cb_init);

	/*
	 * Register live update callbacks.
	 */
	/* - Agree to update immediately when LU is requested in a valid state. */
	sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
	/* - Support live update starting from any standard state. */
	sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
	/* - Register a custom routine to save the state. */
	sef_setcb_lu_state_save(sef_cb_lu_state_save);

	/* Let SEF perform startup. */
	sef_startup();
}

PRIVATE int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
	/* Initialize the counter driver. */
	int do_announce_driver = TRUE;

	open_counter = 0;
	switch(type) {
		case SEF_INIT_FRESH:
			break;

		case SEF_INIT_LU:
			/* Restore the state. */
			lu_state_restore();
			do_announce_driver = FALSE;
			break;

		case SEF_INIT_RESTART:
			break;
	}

	/* Announce we are up when necessary. */
	if (do_announce_driver) {
		chardriver_announce();
	}

	/* Initialization completed successfully. */
	return OK;
}

PUBLIC int main(void)
{
	/*
	 * Perform initialization.
	 */
	sef_local_startup();

	/*
	 * Run the main loop.
	 */
	chardriver_task(&counter_tab, CHARDRIVER_SYNC);
	return OK;
}

