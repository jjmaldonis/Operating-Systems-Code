/* This file contains the scheduling policy for SCHED
 *
 * The entry points are:
 *   do_noquantum:        Called on behalf of process' that run out of quantum
 *   do_start_scheduling  Request to start scheduling a proc
 *   do_stop_scheduling   Request to stop scheduling a proc
 *   do_nice		  Request to change the nice level on a proc
 *   init_scheduling      Called from main.c to set up/prepare scheduling
 */
#include <stdio.h>
#include "sched.h"
#include "schedproc.h"
#include <assert.h>
#include <minix/com.h>
#include <machine/archtypes.h>
#include "kernel/proc.h" /* for queue constants */

PRIVATE timer_t sched_timer;
PRIVATE unsigned balance_timeout;

#define BALANCE_TIMEOUT	5 /* how often to balance queues in seconds */

FORWARD _PROTOTYPE( int schedule_process, (struct schedproc * rmp,
            unsigned flags));
FORWARD _PROTOTYPE( void set_order, () );
FORWARD _PROTOTYPE( void print_stuff, () );

#define SCHEDULE_CHANGE_PRIO	0x1
#define SCHEDULE_CHANGE_QUANTUM	0x2
#define SCHEDULE_CHANGE_CPU	0x4

#define SCHEDULE_CHANGE_ALL	(	\
        SCHEDULE_CHANGE_PRIO	|	\
        SCHEDULE_CHANGE_QUANTUM	|	\
        SCHEDULE_CHANGE_CPU		\
        )

#define schedule_process_local(p)	\
    schedule_process(p, SCHEDULE_CHANGE_PRIO | SCHEDULE_CHANGE_QUANTUM)
#define schedule_process_migrate(p)	\
    schedule_process(p, SCHEDULE_CHANGE_CPU)

#define CPU_DEAD	-1

#define cpu_is_available(c)	(cpu_proc[c] >= 0)

#define DEFAULT_USER_TIME_SLICE 200

/* processes created by RS are sysytem processes */
#define is_system_proc(p)	((p)->parent == RS_PROC_NR)

PRIVATE unsigned cpu_proc[CONFIG_MAX_CPUS];

PRIVATE void pick_cpu(struct schedproc * proc)
{
#ifdef CONFIG_SMP
    unsigned cpu, c;
    unsigned cpu_load = (unsigned) -1;

    if (machine.processors_count == 1) {
        proc->cpu = machine.bsp_id;
        return;
    }

    /* schedule sysytem processes only on the boot cpu */
    if (is_system_proc(proc)) {
        proc->cpu = machine.bsp_id;
        return;
    }

    /* if no other cpu available, try BSP */
    cpu = machine.bsp_id;
    for (c = 0; c < machine.processors_count; c++) {
        /* skip dead cpus */
        if (!cpu_is_available(c))
            continue;
        if (c != machine.bsp_id && cpu_load > cpu_proc[c]) {
            cpu_load = cpu_proc[c];
            cpu = c;
        }
    }
    proc->cpu = cpu;
    cpu_proc[cpu]++;
#else
    proc->cpu = 0;
#endif
}

/*===========================================================================*
 *				do_noquantum				     *
 *===========================================================================*/

PUBLIC int do_noquantum(message *m_ptr)
{
    register struct schedproc *rmp;
    int rv, proc_nr_n;

    if (sched_isokendpt(m_ptr->m_source, &proc_nr_n) != OK) {
        printf("SCHED: WARNING: got an invalid endpoint in OOQ msg %u.\n",
                m_ptr->m_source);
        return EBADEPT;
    }

    rmp = &schedproc[proc_nr_n];
    rmp->time_used++; /* Jason - increment used_time because we ran out of quantum */

    if (rmp->priority < MIN_USER_Q)
    {
        rmp->priority += 1; /* lower priority */
    }

    set_order(); //run set_order function
    if ((rv = schedule_process_local(rmp)) != OK) {
        return rv;
    }
    return OK;
}

/*===========================================================================*
 *				do_stop_scheduling			     *
 *===========================================================================*/
PUBLIC int do_stop_scheduling(message *m_ptr)
{
    register struct schedproc *rmp;
    int proc_nr_n;

    /* check who can send you requests */
    if (!accept_message(m_ptr))
        return EPERM;

    if (sched_isokendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n) != OK) {
        printf("SCHED: WARNING: got an invalid endpoint in OOQ msg "
                "%ld\n", m_ptr->SCHEDULING_ENDPOINT);
        return EBADEPT;
    }

    rmp = &schedproc[proc_nr_n];
#ifdef CONFIG_SMP
    cpu_proc[rmp->cpu]--;
#endif
    rmp->flags = 0; /*&= ~IN_USE;*/

    return OK;
}

/*===========================================================================*
 *				do_start_scheduling			     *
 *===========================================================================*/
PUBLIC int do_start_scheduling(message *m_ptr)
{
    register struct schedproc *rmp;
    int rv, proc_nr_n, parent_nr_n;

    /* we can handle two kinds of messages here */
    assert(m_ptr->m_type == SCHEDULING_START || 
            m_ptr->m_type == SCHEDULING_INHERIT);

    /* check who can send you requests */
    if (!accept_message(m_ptr))
        return EPERM;

    /* Resolve endpoint to proc slot. */
    if ((rv = sched_isemtyendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n))
            != OK) {
        return rv;
    }
    rmp = &schedproc[proc_nr_n];
    rmp->time_used = 0; /*Set time used to 0*/

    /* Populate process slot */
    rmp->endpoint     = m_ptr->SCHEDULING_ENDPOINT;
    rmp->parent       = m_ptr->SCHEDULING_PARENT;
    rmp->max_priority = (unsigned) m_ptr->SCHEDULING_MAXPRIO;
    if (rmp->max_priority >= NR_SCHED_QUEUES) {
        return EINVAL;
    }

    /* Inherit current priority and time slice from parent. Since there
     * is currently only one scheduler scheduling the whole system, this
     * value is local and we assert that the parent endpoint is valid */
    if (rmp->endpoint == rmp->parent) {
        /* We have a special case here for init, which is the first
           process scheduled, and the parent of itself. */
        rmp->priority   = USER_Q;
        rmp->time_slice = DEFAULT_USER_TIME_SLICE;

        /*
         * Since kernel never changes the cpu of a process, all are
         * started on the BSP and the userspace scheduling hasn't
         * changed that yet either, we can be sure that BSP is the
         * processor where the processes run now.
         */
#ifdef CONFIG_SMP
        rmp->cpu = machine.bsp_id;
        /* FIXME set the cpu mask */
#endif
    }

    switch (m_ptr->m_type) {

        case SCHEDULING_START:
            /* We have a special case here for system processes, for which
             * quanum and priority are set explicitly rather than inherited 
             * from the parent */
            rmp->priority   = rmp->max_priority;
            rmp->time_slice = (unsigned) m_ptr->SCHEDULING_QUANTUM;
            break;

        case SCHEDULING_INHERIT:
            /* Inherit current priority and time slice from parent. Since there
             * is currently only one scheduler scheduling the whole system, this
             * value is local and we assert that the parent endpoint is valid */
            if ((rv = sched_isokendpt(m_ptr->SCHEDULING_PARENT,
                            &parent_nr_n)) != OK)
                return rv;

            rmp->priority = schedproc[parent_nr_n].priority;
            rmp->time_slice = schedproc[parent_nr_n].time_slice;
            break;

        default: 
            /* not reachable */
            assert(0);
    }

    /* Take over scheduling the process. The kernel reply message populates
     * the processes current priority and its time slice */
    if ((rv = sys_schedctl(0, rmp->endpoint, 0, 0, 0)) != OK) {
        printf("Sched: Error taking over scheduling for %d, kernel said %d\n",
                rmp->endpoint, rv);
        return rv;
    }
    rmp->flags = IN_USE;

    /* Schedule the process, giving it some quantum */
    pick_cpu(rmp);
    set_order(); /* Run set_order function*/
    while ((rv = schedule_process(rmp, SCHEDULE_CHANGE_ALL)) == EBADCPU) {
        /* don't try this CPU ever again */
        cpu_proc[rmp->cpu] = CPU_DEAD;
        pick_cpu(rmp);
    }

    if (rv != OK) {
        printf("Sched: Error while scheduling process, kernel replied %d\n",
                rv);
        return rv;
    }

    /* Mark ourselves as the new scheduler.
     * By default, processes are scheduled by the parents scheduler. In case
     * this scheduler would want to delegate scheduling to another
     * scheduler, it could do so and then write the endpoint of that
     * scheduler into SCHEDULING_SCHEDULER
     */

    m_ptr->SCHEDULING_SCHEDULER = SCHED_PROC_NR;

    return OK;
}

/*===========================================================================*
 *				do_nice					     *
 *===========================================================================*/
PUBLIC int do_nice(message *m_ptr)
{
    struct schedproc *rmp;
    int rv;
    int proc_nr_n;
    unsigned new_q, old_q, old_max_q;

    /* check who can send you requests */
    if (!accept_message(m_ptr))
        return EPERM;

    if (sched_isokendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n) != OK) {
        printf("SCHED: WARNING: got an invalid endpoint in OOQ msg "
                "%ld\n", m_ptr->SCHEDULING_ENDPOINT);
        return EBADEPT;
    }

    rmp = &schedproc[proc_nr_n];
    new_q = (unsigned) m_ptr->SCHEDULING_MAXPRIO;
    if (new_q >= NR_SCHED_QUEUES) {
        return EINVAL;
    }

    /* Store old values, in case we need to roll back the changes */
    old_q     = rmp->priority;
    old_max_q = rmp->max_priority;

    /* Update the proc entry and reschedule the process */
    rmp->max_priority = rmp->priority = new_q;

    set_order(); /* Jason - do_nice should never run but I'll put this in here just in case */
    if ((rv = schedule_process_local(rmp)) != OK) {
        /* Something went wrong when rescheduling the process, roll
         * back the changes to proc struct */
        rmp->priority     = old_q;
        rmp->max_priority = old_max_q;
    }

    return rv;
}

/*===========================================================================*
 *				schedule_process			     *
 *===========================================================================*/
PRIVATE int schedule_process(struct schedproc * rmp, unsigned flags)
{
    int err;
    int new_prio, new_quantum, new_cpu;

    pick_cpu(rmp);

    if (flags & SCHEDULE_CHANGE_PRIO)
        new_prio = rmp->priority;
    else
        new_prio = -1;

    if (flags & SCHEDULE_CHANGE_QUANTUM)
        new_quantum = rmp->time_slice;
    else
        new_quantum = -1;

    if (flags & SCHEDULE_CHANGE_CPU)
        new_cpu = rmp->cpu;
    else
        new_cpu = -1;

    if ((err = sys_schedule(rmp->endpoint, new_prio,
                    new_quantum, new_cpu)) != OK) {
        printf("PM: An error occurred when trying to schedule %d: %d\n",
                rmp->endpoint, err);
    }

    return err;
}


/*===========================================================================*
 *				start_scheduling			     *
 *===========================================================================*/

PUBLIC void init_scheduling(void)
{
    balance_timeout = BALANCE_TIMEOUT * sys_hz();
    init_timer(&sched_timer);
}

/*===========================================================================*
 *				set_order				     *
 *===========================================================================*/

/* Sets the priorities to make it so that always has the process with the least time_used run.
 *I do this by going through all the processes and finding the minimum time_used of them all.
 *I then schedule_process the ones with time_used = min_time_used with priority = 1.
 *I schedule_process all the other ones with priority = 2.
 *This runs everytime (right before) the SCHED schedules a process to set everything to the correct priorities.
 */
PRIVATE void set_order()
{
    struct schedproc *rmp;
    int proc_nr;
    int min_quanta_used=100; /* used to record the min quanta used, start at 100 for comparison reasons */

    for (proc_nr=0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) { /* Jason - Find min_quanta_used */
        if (rmp->flags & IN_USE) {
            if (rmp->time_used < min_quanta_used) {
                min_quanta_used = rmp->time_used;
            }
        }
    }

    for (proc_nr=0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) {
        if (rmp->flags & IN_USE) {
            if (rmp->time_used == min_quanta_used) {
                rmp->priority = 1; /* change priority for any process that has min_quanta_used to 1 */
                schedule_process_local(rmp);
            }
            else {
                rmp->priority = 2; /* if not the min, set priority below the min's priority (2 will work) */
                schedule_process_local(rmp);
            }
        }
    }

}
/*===========================================================================*
 *				print_stuff				     *
 *===========================================================================*/

/* This function prints the time_used and priority of each process.
*/
PRIVATE void print_stuff()
{
    struct schedproc *rmp;
    int proc_nr;
    int num_processes=0; /* keeps track of the current number of processes */
    int num_procs_in_0=0; /* keeps track of the current number of processes with time_used = 0*/

    printf("PROC_NR | TIME_USED | PRIORITY | ENDPOINT  (only prints procs with !0 time_used)\n");
    for (proc_nr=0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) { /* Find min_quanta_used */
        if (rmp->flags & IN_USE) {
            if(rmp->time_used != 0)
            {
                printf("%d            %d           %d        %d\n",proc_nr,rmp->time_used,rmp->priority,rmp->endpoint);
            }
            else {
                num_procs_in_0++;
            }
            num_processes++;
        }
    }
    printf("Total number of processes: %d.\n",num_processes);
    printf("Number of processes with time_used = 0: %d.\n",num_procs_in_0);

    set_timer(&sched_timer, 500, print_stuff, 0); /* run print_stuff every 500 ticks - I want to call this recursively like balance_queues was called so that I can see this info every so often */

}
