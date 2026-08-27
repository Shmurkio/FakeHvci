#pragma once

#include <ntifs.h>

struct EprocessOffsets final
{
    SIZE_T DirectoryTableBase = 0x28;
    SIZE_T UniqueProcessId = 0x1D0;
    SIZE_T ActiveProcessLinks = 0x1D8;
    SIZE_T Token = 0x248;
    SIZE_T Peb = 0x2E0;
    SIZE_T ObjectTable = 0x300;
    SIZE_T ImageFileName = 0x338;
    SIZE_T VadRoot = 0x558;
};

class EprocessView final
{
public:
    static constexpr EprocessOffsets DefaultOffsets{};

    constexpr EprocessView() noexcept = default;

    explicit constexpr EprocessView(
        PEPROCESS Process
    ) noexcept : Process_(Process), Offsets_(DefaultOffsets)
    {
    }

    constexpr EprocessView(
        PEPROCESS Process,
        const EprocessOffsets& Offsets
    ) noexcept : Process_(Process), Offsets_(Offsets)
    {
    }

    [[nodiscard]]
    constexpr auto GetProcess() const noexcept -> PEPROCESS
    {
        return Process_;
    }

    [[nodiscard]]
    constexpr auto GetOffsets() const noexcept -> const EprocessOffsets&
    {
        return Offsets_;
    }

    [[nodiscard]]
    constexpr auto IsValid() const noexcept -> bool
    {
        return Process_ != nullptr;
    }

    [[nodiscard]]
    constexpr explicit operator bool() const noexcept
    {
        return IsValid();
    }

    [[nodiscard]]
    auto Cr3() const noexcept -> ULONG64
    {
        return Read<ULONG64>(
            Offsets_.DirectoryTableBase
        );
    }

    [[nodiscard]]
    auto UniqueProcessId() const noexcept -> HANDLE
    {
        return Read<HANDLE>(
            Offsets_.UniqueProcessId
        );
    }

    [[nodiscard]]
    auto ActiveProcessLinks() const noexcept -> PLIST_ENTRY
    {
        return Address<LIST_ENTRY>(
            Offsets_.ActiveProcessLinks
        );
    }

    [[nodiscard]]
    auto TokenRaw() const noexcept -> ULONG_PTR
    {
        return Read<ULONG_PTR>(
            Offsets_.Token
        );
    }

    [[nodiscard]]
    auto TokenObject() const noexcept -> PVOID
    {
        constexpr ULONG_PTR FastRefMask = 0xF;

        const auto Raw = TokenRaw();

        return reinterpret_cast<PVOID>(
            Raw & ~FastRefMask
        );
    }

    [[nodiscard]]
    auto Peb() const noexcept -> PPEB
    {
        return Read<PPEB>(
            Offsets_.Peb
        );
    }

    [[nodiscard]]
    auto ObjectTable() const noexcept -> PVOID
    {
        return Read<PVOID>(
            Offsets_.ObjectTable
        );
    }

    [[nodiscard]]
    auto ImageFileName() const noexcept -> const UCHAR*
    {
        return Address<const UCHAR>(
            Offsets_.ImageFileName
        );
    }

    [[nodiscard]]
    auto VadRoot() const noexcept -> PVOID
    {
        return Address<void>(
            Offsets_.VadRoot
        );
    }

    [[nodiscard]]
    auto Address(
        const SIZE_T Offset
    ) const noexcept -> PVOID
    {
        if (Process_ == nullptr)
        {
            return nullptr;
        }

        const auto Base = reinterpret_cast<ULONG_PTR>(Process_);

        return reinterpret_cast<PVOID>(Base + Offset);
    }

private:
    template <typename T>
    [[nodiscard]]
    auto Address(
        const SIZE_T Offset
    ) const noexcept -> T*
    {
        if (Process_ == nullptr)
        {
            return nullptr;
        }

        const auto Base = reinterpret_cast<ULONG_PTR>(Process_);

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
    PEPROCESS Process_{};
    EprocessOffsets Offsets_{ DefaultOffsets };
};