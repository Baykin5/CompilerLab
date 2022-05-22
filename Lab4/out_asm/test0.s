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

main:
  subu $sp, $sp, 20
  sw $ra, 16($sp)
  sw $fp, 12($sp)
  addi $fp, $sp, 20
  li $t0, 1
  li $t8, 0
  sw $t0, -12($fp)
  bgt $t0,$t8,label1
  j label2
label1:
  li $t0, 2
  sw $t0, -16($fp)
  j label3
label2:
  li $t0, 3
  sw $t0, -16($fp)
label3:
  lw $t0, -16($fp)
  move $a0, $t0
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  sw $t0, -16($fp)
  li $t0, 0
  lw $ra, 16($sp)
  lw $fp, 12($sp)
  addi $sp, $sp, 20
  move $v0, $t0
  jr, $ra
  sw $t0, -20($fp)
