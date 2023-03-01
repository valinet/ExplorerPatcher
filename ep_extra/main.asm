EXTERN worker : PROC

.CODE

ep_extra_EntryPoint PROC EXPORT
  PUSH RBP
  MOV RBP, RSP
  SUB RSP, 30H
  CALL worker
  CMP RAX, 0
  JE finish
  JMP RAX
finish:
  LEAVE
  RET
ep_extra_EntryPoint ENDP

END