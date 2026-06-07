# Finding the right Register

General Flow:

```text
Enable clock (RCC) → Configure mode (MODER) → Set output value (BSRR)
```

## General Memory Map

The general memory map is used to see the overall picture, and have a bigger overlook on where different peripherals registers are set at. Some MCUs might have slightly different mappings. It also shows where the flash will be at for the linker to set into

![alt text](imgs/image-3.png)
![alt text](imgs/image.png)
![alt text](imgs/image-1.png)
![alt text](imgs/image-2.png)

## GPIO

GPIO is the main peripheral to control the led output.

```text
Enable GPIO -> Set Mode (output) -> Set Output value (ON/OFF)
```

![alt text](imgs/image-9.png)
![alt text](imgs/image-10.png)
![alt text](imgs/image-7.png)
![alt text](imgs/image-8.png)

## RCC

RCC is needed to enable clock for peripherals so they can have a reference to work with

```text
Enable RCC for certain GPIO Port (Port A)
```

![alt text](imgs/image-4.png)
![alt text](imgs/image-5.png)
![alt text](imgs/image-6.png)