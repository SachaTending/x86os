#include <idt.hpp>
#include <io.h>
#include <timer.hpp>
#include <libc.hpp>
#include <malloc.hpp>

typedef struct task {
    int pid;
    const char *name;
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    task *next;
} task_t;

task_t *current, *root_task;
bool schedule = false;
int timer_tick = 0;
int next_pid = 0;
int tasks = 0;

void task1() {
    while (1) asm volatile ("hlt");
}

void task2() {
    while (1) printf("task2\n");
}
void task3() {
    while (1) printf("task3\n");
}

void timer_idt(registers_t *regs) {
    timer_tick++;
    if (schedule && timer_tick % 15) {
        task_t *prev = current;
        current = prev->next;
        // Save registers
        prev->eip = regs->eip;
        prev->esp = regs->esp;
        prev->ebp = regs->ebp;
        // Load new registers
        regs->eip = current->eip;
        regs->ebp = current->ebp;
        regs->esp = current->esp;
    }
}

void new_task(void(*task_func)(), const char *name) {
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    new_task->esp = (uint32_t)malloc(16384);
    new_task->eip = (uint32_t)task_func;
    new_task->ebp = 0;
    new_task->name = name;
    new_task->next = root_task;
    task_t *t = root_task;
    for (int i=0;i<tasks;i++) {
        t = t->next;
    }
    t->next = new_task;
}

void sched_start() {
    // Create idle task
    current = (task_t *)malloc(sizeof(task_t));
    current->esp = (uint32_t)malloc(16384);
    current->eip = (uint32_t)task1;
    current->ebp = 0;
    current->name = "Idle";
    current->pid = next_pid;
    current->next = current;
    root_task = current;
    next_pid++;
    tasks++;
    // Create next task
    new_task(task2, "Dumb");
    new_task(task3, "Dumb2");
    schedule = true;
}

void Timer::Init() {
    IDT::AddHandler(0, timer_idt);
    int divisor = 1193180 / 10;
    outb(0x43, 0x36);
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}