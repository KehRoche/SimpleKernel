
/**
 * @file spinlock.cpp
 * @brief 自旋锁实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "spinlock.h"
#include "cpu.hpp"
#include "scheduler.h"
#include "core.hpp"
#include "cpu.hpp"

// TODO

spinlock_t::spinlock_t(void) {
    name   = "";
    locked = false;
    hartid = -1;
    return;
}

spinlock_t::spinlock_t(const char *_name) : name(_name) {
    locked = false;
    hartid = -1;
    info("spinlock: %s init.\n", name);
    return;
}

void spinlock_t::lock(void) {
    push_off();
    if (is_holding() == true) {
        err("lock\n");
    }

    while (__atomic_test_and_set(&locked, 1) != 0) {
        ;
    }

    __sync_synchronize();

    hartid = COMMON::get_curr_core_id(CPU::READ_SP());
    return;
}

// Release the lock.
void spinlock_t::unlock(void) {
    if (is_holding() == false) {
        err("unlock\n");
    }
    hartid = -1;
    __sync_synchronize();
    __sync_lock_release(&locked);
    pop_off();
    return;
}

bool spinlock_t::is_holding(void) {
    bool r = (locked && (hartid == COMMON::get_curr_core_id(CPU::READ_SP())));
    return r;
}

void spinlock_t::push_off(void) {
    bool old = CPU::SSTATUS_INTR_status();

    CPU::DISABLE_INTR();

    if (cores[COMMON::get_curr_core_id(CPU::READ_SP())].noff == 0) {
        cores[COMMON::get_curr_core_id(CPU::READ_SP())].intr_enable = old;
    }
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].noff += 1;

    return;
}

void spinlock_t::pop_off(void) {
    // struct cpu *c = mycpu();
    if (CPU::SSTATUS_INTR_status() == true) {
        err("pop_off - interruptible\n");
    }

    if (cores[COMMON::get_curr_core_id(CPU::READ_SP())].noff < 1) {
        err("pop_off\n");
    }
    cores[COMMON::get_curr_core_id(CPU::READ_SP())].noff -= 1;

    if ((cores[COMMON::get_curr_core_id(CPU::READ_SP())].noff == 0) &&
        (cores[COMMON::get_curr_core_id(CPU::READ_SP())].intr_enable == true)) {
        CPU::ENABLE_INTR();
    }

    return;
}
