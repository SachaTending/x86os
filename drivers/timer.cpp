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
    registers_t regs;
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
extern "C" void perform_task_switch(uint32_t, uint32_t, uint32_t, uint32_t);
bool task_is_switching = false;
uint32_t esp_task_switch[32];
void task_switch() {
    if (current->esp == 0) {
        current->esp = (uint32_t)malloc(16384)+16384;
    }
    printf("esp: 0x%x\n", current->esp);
    task_is_switching = false;
    perform_task_switch(current->eip, 0, current->ebp, current->esp);
}
bool task_switch2 = false;
void timer_idt(registers_t *regs) {
    timer_tick++;
    if (schedule && task_is_switching == false) {
        task_t *prev = current;
        if (prev->next == current) return;
        current = prev->next;
        // Save registers
        if (task_switch2) {
            regs->eip = regs->eip;
            prev->esp = regs->esp;
            prev->ebp = regs->ebp;
        }
        else {
            task_switch2 = true;
            //printf("a\n");
        }
        // Load new registers
        //perform_task_switch(current->eip, current->ebp, current->esp);
        regs->eip = current->eip;
        regs->esp = current->esp;
        regs->ebp = current->ebp;
        //printf("eip: old: 0x%u new: 0x%u\n", prev->eip, current->eip);
        //printf("esp: old: 0x%x new: 0x%x\n", prev->eip, current->eip);
        //printf("name: old %s new %s\n", prev->name, current->name);
        //printf("regs: eip: 0x%x ebp: 0x%x esp: 0x%x\n", regs->eip, regs->ebp, regs->esp);
    }
}

void new_task(void(*task_func)(), const char *name) {
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    //new_task->esp = (uint32_t)malloc(16384);
    new_task->eip = (uint32_t)task_func;
    new_task->ebp = 0;
    new_task->name = name;
    new_task->next = root_task;
    task_t *t = root_task;
    while (t->next != root_task) {
        t = t->next;
    }
    t->next = new_task;
    next_pid++;
    tasks++;
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