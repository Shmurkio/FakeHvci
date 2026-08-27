#include <Windows.h>
#include <winternl.h>
#include <iostream>

#pragma comment(lib, "ntdll.lib")

struct HvciState
{
    bool Enabled{};
    bool AuditMode{};
    bool StrictMode{};
    bool IsolatedUserMode{};
};

[[nodiscard]]
HvciState QueryHvciState() noexcept
{
    constexpr ULONG HvciEnabled = 0x400;
    constexpr ULONG HvciAudit = 0x800;
    constexpr ULONG HvciStrict = 0x1000;
    constexpr ULONG HvciIum = 0x2000;

    SYSTEM_CODEINTEGRITY_INFORMATION Information{};
    Information.Length = sizeof(Information);

    if (NtQuerySystemInformation(SystemCodeIntegrityInformation, &Information, sizeof(Information), nullptr) < 0)
    {
        return {};
    }

    const auto Options = Information.CodeIntegrityOptions;

    return
    {
        .Enabled = (Options & HvciEnabled) != 0,
        .AuditMode = (Options & HvciAudit) != 0,
        .StrictMode = (Options & HvciStrict) != 0,
        .IsolatedUserMode = (Options & HvciIum) != 0,
    };
}

int main()
{
    const auto Hvci = QueryHvciState();

    std::cout
        << "HVCI state (NtQuerySystemInformation):\n"
        << "HVCI enabled:          " << std::boolalpha << Hvci.Enabled << '\n'
        << "HVCI audit mode:       " << Hvci.AuditMode << '\n'
        << "HVCI strict mode:      " << Hvci.StrictMode << '\n'
        << "Isolated user mode:    " << Hvci.IsolatedUserMode << '\n';

    system("pause >nul");

    return 0;
}