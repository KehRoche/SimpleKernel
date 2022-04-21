
/**
 * @file task.cpp
 * @brief 任务抽象实现
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

#include "stdlib.h"
#include "stdint.h"
#include "iostream"
#include "task.h"
#include "cpu.hpp"
#include "common.h"
#include "pmm.h"

task_t::task_t(mystl::string _name, void (*_task)(void))
    : name(_name), spinlock(_name) {
    pid            = -1;
    state          = SLEEPING;
    parent         = nullptr;
    stack          = PMM::get_instance().alloc_pages_kernel(COMMON::STACK_SIZE /
                                                            COMMON::PAGE_SIZE);
    context.ra     = (uintptr_t)_task;
    context.coreid = CPU::get_curr_core_id();
    context.callee_regs.sp = stack + COMMON::STACK_SIZE;
    context.sscratch       = (uintptr_t)kmalloc(sizeof(CPU::context_t));
    context.sie |= CPU::SIE_STIE;
    context.sstatus |= CPU::SSTATUS_SIE;
    page_dir    = VMM::get_instance().get_pgd();
    slice       = 0;
    slice_total = 0;
    hartid      = CPU::get_curr_core_id();
    exit_code   = -1;
    return;
}

task_t::~task_t(void) {
    // 停止子进程
    // 回收页目录
    // 回收栈
    PMM::get_instance().free_pages(stack, 4);
    // 回收上下文
    free((void *)context.sscratch);
    // 回收其它
    std::cout << "del task: " << name << std::endl;
    return;
}

std::ostream &operator<<(std::ostream &_os, const task_t &_task) {
    printf("%s: hartid: 0x%X, pid 0x%X, state 0x%X, parent: 0x%X, stack: 0x%X",
           _task.name.c_str(), _task.hartid, _task.pid, _task.state,
           _task.parent, _task.stack);
    return _os;
}
