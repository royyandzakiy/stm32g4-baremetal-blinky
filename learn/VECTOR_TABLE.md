## What is the Vector Table?

The vector table is the first thing the ARM Cortex-M4 processor looks at when it starts up. It's stored at the very beginning of flash memory (0x08000000) and contains **addresses** (pointers) of important functions.

## What Your Code Creates

```c
void (*const vector_table[16 + 91])(void) = {
    _estack,
    _reset,
};
```

This creates an **array of 107 function pointers** in a special section called `.vectors`. But only the first two entries are explicitly set. Here's what it means:

### Understanding the Declaration

```c
void (*const vector_table[107])(void)
```

This breaks down as:

- `void` - the functions return nothing
- `(*const vector_table[107])` - an array of 107 constant pointers to functions
- `(void)` - the functions take no arguments

### Why Put `_estack` and `_reset` Here?

**Entry 0: `_estack`**
This is NOT actually a function pointer - it's a trick! The ARM Cortex-M hardware expects the first entry to be the **initial stack pointer value**, not a function. So `_estack` is a symbol from your linker script that represents the top of RAM (0x20000000 + 128K = 0x20020000). The processor loads this value directly into the Stack Pointer register.

**Entry 1: `_reset`**
This IS a function pointer. When the processor finishes initializing, it reads this address and jumps to the `_reset` function, which is your startup code.

## How the Hardware Uses It

When power comes on or reset happens:

1. Processor reads the first 4 bytes from address 0x08000000
2. It puts that value (0x20020000) into the Stack Pointer (SP)
3. Processor reads the next 4 bytes from address 0x08000004
4. It sees the address of `_reset` function
5. It jumps to that address and starts executing `_reset`

## What About the Other 105 Entries?

They're implicitly set to zero (null pointers). The = { \_estack, \_reset } syntax means:

- First two entries get those values
- Everything else defaults to 0

If an interrupt fires (like a timer, UART, etc.) and the corresponding handler is 0, nothing bad happens because you're not enabling those interrupts yet.

## Visual Representation in Flash

```
Address 0x08000000: [0x20020000]  <- Stack pointer value (end of RAM)
Address 0x08000004: [0x08000XXX]  <- Address of _reset function
Address 0x08000008: [0x00000000]  <- NMI handler (unused)
Address 0x0800000C: [0x00000000]  <- HardFault handler (unused)
... (remaining 103 entries)
```

So `_estack` isn't a function pointer being misused - it's deliberately providing the stack address that the hardware needs, even though the C type says "function pointer." The actual value matters more than the type here.
