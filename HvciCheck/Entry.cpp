#include <Windows.h>
#include <winternl.h>
#include <iostream>

#pragma comment(lib, "ntdll.lib")

extern "C" NTSTATUS NTAPI NtQuerySystemInformationEx(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

struct SYSTEM_ISOLATED_USER_MODE_INFORMATION_INPUT
{
    ULONG Version;
    ULONG Spare;
};

struct SYSTEM_ISOLATED_USER_MODE_INFORMATION
{
    BOOLEAN SecureKernelRunning : 1;
    BOOLEAN HvciEnabled : 1;
    BOOLEAN HvciStrictMode : 1;
    BOOLEAN DebugEnabled : 1;
    BOOLEAN FirmwarePageProtection : 1;
    BOOLEAN EncryptionKeyAvailable : 1;
    BOOLEAN SpareFlags : 2;
    BOOLEAN TrustletRunning : 1;
    BOOLEAN HvciDisableAllowed : 1;
    BOOLEAN HardwareEnforcedVbs : 1;
    BOOLEAN NoSecrets : 1;
    BOOLEAN EncryptionKeyPersistent : 1;
    BOOLEAN HardwareEnforcedHvpt : 1;
    BOOLEAN HardwareHvptAvailable : 1;
    BOOLEAN SpareFlags2 : 1;
    BOOLEAN EncryptionKeyTpmBound : 1;
    BOOLEAN Spare0[5];
    ULONGLONG Spare1;
};

struct HvciState
{
    bool Enabled{};
    bool AuditMode{};
    bool StrictMode{};
    bool IsolatedUserMode{};
    bool VbsHardwareEnforced{};
    bool SecureKernelRunning{};
};

[[nodiscard]]
HvciState QueryHvciState() noexcept
{
    constexpr auto SystemIsolatedUserModeInformation = static_cast<SYSTEM_INFORMATION_CLASS>(165);

    SYSTEM_ISOLATED_USER_MODE_INFORMATION Information{};
    SYSTEM_ISOLATED_USER_MODE_INFORMATION_INPUT Input{ .Version = 1 };

    NTSTATUS Status = NtQuerySystemInformationEx(
        SystemIsolatedUserModeInformation,
        &Input,
        sizeof(Input),
        &Information,
        sizeof(Information),
        nullptr
    );

    if (!NT_SUCCESS(Status))
    {
        return {};
    }

    return
    {
        .Enabled = static_cast<bool>(Information.HvciEnabled),
        // Audit mode is not directly exposed in this struct, unlike in SystemCodeIntegrityInformation
        .AuditMode = false,
        .StrictMode = static_cast<bool>(Information.HvciStrictMode),
        .IsolatedUserMode = static_cast<bool>(Information.TrustletRunning),
        .VbsHardwareEnforced = static_cast<bool>(Information.HardwareEnforcedVbs),
        .SecureKernelRunning = static_cast<bool>(Information.SecureKernelRunning),
    };
}

int main()
{
    const auto Hvci = QueryHvciState();

    std::cout
        << "HVCI state (NtQuerySystemInformationEx):\n"
        << "HVCI enabled:          " << std::boolalpha << Hvci.Enabled << '\n'
        //<< "HVCI audit mode:       " << Hvci.AuditMode << '\n'
        << "HVCI strict mode:      " << Hvci.StrictMode << '\n'
        << "Isolated user mode:    " << Hvci.IsolatedUserMode << '\n'
        << "VBS hardware enforced: " << Hvci.VbsHardwareEnforced << '\n'
        << "Secure kernel running: " << Hvci.SecureKernelRunning << '\n';

    system("pause >nul");

    return 0;
}