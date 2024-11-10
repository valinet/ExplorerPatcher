ORG 100h                ; Origin for .COM file

; Set Video Mode 03h (Text Mode 80x25, 16 colors)
MOV AH, 0               ; Function to set video mode
MOV AL, 3               ; Mode 03h (80x25 text mode)
INT 10h                 ; Call BIOS interrupt

; Move the cursor to position (10, 10) (row 10, column 10)
MOV AH, 02h             ; Function to move cursor
MOV BH, 0               ; Page number (0 for active page)
MOV DH, 10              ; Row (line 10)
MOV DL, 10              ; Column (column 10)
INT 10h                 ; Call BIOS interrupt

; Display "Hello"
MOV AH, 09h             ; Function to display string
LEA DX, HelloMessage    ; Load address of "Hello" message
INT 21h                 ; DOS interrupt for printing string

; Exit program
MOV AH, 4Ch             ; DOS function to terminate program
INT 21h                 ; Call DOS interrupt

HelloMessage DB 'Hello$' ; Message to be printed
