#pragma once

#include <ntifs.h>

#include "ModuleView.hpp"

namespace Module
{
    [[nodiscard]]
    inline auto ResolveLoadedModuleList() noexcept -> PLIST_ENTRY
    {
        UNICODE_STRING Name{};

        RtlInitUnicodeString(
            &Name,
            L"PsLoadedModuleList"
        );

        return reinterpret_cast<PLIST_ENTRY>(MmGetSystemRoutineAddress(&Name));
    }

    [[nodiscard]]
    inline auto FromInLoadOrderLinks(
        PLIST_ENTRY Entry,
        const ModuleOffsets& Offsets
    ) noexcept -> PVOID
    {
        if (Entry == nullptr)
        {
            return nullptr;
        }

        const auto Address = reinterpret_cast<ULONG_PTR>(Entry);

        return reinterpret_cast<PVOID>(Address - Offsets.InLoadOrderLinks);
    }

    [[nodiscard]]
    inline auto NameEquals(
        const UNICODE_STRING* Current,
        const UNICODE_STRING* Wanted
    ) noexcept -> bool
    {
        if (Current == nullptr || Wanted == nullptr)
        {
            return false;
        }

        if (Current->Buffer == nullptr || Wanted->Buffer == nullptr)
        {
            return false;
        }

        return RtlEqualUnicodeString(
            Current,
            Wanted,
            TRUE
        ) != FALSE;
    }

    [[nodiscard]]
    inline auto FindModule(
        PLIST_ENTRY LoadedModuleList,
        const UNICODE_STRING* BaseDllName,
        const ModuleOffsets& Offsets =
        ModuleView::DefaultOffsets
    ) noexcept -> ModuleView
    {
        if (LoadedModuleList == nullptr || BaseDllName == nullptr || BaseDllName->Buffer == nullptr || BaseDllName->Length == 0)
        {
            return {};
        }

        auto Entry = LoadedModuleList->Flink;

        while (Entry != nullptr && Entry != LoadedModuleList)
        {
            const auto LoaderEntry = FromInLoadOrderLinks(
                Entry,
                Offsets
            );

            if (LoaderEntry == nullptr)
            {
                return {};
            }

            const ModuleView View{
                LoaderEntry,
                Offsets
            };

            if (NameEquals(View.BaseDllName(), BaseDllName))
            {
                return View;
            }

            Entry = Entry->Flink;
        }

        return {};
    }

    [[nodiscard]]
    inline auto FindModule(
        PLIST_ENTRY LoadedModuleList,
        const wchar_t* BaseDllName,
        const ModuleOffsets& Offsets =
        ModuleView::DefaultOffsets
    ) noexcept -> ModuleView
    {
        if (BaseDllName == nullptr ||
            BaseDllName[0] == L'\0')
        {
            return {};
        }

        UNICODE_STRING Name{};

        RtlInitUnicodeString(
            &Name,
            BaseDllName
        );

        return FindModule(
            LoadedModuleList,
            &Name,
            Offsets
        );
    }

    [[nodiscard]]
    inline auto FindModule(
        const wchar_t* BaseDllName,
        const ModuleOffsets& Offsets =
        ModuleView::DefaultOffsets
    ) noexcept -> ModuleView
    {
        const auto LoadedModuleList = ResolveLoadedModuleList();

        if (LoadedModuleList == nullptr)
        {
            return {};
        }

        return FindModule(
            LoadedModuleList,
            BaseDllName,
            Offsets
        );
    }
}