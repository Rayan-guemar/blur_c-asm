// arm64 macOS

.global _blur
.align 2

// blur(uint8_t* src, uint8_t* dst, int width, int height)
_blur:
    // Prologue
    stp     fp, lr, [sp, -16]!   // sauver fp/lr
    mov     fp, sp

    // Sauvegarde des paramètres dans des registres callee-saved
    mov     x22, x0      // src
    mov     x23, x1      // dst
    mov     x20, x2      // width
    mov     x21, x3      // height
    mov     x24, x0      // curent src position
    mov     x25, x1      // curent dst position

    // Calcul stride = width*3 + 1 (car filtre au début de chaque ligne)
    mov     x9, #3
    mul     x19, x20, x9 // width*3
    add     x19, x19, #1 // +1 pour l’octet filtre

    // Boucle Y
    mov     x10, #0      // y = 0
_y_loop:
    cmp     x10, x21
    bge     _y_loop_end

    ldr w11, [x24]
    str w11, [x25]

    add x24, x24, #1
    add x25, x25, #1

    // Boucle X
    mov     x12, #0      // x = 0
_x_loop:
    cmp     x12, x20
    bge     _x_loop_end

    // -----------------------

    mov x27, #0
_z_loop:
    cmp x27, #3
    bge _z_loop_end

    mov x13, #0
    mov x14, #0

    bl _add_3_pixel_sum_one_color

    cmp x10, #0
    ble _ignore_top
    
    sub x24, x24, x19
    bl _add_3_pixel_sum_one_color
    add x24, x24, x19
_ignore_top:
    sub x21, x21, #1
    cmp x10, x21
    add x21, x21, #1
    bge _ignore_bot
    
    add x24, x24, x19
    bl _add_3_pixel_sum_one_color
    sub x24, x24, x19
_ignore_bot:

    udiv x13, x13, x14
    strb w13, [x25]

    add x24, x24, #1
    add x25, x25, #1 
    // -----------------------

    add x27, x27, #1
    b _z_loop
_z_loop_end:

    add     x12, x12, #1
    b       _x_loop
_x_loop_end:

    add     x10, x10, #1
    b       _y_loop
_y_loop_end:

    // Épilogue
    mov     sp, fp
    ldp     fp, lr, [sp],  #16
    ret

_add_3_pixel_sum_one_color:

    ldrb w26, [x24]
    add x13, x13, x26
    add x14, x14, #1

    cbz x12, _ignore_left_pixel

    sub x15, x24, #3
    ldrb w26, [x15]
    add x13, x13, x26
    add x14, x14, #1
    _ignore_left_pixel:

    sub x15, x20, #1
    cmp x12, x15

    beq _ignore_right_pixel

    add x15, x24, #3
    ldrb w26, [x15]
    add x13, x13, x26
    add x14, x14, #1

_ignore_right_pixel:
    ret