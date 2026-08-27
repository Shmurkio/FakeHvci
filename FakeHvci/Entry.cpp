#include <ntifs.h>
#include "Module.hpp"

namespace
{
	static auto DriverUnload(
		[[maybe_unused]] PDRIVER_OBJECT DriverObject
	) noexcept -> void
	{
		DbgPrintEx(
			0, 0,
			"Driver unloaded\n"
		);
	}

	enum class CiOption : UINT32
	{
		None = 0,
		CodeIntegrityEnabled = 0x00000002,
		TestSigningEnabled = 0x00000008,
		DebugPolicy = 0x00000010,
		CustomSigning = 0x00000020,
		DebuggerPolicy = 0x00000040,
		HvciIumEnabled = 0x00004000,
		HvciKmciEnabled = 0x00008000,
		HvciKmciStrictMode = 0x00010000,
	};

	[[nodiscard]]
	constexpr CiOption operator|(CiOption Lhs, CiOption Rhs) noexcept
	{
		return static_cast<CiOption>(static_cast<UINT32>(Lhs) | static_cast<UINT32>(Rhs));
	}

	[[nodiscard]]
	constexpr CiOption operator&(CiOption Lhs, CiOption Rhs) noexcept
	{
		return static_cast<CiOption>(static_cast<UINT32>(Lhs) & static_cast<UINT32>(Rhs));
	}

	[[nodiscard]]
	constexpr bool HasCiOption(UINT32 options, CiOption option) noexcept
	{
		return (options & static_cast<UINT32>(option)) != 0;
	}
}

extern "C"
auto DriverEntry(
	PDRIVER_OBJECT DriverObject,
	[[maybe_unused]] PUNICODE_STRING RegistryPath
) -> NTSTATUS
{
	DriverObject->DriverUnload = DriverUnload;

	const auto Ci = Module::FindModule(
		L"CI.dll"
	);

	if (!Ci)
	{
		DbgPrintEx(
			0, 0,
			"CI.dll not found\n"
		);

		return STATUS_NOT_FOUND;
	}

	constexpr UINT32 CiOptionsOffset = 0x52004;

	const auto ImageBase = reinterpret_cast<UINT64>(Ci.BaseAddress());
	const auto ImageSize = Ci.ImageSize();
	auto& CiOptions = *reinterpret_cast<UINT32*>(ImageBase + CiOptionsOffset);

	//
	// PatchGuard loves this :)
	//
	// This only affects SystemCodeIntegrityInformation retrieval through NtQuerySystemInformation
	//
	CiOptions |= static_cast<UINT32>(CiOption::CodeIntegrityEnabled);
	CiOptions |= static_cast<UINT32>(CiOption::HvciKmciEnabled);
	CiOptions |= static_cast<UINT32>(CiOption::HvciKmciStrictMode);

	DbgPrintEx(
		0,
		0,
		"CI.dll:\n"
		"  ImageBase=0x%llX\n"
		"  ImageSize=0x%X\n"
		"  CiOptions=0x%X\n"
		"  HVCI=%s\n"
		"  HVCI Strict=%s\n",
		ImageBase,
		ImageSize,
		CiOptions,
		HasCiOption(CiOptions, CiOption::HvciKmciEnabled) ? "Enabled" : "Disabled",
		HasCiOption(CiOptions, CiOption::HvciKmciStrictMode) ? "Enabled" : "Disabled"
	);

    return STATUS_SUCCESS;
}