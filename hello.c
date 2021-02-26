
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm_types.h>
#include <asm/io.h>
#include <linux/highmem.h>
#include <linux/pid.h>

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/mm.h>
//---------------------------------
//#include <asm/pgtable_types.h>
//#include <asm/pgtable.h>
//#include <asm/paravirt_types.h>
//--------------------------------

/*

void find_page_table(struct task_struct *t)
{
    struct mm_struct *mm = t->mm;
    unsigned long address = 0xc0400000;
    
    printk("******* 3G~4G virtual address trans *******");
    while (address < 0xc0408000) {
        printk("[-] Virtual addr : 0x%x\n", address);
        v2p(&init_mm, address, 1);
        v2p(mm, address, 0);
        address += 0x4000;
        printk("-------------------");
    }
    address = 0x8048000;
    printk("******* 0~3G virtual address trans *******");
    
    while (address < 0x8050000) {
        printk("[-] Virtual addr : 0x%x\n", address);
        v2p(&init_mm, address, 1);
        v2p(mm, address, 0);
        address += 0x4000;
        printk("-------------------");
    }

}

asmlinkage long sys_hello(int u_pid)
{
    struct pid *p;
    struct task_struct *t;
    
    printk("PID : %d \n", u_pid);
    p = find_get_pid(u_pid);
    t = pid_task(p, PIDTYPE_PID);
    printk("task->pid : %d \n", t->pid);
    
    find_page_table(t);
    return 0;
                                    
}

*/

static unsigned long vaddr2paddr(struct mm_struct *mm, unsigned long vaddr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
	unsigned long page_addr = 0;
    unsigned long page_offset = 0;

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd)) {
        return -1;
    }

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d)){
		return -1;
	}

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud)) {
        return -1;
    }

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd)) {
        return -1;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    if (pte_none(*pte)) {
        return -1;
    }

    /* Page frame physical address mechanism | offset */
    page_addr = pte_val(*pte) & PAGE_MASK;
    page_offset = vaddr & ~PAGE_MASK;
    paddr = page_addr | page_offset;
    //printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
    printk("vaddr = 0x%lx, paddr = 0x%lx\n", vaddr, paddr);

    return paddr;
}

asmlinkage long sys_hello(void)
{

    printk("PID \t PNAME \t SIZE \n");
    struct task_struct *task = current;
    struct mm_struct *mm;
    struct mm_struct *curr_mm;
    struct vm_area_struct *vma;
    mm = get_task_mm(task);
    if (mm != NULL)
    {
        unsigned long address = mm->start_code;//0x08048000;//0x00000000;
        printk("%d \t %s \t %ld \n", task->pid, task->comm, mm->total_vm);
         
        printk("task:0x%lx", mm->start_code);
        printk("==== User space ====");
        while (address < mm->start_code + 4 * PAGE_SIZE) {
            vaddr2paddr(mm, address);
            address += PAGE_SIZE;
        }
        address = 0xffffffff81000000;
        printk("==== Kernel space ====");
        while (address < 0xffffffff81004000 ) {
            vaddr2paddr(mm, address);
            address += PAGE_SIZE;
        }
        printk("----------------------");
    }
   

    return 0;
}
