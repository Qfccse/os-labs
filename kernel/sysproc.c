#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

/*
sysproc.c

修改sys_sbrk()。因为内存增加时需要复制到内核页表，内存减少需要解除映射。
这里把55行int改为uint64可能会陷入无限循环，出现panic: uvmunmap: walk，某个测试点过不了.
*/
uint64
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc *p=myproc();
  if(argint(0, &n) < 0)
    return -1;
  addr = p->sz;
  if(growproc(n) < 0)
    return -1;
  if(n>0){
    vmcopypage(p->pagetable,p->kpagetable,addr,n);
  }else{
      for(int j=addr-PGSIZE;j>=addr+n;j-=PGSIZE){
          uvmunmap(p->kpagetable,j,1,0);
      }
  }
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
