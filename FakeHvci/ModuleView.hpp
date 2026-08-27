#pragma once

#include <ntifs.h>

struct ModuleOffsets final
{
    SIZE_T InLoadOrderLinks = 0x00;
    SIZE_T DllBase = 0x30;
    SIZE_T EntryPoint = 0x38;
    SIZE_T SizeOfImage = 0x40;
    SIZE_T FullDllName = 0x48;
    SIZE_T BaseDllName = 0x58;
    SIZE_T Flags = 0x68;
};

class ModuleView final
{
public:
    static constexpr ModuleOffsets DefaultOffsets{};

    constexpr ModuleView() noexcept = default;

    explicit constexpr ModuleView(
        PVOID Entry
    ) noexcept
        : Entry_(Entry),
        Offsets_(DefaultOffsets)
    {
    }

    constexpr ModuleView(
        PVOID Entry,
        const ModuleOffsets& Offsets
    ) noexcept : Entry_(Entry), Offsets_(Offsets)
    {
    }

    [[nodiscard]]
    constexpr auto GetEntry() const noexcept -> PVOID
    {
        return Entry_;
    }

    [[nodiscard]]
    constexpr auto GetOffsets() const noexcept -> const ModuleOffsets&
    {
        return Offsets_;
    }

    [[nodiscard]]
    constexpr auto IsValid() const noexcept -> bool
    {
        return Entry_ != nullptr;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return IsValid();
    }

    [[nodiscard]]
    auto InLoadOrderLinks() const noexcept -> PLIST_ENTRY
    {
        return Address<LIST_ENTRY>(
            Offsets_.InLoadOrderLinks
        );
    }

    [[nodiscard]]
    auto BaseAddress() const noexcept -> PVOID
    {
        return Read<PVOID>(
            Offsets_.DllBase
        );
    }

    [[nodiscard]]
    auto EntryPoint() const noexcept -> PVOID
    {
        return Read<PVOID>(
            Offsets_.EntryPoint
        );
    }

    [[nodiscard]]
    auto ImageSize() const noexcept -> ULONG
    {
        return Read<ULONG>(
            Offsets_.SizeOfImage
        );
    }

    [[nodiscard]]
    auto FullDllName() const noexcept -> const UNICODE_STRING*
    {
        return Address<const UNICODE_STRING>(
            Offsets_.FullDllName
        );
    }

    [[nodiscard]]
    auto BaseDllName() const noexcept -> const UNICODE_STRING*
    {
        return Address<const UNICODE_STRING>(
            Offsets_.BaseDllName
        );
    }

    [[nodiscard]]
    auto Flags() const noexcept -> ULONG
    {
        return Read<ULONG>(
            Offsets_.Flags
        );
    }

    [[nodiscard]]
    auto EndAddress() const noexcept -> PVOID
    {
        const auto Base =
            reinterpret_cast<ULONG_PTR>(BaseAddress());

        if (Base == 0)
        {
            return nullptr;
        }

        return reinterpret_cast<PVOID>(Base + ImageSize());
    }

    [[nodiscard]]
    auto ContainsAddress(
        const PVOID AddressToCheck
    ) const noexcept -> bool
    {
        if (AddressToCheck == nullptr)
        {
            return false;
        }

        const auto Base = reinterpret_cast<ULONG_PTR>(BaseAddress());

        const auto Address = reinterpret_cast<ULONG_PTR>(AddressToCheck);

        const auto Size = static_cast<ULONG_PTR>(ImageSize());

        if (Base == 0 || Size == 0)
        {
            return false;
        }

        return Address >= Base && Address < Base + Size;
    }

    [[nodiscard]]
    auto Address(
        const SIZE_T Offset
    ) const noexcept -> PVOID
    {
        if (Entry_ == nullptr)
        {
            return nullptr;
        }

        const auto Base = reinterpret_cast<ULONG_PTR>(Entry_);

        return reinterpret_cast<PVOID>(Base + Offset);
    }

private:
    template <typename T>
    [[nodiscard]]
    auto Address(
        const SIZE_T Offset
    ) const noexcept -> T*
    {
        if (Entry_ == nullptr)
        {
            return nullptr;
        }

        const auto Base = reinterpret_cast<ULONG_PTR>(Entry_);

        return reinterpret_cast<T*>(Base + Offset);
    }

    template <typename T>
    [[nodiscard]]
    auto Read(
        const SIZE_T Offset
    ) const noexcept -> T
    {
        const auto Value = Address<const T>(
            Offset
        );

        if (Value == nullptr)
        {
            return {};
        }

        return *Value;
    }

private:
    PVOID Entry_{};
    ModuleOffsets Offsets_{ DefaultOffsets };
};