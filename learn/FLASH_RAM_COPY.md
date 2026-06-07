## The Two-Phase Flow

### Phase 1: Programming (When You Flash the Board)

```
Your C code:  int counter = 42;
                    ↓ Compiler
            Puts "counter" in .data section
            Puts value 42 as initial data
                    ↓ Linker
            Stores section layout in ELF file:
            - Code at 0x08000000 (Flash)
            - Initial value 42 at 0x08001000 (Flash)
            - But will LIVE at 0x20000100 (RAM)
                    ↓ flasher tool
            Writes everything to physical Flash memory

Board Flash now contains:
┌────────────────┐
│ .vectors       │
│ .text (code)   │
│ .rodata        │
│ 42 (counter)   │ ← 42 stored here permanently
└────────────────┘
```

At this point, RAM is empty/garbage. Power is off. The 42 only exists in Flash.

### Phase 2: Every Power-On/Reset

```
Power On
    ↓
1. Hardware reads vector table
   - Gets stack pointer (_estack)
   - Gets reset handler address (_reset)
    ↓
2. Jumps to _reset function
    ↓
3. _reset copies .data from Flash → RAM
   - Reads 42 from Flash (at _sidata)
   - Writes 42 to RAM (at _sdata)
    ↓
4. _reset zeros .bss in RAM
    ↓
5. Calls main()
    ↓
6. Your code runs, counter = 42 ✓
```

## Why This Two-Step Dance?

**You can't modify Flash easily at runtime.** If `counter` was only in Flash:

```c
counter = 43;  // This would FAIL or crash!
```

So the value must be in RAM. But **RAM loses everything when power is off.** If there was no copy from Flash:

```c
int counter = 42;  // After power-on, counter = random garbage!
```

## The Key Insight

**The initial value lives in Flash permanently.** Every time the board powers on, the startup code copies that initial value from Flash to RAM, like resetting a save file to the beginning of a game.

```c
// This happens EVERY reset:
counter starts as 42 (copied from Flash)
counter becomes 43 (your code modifies it)
counter becomes 100
// ... power off ...
// ... power on ...
counter starts as 42 again (fresh copy from Flash)
```

## .bss Is Different

```c
int uninitialized_var;  // No initial value to store!
```

Since the C standard says this must be 0, and storing zeros in Flash would waste space, `.bss` just gets zeroed in RAM during startup. No Flash copy needed.

So the complete power-on sequence for your variables:

```
Flash:         "42"          (nothing for bss)
                ↓ copy
RAM:     counter = 42    uninit = 0 (zeroed)
```
