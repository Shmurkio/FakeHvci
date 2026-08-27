#pragma once

#include <ntifs.h>

#include "EprocessView.hpp"

namespace Process
{
    [[nodiscard]]
    inline auto ImageNameEquals(
        const UCHAR* Current,
        const char* Wanted
    ) noexcept -> bool
    {
        if (Current == nullptr || Wanted == nullptr)
        {
            return false;
        }

        constexpr SIZE_T MaximumImageNameLength = 15;

        for (SIZE_T Index = 0; Index < MaximumImageNameLength; ++Index)
        {
            const auto Left = static_cast<char>(Current[Index]);
            const auto Right = Wanted[Index];

            if (Left != Right)
            {
                return false;
            }

            if (Left == '\0')
            {
                return true;
            }
        }

        return Wanted[MaximumImageNameLength] == '\0';
    }

    [[nodiscard]]
    inline auto FromActiveProcessLinks(
        PLIST_ENTRY Entry,
        const EprocessOffsets& Offsets
    ) noexcept -> PEPROCESS
    {
        if (Entry == nullptr)
        {
            return nullptr;
        }

        const auto Address = reinterpret_cast<ULONG_PTR>(Entry);

        return reinterpret_cast<PEPROCESS>(Address - Offsets.ActiveProcessLinks);
    }

    [[nodiscard]]
    inline auto FindProcess(
        const char* ImageFileName,
        const EprocessOffsets& Offsets = EprocessView::DefaultOffsets
    ) noexcept -> EprocessView
    {
        if (ImageFileName == nullptr || ImageFileName[0] == '\0')
        {
            return {};
        }

        if (PsInitialSystemProcess == nullptr)
        {
            return {};
        }

        const EprocessView System{ PsInitialSystemProcess, Offsets };

        const auto FirstEntry = System.ActiveProcessLinks();

        if (FirstEntry == nullptr)
        {
            return {};
        }

        auto Entry = FirstEntry;

        do
        {
            const auto Process = FromActiveProcessLinks(
                Entry,
                Offsets
            );

            if (Process == nullptr)
            {
                return {};
            }

            const EprocessView View{ Process, Offsets };

            if (ImageNameEquals(View.ImageFileName(), ImageFileName))
            {
                return View;
            }

            Entry = Entry->Flink;

        } while (Entry != nullptr && Entry != FirstEntry);

        return {};
    }
}