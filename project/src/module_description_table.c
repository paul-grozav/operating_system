// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
#include "module_serial.h"
// -------------------------------------------------------------------------- //
void memset(void *start, const char value, const size_t length)
{
  char *buffer = (char*)start;
  for(size_t i=0; i<length; i++)
  {
    buffer[i] = value;
  }
}
// -------------------------------------------------------------------------- //
void debug(char *text)
{
  module_terminal_vga t = module_terminal_vga_create();
  module_terminal_init(&t);
  module_terminal_print_c_string(text, &t);
  asm volatile ("hlt"); // halt cpu
}
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// GDT
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
}__attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));

void init_gdt();

extern void gdt_flush(uint32_t);
static void gdt_set_gate(int32_t,uint32_t,uint32_t,uint8_t,uint8_t);

struct gdt_entry gdt_entries[5];
struct gdt_ptr gdt_ptr;

void init_gdt() {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;
    gdt_set_gate(0,0,0,0,0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdt_flush((uint32_t)&gdt_ptr);
}

static void gdt_set_gate(int32_t num,uint32_t base,uint32_t limit,
uint8_t access,uint8_t granularity) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= granularity & 0xF0;
    gdt_entries[num].access = access;
}
// IDT
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always_zero;
    uint8_t flags;
    uint16_t base_high;
}__attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));

struct idt_ptr idt_ptr;
struct idt_entry idt_entries[256];
extern void idt_flush(uint32_t);
static void idt_set_gate(uint8_t,uint32_t,uint16_t,uint8_t);
void init_idt();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt_entries;
    memset(&idt_entries,0,sizeof(struct idt_entry) * 256);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(0,(uint32_t)isr0,0x08,0x8E);
    idt_set_gate(1,(uint32_t)isr1,0x08,0x8E);
    idt_set_gate(2,(uint32_t)isr2,0x08,0x8E);
    idt_set_gate(3,(uint32_t)isr3,0x08,0x8E);
    idt_set_gate(4,(uint32_t)isr4,0x08,0x8E);
    idt_set_gate(5,(uint32_t)isr5,0x08,0x8E);
    idt_set_gate(6,(uint32_t)isr6,0x08,0x8E);
    idt_set_gate(7,(uint32_t)isr7,0x08,0x8E);
    idt_set_gate(8,(uint32_t)isr8,0x08,0x8E);
    idt_set_gate(9,(uint32_t)isr9,0x08,0x8E);
    idt_set_gate(10,(uint32_t)isr10,0x08,0x8E);
    idt_set_gate(11,(uint32_t)isr11,0x08,0x8E);
    idt_set_gate(12,(uint32_t)isr12,0x08,0x8E);
    idt_set_gate(13,(uint32_t)isr13,0x08,0x8E);
    idt_set_gate(14,(uint32_t)isr14,0x08,0x8E);
    idt_set_gate(15,(uint32_t)isr15,0x08,0x8E);
    idt_set_gate(16,(uint32_t)isr16,0x08,0x8E);
    idt_set_gate(17,(uint32_t)isr17,0x08,0x8E);
    idt_set_gate(18,(uint32_t)isr18,0x08,0x8E);
    idt_set_gate(19,(uint32_t)isr19,0x08,0x8E);
    idt_set_gate(20,(uint32_t)isr20,0x08,0x8E);
    idt_set_gate(21,(uint32_t)isr21,0x08,0x8E);
    idt_set_gate(22,(uint32_t)isr22,0x08,0x8E);
    idt_set_gate(23,(uint32_t)isr23,0x08,0x8E);
    idt_set_gate(24,(uint32_t)isr24,0x08,0x8E);
    idt_set_gate(25,(uint32_t)isr25,0x08,0x8E);
    idt_set_gate(26,(uint32_t)isr26,0x08,0x8E);
    idt_set_gate(27,(uint32_t)isr27,0x08,0x8E);
    idt_set_gate(28,(uint32_t)isr28,0x08,0x8E);
    idt_set_gate(29,(uint32_t)isr29,0x08,0x8E);
    idt_set_gate(30,(uint32_t)isr30,0x08,0x8E);
    idt_set_gate(31,(uint32_t)isr31,0x08,0x8E);
    idt_set_gate(32,(uint32_t)irq0,0x08,0x8E);
    idt_set_gate(33,(uint32_t)irq1,0x08,0x8E);
    idt_set_gate(34,(uint32_t)irq2,0x08,0x8E);
    idt_set_gate(35,(uint32_t)irq3,0x08,0x8E);
    idt_set_gate(36,(uint32_t)irq4,0x08,0x8E);
    idt_set_gate(37,(uint32_t)irq5,0x08,0x8E);
    idt_set_gate(38,(uint32_t)irq6,0x08,0x8E);
    idt_set_gate(39,(uint32_t)irq7,0x08,0x8E);
    idt_set_gate(40,(uint32_t)irq8,0x08,0x8E);
    idt_set_gate(41,(uint32_t)irq9,0x08,0x8E);
    idt_set_gate(42,(uint32_t)irq10,0x08,0x8E);
    idt_set_gate(43,(uint32_t)irq11,0x08,0x8E);
    idt_set_gate(44,(uint32_t)irq12,0x08,0x8E);
    idt_set_gate(45,(uint32_t)irq13,0x08,0x8E);
    idt_set_gate(46,(uint32_t)irq14,0x08,0x8E);
    idt_set_gate(47,(uint32_t)irq15,0x08,0x8E);


    idt_flush((uint32_t)&idt_ptr);
}

 static void idt_set_gate(uint8_t num,uint32_t base,uint16_t selector,uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].always_zero = 0;
    idt_entries[num].flags = flags | 0x60;
}
// -------------------------------------------------------------------------- //
// INTERRUPTS
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

struct registers {
    uint32_t ds;
    uint32_t edi,esi,ebp,esp,ebx,edx,ecx,eax;
    uint32_t int_no,err_code;
};

typedef void (*isr_t)(registers_t);
void register_interrupt_handler(uint8_t n, isr_t handler);

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n,isr_t handler) {
	interrupt_handlers[n] = handler;
}

void isr_handler(struct registers regs) {
  module_terminal_vga t = module_terminal_vga_create();
  module_terminal_init(&t);
  module_terminal_print_c_string("isr_handler: Received interrupt:", &t);
  if( regs.int_no == 65536 )
  {
      module_terminal_print_c_string("GOT BIG INT\n", &t);
  }
  module_terminal_print_uint64(regs.int_no, &t);
  module_terminal_print_char('\n', &t);
//    WriteScreen("received interrupt:");
//    monitor_write_dec(regs.int_no);
//    WriteScreen("\n");
}

void irq_handler(struct registers regs) {
  module_terminal_vga t = module_terminal_vga_create();
  module_terminal_init(&t);
  module_terminal_print_c_string("irq_handler: Received interrupt:", &t);
  module_terminal_print_uint64(regs.int_no, &t);
  module_terminal_print_char('\n', &t);
  asm volatile ("hlt"); // halt cpu

	if (regs.int_no >= 40) {
		outb(0xA0,0x20);
	}
	outb(0x20,0x20);
	if (interrupt_handlers[regs.int_no] != 0 ) {
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}
// -------------------------------------------------------------------------- //
void module_interrupts_test()
{
  init_gdt();
  init_idt();
  asm volatile ("int $0x10"); // call interrupt 16(10 in hex)

//  debug("module_interrupts_test");
}
// -------------------------------------------------------------------------- //

