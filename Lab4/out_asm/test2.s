.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

fact:
  subu $sp, $sp, 28
  sw $ra, 24($sp)
  sw $fp, 20($sp)
  addi $fp, $sp, 28
  li $t8, 1
  sw $a0, -12($fp)
  beq $a0,$t8,label1
  j label2
label1:
  lw $t1, -12($fp)
  move $t0,$t1
  lw $ra, 24($sp)
  lw $fp, 20($sp)
  addi $sp, $sp, 28
  move $v0, $t0
  jr, $ra
  sw $t1, -12($fp)
  sw $t0, -16($fp)
  j label3
label2:
  lw $t0, -12($fp)
  addi $t1, $t0, -1
  move $a0, $t1
  sw $t0, -12($fp)
  sw $t1, -20($fp)
  jal fact
  addi $sp, $sp, 0
  move $t0, $v0
  sw $t0, -24($fp)
  lw $t0, -12($fp)
  lw $t1, -24($fp)
  mul $t2, $t0, $t1
  lw $ra, 24($sp)
  lw $fp, 20($sp)
  addi $sp, $sp, 28
  move $v0, $t2
  jr, $ra
  sw $t0, -12($fp)
  sw $t1, -24($fp)
  sw $t2, -28($fp)
label3:
main:
  subu $sp, $sp, 28
  sw $ra, 24($sp)
  sw $fp, 20($sp)
  addi $fp, $sp, 28
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t0, $v0
  sw $t0, -12($fp)
  lw $t1, -12($fp)
  move $t0,$t1
  li $t8, 1
  sw $t1, -12($fp)
  sw $t0, -16($fp)
  bgt $t0,$t8,label4
  j label5
label4:
  lw $t0, -16($fp)
  move $a0, $t0
  sw $t0, -16($fp)
  jal fact
  addi $sp, $sp, 0
  move $t0, $v0
  sw $t0, -20($fp)
  lw $t1, -20($fp)
  move $t0,$t1
  sw $t0, -24($fp)
  sw $t1, -20($fp)
  j label6
label5:
  li $t0, 1
  sw $t0, -24($fp)
label6:
  lw $t0, -24($fp)
  move $a0, $t0
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  sw $t0, -24($fp)
  li $t0, 0
  lw $ra, 24($sp)
  lw $fp, 20($sp)
  addi $sp, $sp, 28
  move $v0, $t0
  jr, $ra
  sw $t0, -28($fp)
