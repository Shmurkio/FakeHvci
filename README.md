# FakeHvci

FakeHvci is a small Windows kernel-mode proof of concept that modifies the internal `g_CiOptions` value in `CI.dll` to spoof the HVCI state reported by `NtQuerySystemInformation(SystemCodeIntegrityInformation)`.

## Important

This project is **only a proof of concept**.

* It only modifies the state reported through `SystemCodeIntegrityInformation`.
* It uses a build-specific `CI.dll` offset.
* It is **not PatchGuard compliant** and will cause a bugcheck.
* It is intended purely for research.

## Projects

* **FakeHvci** — Kernel driver that modifies `g_CiOptions`.
* **HvciCheck** — User-mode utility for querying the reported HVCI state through `NtQuerySystemInformation`.

## Showcase

[![Showcase](https://img.youtube.com/vi/Q-vAiqaQjlw/maxresdefault.jpg)](https://www.youtube.com/watch?v=Q-vAiqaQjlw )