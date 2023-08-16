#include <idt.hpp>
#include <io.h>
#include <timer.hpp>
#include <libc.hpp>
#include <malloc.hpp>
#include <stdint.h>

enum TASK_STATE {
    TASK_CREATED,
    TASK_RUNNING
};

typedef struct task {
    int pid;
    const char *name;
    uint32_t eip;
    uint32_t esp;
    uint32_t entry;
    task *next;
    registers_t regs;
    TASK_STATE state;
} task_t;

task_t *current, *root_task;
bool schedule2 = false;
int timer_tick = 0;
int next_pid = 0;
int tasks = 0;

void task1() {
    while (1) {
        asm volatile ("cli");
    }
}

void task2() {
    while (1); //printf("task2\n");
}
void task3() {
    while (1); //printf("task3\n");
}
extern "C" void perform_task_switch(uint32_t, uint32_t, uint32_t, uint32_t);
extern "C" void usermode_entry();
bool task_is_switching = false;

bool task_switch2 = false;
void schedule_timer(registers_t *regs);
void timer_idt(registers_t *regs) {
    //printf("triggered\n");
    timer_tick++;
    if (schedule2) {
        IDT::AddHandler(0, schedule_timer);
    } else {
        //printf("no multitasking(\n");
    }
}

void schedule_timer(registers_t *regs) {
    timer_tick++;
    task_t *prev = current;
    if (prev->next == current) return;
    current = prev->next;
    // Save registers
    if (prev->state != TASK_CREATED) memcpy(&prev->regs, regs, sizeof(registers_t));
    // Load new registers
    if (current->state == TASK_CREATED) {
        regs->eip = current->eip;
        regs->esp = current->esp;
        current->state = TASK_RUNNING;
        //printf("%s->%s\n", prev->name, current->name);
    } else {
        memcpy(regs, &current->regs, sizeof(registers_t));
    }
    //perform_task_switch(current->eip, current->ebp, current->esp);
    //printf("eip: old: 0x%u new: 0x%u\n", prev->eip, current->eip);
    //printf("esp: old: 0x%x new: 0x%x\n", prev->eip, current->eip);
    //printf("name: old %s new %s\n", prev->name, current->name);
    //printf("%s->%s\n", prev->name, current->name);
    //printf("regs: eip: 0x%x ebp: 0x%x esp: 0x%x\n", regs->eip, regs->ebp, regs->esp);
}

void new_task(void(*task_func)(), const char *name) {
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    new_task->esp = (uint32_t)malloc(16384)+16384;
    new_task->eip = (uint32_t)task_func;
    new_task->state = TASK_CREATED;
    new_task->name = name;
    new_task->next = root_task;
    new_task->entry = (uint32_t)task_func;
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
    current->esp = (uint32_t)malloc(16384)+16384;
    //current->eip = (uint32_t)task1;
    current->eip = (uint32_t)task2;
    current->entry = (uint32_t)task1;
    current->state = TASK_CREATED;
    current->name = "Idle";
    current->pid = next_pid;
    current->next = current;
    root_task = current;
    next_pid++;
    tasks++;
    // Create next task
    new_task(task2, "Dumb");
    new_task(task3, "Dumb2");
    schedule2 = true;
}

void Timer::Init() {
    IDT::AddHandler(0, timer_idt);
    int divisor = 1193180 / 100;
    outb(0x43, 0x36);
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}