
/**
 * @file kernel_main.cpp
 * @brief 内核主要逻辑
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>Zone.N (Zone.Niuzh@hotmail.com)<td>迁移到 doxygen
 * </table>
 */

#include "kernel.h"
#include "common.h"
#include "iostream"
#include "cstdio"

/**
 * @brief 内核主要逻辑
 */
int kernel_main(int, char**) {
    show_info();

    // 进入死循环
    while (1) {
        ;
    }
    return 0;
}

/**
 * @brief 输出系统信息
 */
void show_info(void) {
    // 内核实际大小
    auto kernel_size  = COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR;
    // 内核实际占用页数
    auto kernel_pages = (COMMON::KERNEL_END_ADDR - COMMON::KERNEL_START_ADDR)
                      / COMMON::PAGE_SIZE;
    info("Kernel start: 0x%p, end 0x%p, size: 0x%X bytes, 0x%X pages.\n",
         COMMON::KERNEL_START_ADDR, COMMON::KERNEL_END_ADDR, kernel_size,
         kernel_pages);
    std::cout << "Simple Kernel." << std::endl;
    return;
}
