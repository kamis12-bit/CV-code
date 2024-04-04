; Implementation of a distributed stack machine
; Each machine executing below instructions is called a core

; Arguments:
;   rdi: n - core id (constant)
;   rsi: p - pointer to string CALC (variable) containing the instructions for the core
; End condition: end of string met: '\0'
; Result: value from the top of stack

; Operations (chars in CALC):
;   +   pop two numbers, push their sum
;   *   pop two numbers, push their product
;   -   negate arithmetically number from the top
;  0-9  push 0-9 respectivly
;   n   push core id
;   B   pop value v; if top is nonzero, move pointer 2comp(v) operations
;   C   pop value & discard it
;   D   push value of the top (duplicate top)
;   E   switch places of two top values
;   G   push get_value(n)
;   P   pop value v & call put_value(n, v)
;   S   synchronise: pop value m; wait for operation S of core m with popped value n; switch tops of stacks n and m

; Assumptions: (A)
;   1. All operations are on 64-bit numbers mod 2^64.
;   2. Core id n is correct.
;   3. CALC contains only above characters.
;   4. CALC terminates in '\0'.
;   5. No operation tries to reach top of an empty stack.
;   6. CALC doesn't produce a deadlock.
;   7. The behaviour of the core for an incorrect CALC is undefined.



global core
extern get_value
extern put_value

; N is the overall number of cores - set at compilation
SECTION .data
    exchange_array: times 2*N dq N ; the number in cell n is that of a core which can access value; if N, it is being accessed
    ; First N numbers are addresses - the number is the index of the only core that can access the value; N means someone is accessing it now.
    ; Last  N numbers are values    - the number a core puts up to be exchanged.
    ; Both are in one array - we only need to lea the pointer to array once.
    ; The numbers are initialised as N, so the core is assured to have access to his data as first.

SECTION .text
    ; The general use of registers:
    ;   rdi: n - the index of the core
    ;   rsi: p - the pointer to the instruction string CALC
    ;   rax: the character of the current instruction or helper register for value pushed/popped from stack
    ;   rbx: the address of exchange_array
    ;   rcx: the exchanged value in .synchronise
    ;   rdx: the top of the stack
    ;   rbp: the address to which the stack pointer should be restored at the end of the program
    ;   rsp: the stack pointer
    ;   r8 : helper for .synchronise
core:
    push rbp
    push rbx                        ; Storing rbp and rbx, so they can be restored as the ABI demmands
    mov  rbp, rsp                   ; Current, base position of the stack pointer
    lea  rbx, [rel exchange_array]  ; The address of the exchange_array
    mov  [rbx+8*rdi], rdi           ; Setting starting address in our cell to our number, so we can access it at beginning of .synchronise
    jmp  .step                      ; For the first step, the instruction pointer is already in the right position

.inc_ptr:
    inc rsi

.step:
    movzx rax, byte [rsi]           ; Loading character from CALC
    pop rdx                         ; Most operations require popping from stack, so might as well do it here

    ; Straightforward switch-casing for detecting the type of instruction
    cmp al, '+'
    je  .sum

    cmp al, '*'
    je  .product

    cmp al, 'B'
    je  .shift

    cmp al, 'C'
    je  .inc_ptr                    ; The top is popped already, and that's enough

    cmp al, 'E'
    je  .switch

    cmp al, 'P' 
    je  .put_value

    cmp al, 'S'
    je  .synchronise

    push rdx                        ; The instructions below don't need the top to be popped, so it's pushed again

    cmp al, 'D'
    je  .dupl

    cmp al, '-' 
    je  .neg

    cmp al, 'n' 
    je  .push_n

    cmp al, 'G' 
    je  .get_value
    
    cmp al, '0' 
    jb  .return                     ; Meaning char is not one of the above or a number, so it is interpreted as '\0'
    cmp al, '9'
    jna .push_number


.return:
    pop rax         ; At this point we know that top of stack has been popped and pushed again, so it's correct
    mov rsp, rbp    ; Restoring the proper position of stack pointer
    pop rbx         ; Restoring rbx and rbp in the proper order
    pop rbp
    ret

.sum:
    add [rsp], rdx
    jmp .inc_ptr


.product:
    pop  rax
    imul rdx, rax
    push rdx
    jmp  .inc_ptr

.neg:
    neg qword [rsp]     ; Directly negating top of the stack
    jmp .inc_ptr

.shift:
    pop  rax
    test rax, rax       ; Testing if top of stack is zero
    push rax            ; Push doesn't affect any flags
    jz   .inc_ptr
    add  rsi, rdx       ; If stacktop nonzero, CALC pointer shifted properly
    jmp  .inc_ptr


.switch:
    pop  rax
    push rdx
    push rax
    jmp  .inc_ptr

.dupl:
    push rdx            ; We now that rdx already has the top of the stack, so we only need to push it again
    jmp  .inc_ptr

.push_n:
    push rdi            ; rdi stores n
    jmp  .inc_ptr

.push_number:
    sub  rax, '0'       ; Making rax store actual number, not the character
    push rax
    jmp  .inc_ptr


.get_value:
    push rdi
    push rsi                    ; Storing the values we want to preserve. ABI ensures rbp, rbx, rsp will not be altered.
    test spl, 0xf               ; Testing if spl is misaligned for the function call
    jnz .align_and_call_get     ; If it is, jump to version with alignment
    call get_value wrt ..plt    ; wrt ..plt allows relocation
    jmp .restore_regs_after_get 
.align_and_call_get:
    sub rsp, 8
    call get_value wrt ..plt
    add rsp, 8
.restore_regs_after_get:
    pop rsi                     ; Restoring values where they belong
    pop rdi
    push rax                    ; Pushing the result of the get_value call
    jmp .inc_ptr
    
; .put_value works like .get_value. It doesn't push result, since it doesn't have any, but places argument properly
.put_value:
    push rdi
    push rsi
    mov rsi, rdx                    ; Placing the second argument. The first one is already n, like it should be.
    test spl, 0xf
    jnz .align_and_call_put
    call put_value wrt ..plt
    jmp .restore_regs_after_put
.align_and_call_put:
    sub rsp, 8
    call put_value wrt ..plt
    add rsp, 8
.restore_regs_after_put:
    pop rsi
    pop rdi
    jmp .inc_ptr

; Basic idea: we first give up x at our address, then take y from m address.
; We can only access, if our id was put there. Both times we give access to the other core.
; Therefore if we have access to the m address, we know that the number there is intended for us by the m core.
.synchronise:                       ; rdx stores m - the number of the core we want to exchange with. 
    pop rcx                         ; Popping the value to be exchanged.
    mov r10, N
    mov r8, rdi                     ; r8 stores the number of the cell we want to access - n at beginning
.check:
    mov rax, rdi                    ; At beginning of each loop rax is primed with n
    lock cmpxchg [rbx+8*r8], r10    ; Requesting access to r8 cell; if gotten, atomically set to N
    jnz .check                      ; Checking if access received. If not, continue checking. 
    xchg [rbx+8*r8+8*N], rcx        ; If yes, critical section: exchange the value with rcx. At first loop, putting x, at second, getting y
    mov [rbx+8*r8], rdx             ; Handing the cell over to m, thereby ending critical section.
    cmp r8, rdx                     ; Checking if we were accessing our cell or m cell.
    mov r8, rdx                     ; mov doesn't affect flags. Ensures we execute exactly two loops (since 'nS' is undefined).
    jne .check                      ; This jump executes exactly once. First loop is accessing our cell, second - m cell.
    push rcx                        ; Pushing y, the value gotten from the exchange, to stack.
    jmp .inc_ptr                    ; Exchange ends with cells handed back over to their owners, like at beginning ([rbx+8*n] = n).


