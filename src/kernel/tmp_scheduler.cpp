
/**
 * @file scheduler.h
 * @brief 调度器实现
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

#include "stdint.h"
#include "types.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "tmp_scheduler.h"
#include "vmm.h"
#include "cpu.hpp"
#include "core.h"

extern "C" void context_init(CPU::context_t *_context);
extern "C" void switch_context(CPU::context_t *_old, CPU::context_t *_new);
extern "C" void switch_os(CPU::context_t *_os);
/// idle 任务指针
task_t *idle_task[COMMON::CORES_COUNT] = {0};
/**
 * @brief idle 任务
 */
void idle(void) {
    while (1) {
        asm("wfi");
    }
    // 不会执行到这里
    assert(0);
    return;
}

pid_t liner_scheduler::alloc_pid(void) {
    pid_t res = g_pid++;
    return res;
}

void liner_scheduler::free_pid(pid_t _pid) {
    _pid = _pid;
    return;
}

// 获取下一个要执行的任务
task_t *liner_scheduler::get_next_task(void) {
    task_t *task = nullptr;
    // TODO: 如果当前任务的本次运行时间超过 1，进行切换
    // 如果任务未结束
    if (core_t::get_curr_task()->state == RUNNING) {
        // 如果不是 os 线程
        if (core_t::get_curr_task()->pid != 0) {
            // 重新加入队列
            spinlock.lock();
            task_queue->push(core_t::get_curr_task());
            spinlock.unlock();
        }
    }
    // 否则删除
    else {
        remove_task(core_t::get_curr_task());
    }
    spinlock.lock();
    // 如果队列为空
    if (task_queue->empty() == true) {
        // 运行 idle 线程
        task = idle_task[CPU::get_curr_core_id()];
    }
    else {
        // 不为空的话弹出一个任务
        task = task_queue->front();
        task_queue->pop();
    }
    spinlock.unlock();
    return task;
}

// 切换到下一个任务
void liner_scheduler::switch_task(void) {
    // 获取下一个线程并替换为当前线程下一个线程
    auto tmp = get_next_task();
    // 设置 core 当前线程信息
    core_t::set_curr_task(tmp);
    // 切换
    switch_context(&core_t::cores[CPU::get_curr_core_id()].sched_task->context,
                   &core_t::get_curr_task()->context);
    return;
}


liner_scheduler::liner_scheduler(void) : SCHEDULER("tmp scheduler") {
    return;
}

liner_scheduler::liner_scheduler(const mystl::string &_name):SCHEDULER(_name.c_str()){
    return;
}

liner_scheduler::~liner_scheduler(void) {
    return;
}

liner_scheduler &liner_scheduler::get_instance(void) {
    static liner_scheduler scheduler;
    return scheduler;
}

bool liner_scheduler::init(void) {
    // 初始化自旋锁
    spinlock.init("liner_scheduler");
    // 初始化进程队列
    task_queue = new mystl::queue<task_t *>;
    // 当前进程
    task_t *task = new task_t("init", nullptr);
    task->hartid = CPU::get_curr_core_id();
    task->pid    = 0;
    task->state  = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    context_init(&task->context);
    // 初始化 core 信息
    core_t::cores[CPU::get_curr_core_id()].core_id    = CPU::get_curr_core_id();
    core_t::cores[CPU::get_curr_core_id()].curr_task  = task;
    core_t::cores[CPU::get_curr_core_id()].sched_task = task;
    // 创建 idle 任务
    idle_task[CPU::get_curr_core_id()]        = new task_t("idle", idle);
    idle_task[CPU::get_curr_core_id()]->state = RUNNING;
    info("task init.\n");
    return true;
}

bool liner_scheduler::init_other_core(void) {
    // 当前进程
    task_t *task = new task_t("init other", nullptr);
    task->hartid = CPU::get_curr_core_id();
    task->pid    = 0;
    task->state  = RUNNING;
    // 原地跳转，填充启动进程的 task_t 信息
    context_init(&task->context);
    // 初始化 core 信息
    core_t::cores[CPU::get_curr_core_id()].core_id    = CPU::get_curr_core_id();
    core_t::cores[CPU::get_curr_core_id()].curr_task  = task;
    core_t::cores[CPU::get_curr_core_id()].sched_task = task;
    idle_task[CPU::get_curr_core_id()]        = new task_t("idle", idle);
    idle_task[CPU::get_curr_core_id()]->state = RUNNING;
    info("task other init.\n");
    return true;
}

void liner_scheduler::sched(void) {
    // TODO: 根据当前任务的属性进行调度
    switch_task();
    return;
}

void liner_scheduler::add_task(task_t *_task) {
    spinlock.lock();
    _task->pid   = alloc_pid();
    _task->state = RUNNING;
    // 将新进程添加到链表
    task_queue->push(_task);
    spinlock.unlock();
    return;
}

void liner_scheduler::remove_task(task_t *_task) {
    spinlock.lock();
    // 回收 pid
    free_pid(_task->pid);
    spinlock.unlock();
    return;
}

void liner_scheduler::exit(uint32_t _exit_code) {
    core_t::get_curr_task()->exit_code = _exit_code;
    core_t::get_curr_task()->state     = ZOMBIE;
    switch_os(&core_t::cores[CPU::get_curr_core_id()].sched_task->context);
    assert(0);
    return;
}

extern "C" void exit(int _status) {
    liner_scheduler::get_instance().exit(_status);
}
