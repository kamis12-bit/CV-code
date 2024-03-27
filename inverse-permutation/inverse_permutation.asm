global inverse_permutation

;Accepts as arguments (size_t n) -> rdi and (int * p) -> rsi
;Returns bool: true if all data correct and permutation reversed, false otherwise (if false, array unchanged)
;Uses registers: rcx, rdx, r8

section .rodata

INT_MAX equ 0x7fffffff      ;for checking if n is in proper range
POS_BITMASK equ 0x7fffffff  ;for making first bit 0 with 'and'
NEG_BITMASK equ 0x80000000  ;for making first bit 1 with 'xor'

section .text

inverse_permutation:
;Checking if n is correct
    test rdi, rdi       ;if 0
    jz   .return_false
    cmp  rdi, INT_MAX   ;if greater than INT_MAX
    ja   .return_false

;Checking in a loop if numbers are in proper range [0..n-1] and form a permutation
;The values we keep:
;   ecx: currently considered index in array
;   edx: number on index [ecx]
    mov ecx, edi                        ;we start with n in ecx
.checking_loop:
    dec rcx                             ;decreasing index, so we start with n-1
    mov edx, [rsi+rcx*4]                ;moving the number from ecx position into edx
    and edx, POS_BITMASK                ;making positive (setting first bit to 0)
    cmp edx, edi                        ;checking if edx < n
    jnb .rollback                       ;   if not (Not Below), jump to rollback, return false
                                        ;   if yes, is in proper range and can be used as index
    cmp   dword [rsi+rdx*4], 0          ;check if [rsi+rdx*4] is negative
    jl    .rollback                     ;   if yes (Less), jump to rollback, return false 
                                        ;       (meaning rdx repeats in array, or a[rdx] was already negative)
    xor   dword [rsi+rdx*4], NEG_BITMASK;   if no, flip its negative-bit to 1
    jrcxz .inversing                    ;if we hit 0, jump to inversing

    jmp .checking_loop                  ;looping (while ecx != 0)

;Inversing we start with all numbers having oldest bit set
.inversing:
    mov rcx, rdi                        ;starting with index = n
.inv_loop:
    jrcxz .return_true                  ;if we passed the whole array we return true; if we jump we already did the loop for index 0
    dec   rcx                           ;decreasing index so we start with n-1
    cmp   dword [rsi+rcx*4], 0          ;checking if the number at index rcx is negative
    jge   .inv_loop                     ;   if no (Greater Equal), this number is already in its proper place; go to next
                                        ;   if yes, we start reversing the cycle
    mov edx, [rsi+rcx*4]                ;taking number at index rcx
    and edx, POS_BITMASK                ;making the first bit 0, so we can use the number at rdx as index

.cycle_reverse:
    cmp dword [rsi+rdx*4], 0            ;checking if next number in cycle is nonnegative we have finished reversing the cycle
    jge .inv_loop                       ;   if yes, we have finished reversing the cycle, and rcx is as it was at the beginning
    mov r8, [rsi+rdx*4]                 ;storing the value at index rdx, so we can use it later
    and r8, POS_BITMASK                 ;zeroing the oldest bit
    mov [rsi+rdx*4], ecx                ;this is the proper 'inversing' step: at this point, since p[rcx]=rdx, now p[rdx]=rcx
    mov rcx, rdx                        ;moving over in the cycle: now, what was the value of permutation function is its argument
    mov rdx, r8                         ;in r8 is what was before the value of p[rdx]; now rcx-rdx is again an index-value pair
    jmp .cycle_reverse                  ;continuing the reversal

    

.return_true:
    mov rax, 0x1            ;setting result true
    ret

.rollback:                  ;reversing the changes made during checking
    inc rcx                             ;increasing index
    cmp ecx, edi                        ;checking if we're not done yet
    jnb .return_false                   ;   if we are (Not Below), return false
                                        ;   if we are not, we can use rcx as index
    mov edx, [rsi+rcx*4]                ;moving the number from ecx position into edx
    and edx, POS_BITMASK                ;making positive (setting first bit to 0)
    xor dword [rsi+rdx*4], NEG_BITMASK  ;fliping negative-bit to 0
    jmp .rollback
    
.return_false:
    xor eax, eax            ;setting result false: operation on eax zeroes older 32 bits of rax
    ret


