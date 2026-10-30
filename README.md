# chaos_recreation

> A simple Chaos physics recreation to support Unreal Engine 5 games.

[![Proof](https://i.postimg.cc/s2r0mX4V/chaos-recreation-Nzq-Iuw0MWA.png)](https://i.postimg.cc/s2r0mX4V/chaos-recreation-Nzq-Iuw0MWA.png)

In theory this shouldn't change much between games, but keep in mind it has **not** been tested elsewhere (and probably won't be).

It was tested on `LyraGame-Win64-Shipping` (the templated Unreal Engine game), with PDB access to make things easier.

---

## Table of Contents

- [Updating](#updating)
- [Offsets at a glance](#offsets-at-a-glance)
- [1. UBodySetup::ChaosTriMeshes](#1-ubodysetupchaostrimeshes)
  - [With a PDB](#with-a-pdb)
  - [Without a PDB](#without-a-pdb)
- [2. FTriangleMeshImplicitObject data](#2-ftrianglemeshimplicitobject-data)
  - [With a PDB](#with-a-pdb-1)
  - [Without a PDB](#without-a-pdb-1)

---

## Updating

This is pretty simple. There are only a few offsets that need to be changed, all of them specific to the Chaos data. Everything else you *should* know in theory.

There are two things to find:

1. `UBodySetup::ChaosTriMeshes`
2. The `FTriangleMeshImplicitObject` layout (particles and index buffers)

## Offsets at a glance

| Field | Offset | How to find it |
| --- | --- | --- |
| `UBodySetup::ChaosTriMeshes` | `WalkableSlopeOverride + 0x30` | `FBodyInstance::GetWalkableSlopeOverride` |
| `FTriangleMeshImplicitObject::MParticles` | `+0x48` | `FTriangleMeshImplicitObject::GetFaceNormal` |
| `FTrimeshIndexBuffer::LargeIdxBuffer` | `+0x60` | `FTriangleMeshImplicitObject::GetFaceNormal` |
| `FTrimeshIndexBuffer::SmallIdxBuffer` | `+0x70` | `FTriangleMeshImplicitObject::GetFaceNormal` |
| `FTrimeshIndexBuffer::bRequiresLargeIndices` | `+0x80` | `FTriangleMeshImplicitObject::GetFaceNormal` |

> [!NOTE]
> The values above come from the tested Lyra build. Your game's offsets may differ, so use the sections below to re-derive them.

---

## 1. UBodySetup::ChaosTriMeshes

`UBodySetup::ChaosTriMeshes` usually isn't reflected, so finding it can be a little more tedious. It is typically located at **`+0x30` from `WalkableSlopeOverride`** inside `UBodySetup` (thank you voidie x).

**Signature**

```
40 53 48 83 EC ? F6 41 ? ? 48 8B D9 75 ? E8 ? ? ? ? 84 C0
```

> [!WARNING]
> This signature likely won't work depending on your game. The function is small, so it may be inlined or change aggressively.

For reference, this is `FBodyInstance::GetWalkableSlopeOverride`. The `add rax, 0E8h` is our `WalkableSlopeOverride` offset. Add `0x30` to it to get `ChaosTriMeshes`.

### With a PDB

<details>
<summary>Show disassembly (PDB symbols)</summary>

```asm
.text:0000000143DB33F0
.text:0000000143DB33F0                         ; =============== S U B R O U T I N E =======================================
.text:0000000143DB33F0
.text:0000000143DB33F0
.text:0000000143DB33F0                         ; public: struct FWalkableSlopeOverride const & FBodyInstance::GetWalkableSlopeOverride(void)const
.text:0000000143DB33F0                         ?GetWalkableSlopeOverride@FBodyInstance@@QEBAAEBUFWalkableSlopeOverride@@XZ proc near
.text:0000000143DB33F0                                                                 ; CODE XREF: UPrimitiveComponent::GetWalkableSlopeOverride(void)+7↓j
.text:0000000143DB33F0                                                                 ; DATA XREF: .pdata:00000001499863DC↓o
.text:0000000143DB33F0 40 53                                   push    rbx
.text:0000000143DB33F2 48 83 EC 20                             sub     rsp, 20h
.text:0000000143DB33F6 F6 41 3D 20                             test    byte ptr [rcx+3Dh], 20h
.text:0000000143DB33FA 48 8B D9                                mov     rbx, rcx
.text:0000000143DB33FD 75 1D                                   jnz     short loc_143DB341C
.text:0000000143DB33FF E8 8C B8 77 FD                          call    ?IsValid@FWeakObjectPtr@@QEBA_NXZ ; FWeakObjectPtr::IsValid(void)
.text:0000000143DB3404 84 C0                                   test    al, al
.text:0000000143DB3406 74 14                                   jz      short loc_143DB341C
.text:0000000143DB3408 48 8B CB                                mov     rcx, rbx
.text:0000000143DB340B E8 10 88 77 FD                          call    ?Get@FWeakObjectPtr@@QEBAPEAVUObject@@XZ ; FWeakObjectPtr::Get(void)
.text:0000000143DB3410 48 05 E8 00 00 00                       add     rax, 0E8h ; THIS IS OUR WalkableSlopeOverride
.text:0000000143DB3416 48 83 C4 20                             add     rsp, 20h
.text:0000000143DB341A 5B                                      pop     rbx
.text:0000000143DB341B C3                                      retn
.text:0000000143DB341C                         ; ---------------------------------------------------------------------------
.text:0000000143DB341C
.text:0000000143DB341C                         loc_143DB341C:                          ; CODE XREF: FBodyInstance::GetWalkableSlopeOverride(void)+D↑j
.text:0000000143DB341C                                                                 ; FBodyInstance::GetWalkableSlopeOverride(void)+16↑j
.text:0000000143DB341C 48 8D 83 10 01 00 00                    lea     rax, [rbx+110h]
.text:0000000143DB3423 48 83 C4 20                             add     rsp, 20h
.text:0000000143DB3427 5B                                      pop     rbx
.text:0000000143DB3428 C3                                      retn
.text:0000000143DB3428                         ?GetWalkableSlopeOverride@FBodyInstance@@QEBAAEBUFWalkableSlopeOverride@@XZ endp
.text:0000000143DB3428
.text:0000000143DB3428                         ; ---------------------------------------------------------------------------
```

</details>

The line of interest:

```asm
.text:0000000143DB3410 48 05 E8 00 00 00    add     rax, 0E8h ; THIS IS OUR WalkableSlopeOverride
```

`0xE8 + 0x30 = 0x118` is the `ChaosTriMeshes` offset in this build.

### Without a PDB

What if you **don't** have a PDB with nicely renamed functions? That's partially not my problem, but here is the assembly and pseudocode without one.

<details>
<summary>Show disassembly (no PDB)</summary>

```asm
.text:0000000143DB33F0
.text:0000000143DB33F0 ; =============== S U B R O U T I N E =======================================
.text:0000000143DB33F0
.text:0000000143DB33F0
.text:0000000143DB33F0 ; __int64 __fastcall sub_143DB33F0(__int64)
.text:0000000143DB33F0 sub_143DB33F0   proc near
.text:0000000143DB33F0                 push    rbx
.text:0000000143DB33F2                 sub     rsp, 20h
.text:0000000143DB33F6                 test    byte ptr [rcx+3Dh], 20h
.text:0000000143DB33FA                 mov     rbx, rcx
.text:0000000143DB33FD                 jnz     short loc_143DB341C
.text:0000000143DB33FF                 call    sub_14152EC90
.text:0000000143DB3404                 test    al, al
.text:0000000143DB3406                 jz      short loc_143DB341C
.text:0000000143DB3408                 mov     rcx, rbx
.text:0000000143DB340B                 call    sub_14152BC20
.text:0000000143DB3410                 add     rax, 0E8h ; HERE HERE HERE!
.text:0000000143DB3416                 add     rsp, 20h
.text:0000000143DB341A                 pop     rbx
.text:0000000143DB341B                 retn
.text:0000000143DB341C ; ---------------------------------------------------------------------------
.text:0000000143DB341C
.text:0000000143DB341C loc_143DB341C:                          ; CODE XREF: sub_143DB33F0+D↑j
.text:0000000143DB341C                                         ; sub_143DB33F0+16↑j
.text:0000000143DB341C                 lea     rax, [rbx+110h]
.text:0000000143DB3423                 add     rsp, 20h
.text:0000000143DB3427                 pop     rbx
.text:0000000143DB3428                 retn
.text:0000000143DB3428 sub_143DB33F0   endp
.text:0000000143DB3428
.text:0000000143DB3428 ; ---------------------------------------------------------------------------
```

</details>

**Pseudocode**

```c
__int64 __fastcall sub_143DB33F0(__int64 a1)
{
  if ( (*(_BYTE *)(a1 + 61) & 0x20) != 0 || (unsigned __int8)sub_14152EC90() == 0 )
    return a1 + 0x110; // ding ding ding!
  else
    return sub_14152BC20(a1) + 232;
}
```

---

## 2. FTriangleMeshImplicitObject data

Next we need the offsets for the `FTriangleMesh` data. These can all be found in one single function: `Chaos::FTriangleMeshImplicitObject::GetFaceNormal`.

**Signature**

```
4C 8B 59 ? 0F 29 74 24 ? 0F 29 7C 24 ? 44 0F 29 44 24 ? 44 0F 29 0C 24 49 63 C0 74 ? 48 8B 49
```

Key lines in the disassembly:

| Instruction | Meaning |
| --- | --- |
| `mov r11, [rcx+48h]` | `FTriangleMeshImplicitObject::MParticles` |
| `cmp byte ptr [rcx+80h], 0` | `FTrimeshIndexBuffer::bRequiresLargeIndices` |
| `mov rcx, [rcx+60h]` | `FTrimeshIndexBuffer::LargeIdxBuffer` |
| `mov rcx, [rcx+70h]` | `FTrimeshIndexBuffer::SmallIdxBuffer` |

### With a PDB

<details>
<summary>Show disassembly (PDB symbols)</summary>

```asm
.text:0000000141F0BEA0
.text:0000000141F0BEA0                         ; =============== S U B R O U T I N E =======================================
.text:0000000141F0BEA0
.text:0000000141F0BEA0
.text:0000000141F0BEA0                         ; double *__fastcall Chaos::FTriangleMeshImplicitObject::GetFaceNormal(__int64, double *, int)
.text:0000000141F0BEA0                         ?GetFaceNormal@FTriangleMeshImplicitObject@Chaos@@QEBA?AV?$TVector@N$02@2@H@Z proc near
.text:0000000141F0BEA0                                                                 ; CODE XREF: ??$?RV?$TArray@V?$TVector@G$02@Chaos@@V?$TSizedDefaultAllocator@$0CA@@@@@@_lambda_1_@?1???$SweepGeomCCDImp@V?$TBox@N$02@Chaos@@@FTriangleMeshImplicitObject@Chaos@@AEBA_NAEBV?$TBox@N$02@2@AEBV?$TRigidTransform@N$02@2@AEBV?$TVector@N$02@2@NNNAEAN3AEAV52@4AEAH42@Z@QEBA?A_PAEBV?$TArray@V?$TVector@G$02@Chaos@@V?$TSizedDefaultAllocator@$0CA@@@@@@Z+94C↑p
.text:0000000141F0BEA0                                                                 ; ??$?RV?$TArray@V?$TVector@G$02@Chaos@@V?$TSizedDefaultAllocator@$0CA@@@@@@_lambda_1_@?1???$SweepGeomCCDImp@V?$TImplicitObjectScaled@V?$TBox@N$02@Chaos@@$00@Chaos@@@FTriangleMeshImplicitObject@Chaos@@AEBA_NAEBV?$TImplicitObjectScaled@V?$TBox@N$02@Chaos@@$00@2@AEBV?$TRigidTransform@N$02@2@AEBV?$TVector@N$02@2@NNNAEAN3AEAV52@4AEAH42@Z@QEBA?A_PAEBV?$TArray@V?$TVector@G$02@Chaos@@V?$TSizedDefaultAllocator@$0CA@@@@@@Z+94C↑p ...
.text:0000000141F0BEA0
.text:0000000141F0BEA0                         var_48          = xmmword ptr -48h
.text:0000000141F0BEA0                         var_38          = xmmword ptr -38h
.text:0000000141F0BEA0                         var_28          = xmmword ptr -28h
.text:0000000141F0BEA0                         var_18          = xmmword ptr -18h
.text:0000000141F0BEA0
.text:0000000141F0BEA0 48 83 EC 48                             sub     rsp, 48h
.text:0000000141F0BEA4 4C 8B D2                                mov     r10, rdx
.text:0000000141F0BEA7 41 83 F8 FF                             cmp     r8d, 0FFFFFFFFh
.text:0000000141F0BEAB 0F 84 9B 01 00 00                       jz      loc_141F0C04C
.text:0000000141F0BEB1 80 B9 80 00 00 00 00                    cmp     byte ptr [rcx+80h], 0                   ; FTrimeshIndexBuffer::bRequiresLargeIndices
.text:0000000141F0BEB8 4C 8B 59 48                             mov     r11, [rcx+48h]                          ; FTriangleMeshImplicitObject::MParticles!
.text:0000000141F0BEBC
.text:0000000141F0BEBC                         loc_141F0BEBC:                          ; DATA XREF: .rdata:0000000148CFCA4C↓o
.text:0000000141F0BEBC                                                                 ; .rdata:0000000148CFCA5C↓o ...
.text:0000000141F0BEBC 0F 29 74 24 30                          movaps  [rsp+48h+var_18], xmm6
.text:0000000141F0BEC1 0F 29 7C 24 20                          movaps  [rsp+48h+var_28], xmm7
.text:0000000141F0BEC6 44 0F 29 44 24 10                       movaps  [rsp+48h+var_38], xmm8
.text:0000000141F0BECC 44 0F 29 0C 24                          movaps  [rsp+48h+var_48], xmm9
.text:0000000141F0BED1 49 63 C0                                movsxd  rax, r8d
.text:0000000141F0BED4 74 34                                   jz      short loc_141F0BF0A
.text:0000000141F0BED6 48 8B 49 60                             mov     rcx, [rcx+60h]                         ; FTrimeshIndexBuffer::LargeIdxBuffer
.text:0000000141F0BEDA 4C 8D 04 40                             lea     r8, [rax+rax*2]
.text:0000000141F0BEDE 4A 63 04 81                             movsxd  rax, dword ptr [rcx+r8*4]
.text:0000000141F0BEE2 4C 8D 0C 40                             lea     r9, [rax+rax*2]
.text:0000000141F0BEE6 4A 63 44 81 04                          movsxd  rax, dword ptr [rcx+r8*4+4]
.text:0000000141F0BEEB 48 8D 14 40                             lea     rdx, [rax+rax*2]
.text:0000000141F0BEEF 4A 63 44 81 08                          movsxd  rax, dword ptr [rcx+r8*4+8]
.text:0000000141F0BEF4 F3 41 0F 10 34 93                       movss   xmm6, dword ptr [r11+rdx*4]
.text:0000000141F0BEFA F3 41 0F 10 6C 93 04                    movss   xmm5, dword ptr [r11+rdx*4+4]
.text:0000000141F0BF01 F3 41 0F 10 5C 93 08                    movss   xmm3, dword ptr [r11+rdx*4+8]
.text:0000000141F0BF08 EB 32                                   jmp     short loc_141F0BF3C
.text:0000000141F0BF0A                         ; ---------------------------------------------------------------------------
.text:0000000141F0BF0A
.text:0000000141F0BF0A                         loc_141F0BF0A:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+34↑j
.text:0000000141F0BF0A 48 8B 49 70                             mov     rcx, [rcx+70h]                        ; FTrimeshIndexBuffer::SmallIdxBuffer
.text:0000000141F0BF0E 48 8D 14 40                             lea     rdx, [rax+rax*2]
.text:0000000141F0BF12 0F B7 04 51                             movzx   eax, word ptr [rcx+rdx*2]
.text:0000000141F0BF16 4C 8D 0C 40                             lea     r9, [rax+rax*2]
.text:0000000141F0BF1A 0F B7 44 51 02                          movzx   eax, word ptr [rcx+rdx*2+2]
.text:0000000141F0BF1F 4C 8D 04 40                             lea     r8, [rax+rax*2]
.text:0000000141F0BF23 0F B7 44 51 04                          movzx   eax, word ptr [rcx+rdx*2+4]
.text:0000000141F0BF28 F3 43 0F 10 34 83                       movss   xmm6, dword ptr [r11+r8*4]
.text:0000000141F0BF2E F3 43 0F 10 6C 83 04                    movss   xmm5, dword ptr [r11+r8*4+4]
.text:0000000141F0BF35 F3 43 0F 10 5C 83 08                    movss   xmm3, dword ptr [r11+r8*4+8]
.text:0000000141F0BF3C
.text:0000000141F0BF3C                         loc_141F0BF3C:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+68↑j
.text:0000000141F0BF3C F3 43 0F 5C 5C 8B 08                    subss   xmm3, dword ptr [r11+r9*4+8]
.text:0000000141F0BF43 F3 43 0F 5C 6C 8B 04                    subss   xmm5, dword ptr [r11+r9*4+4]
.text:0000000141F0BF4A F3 43 0F 5C 34 8B                       subss   xmm6, dword ptr [r11+r9*4]
.text:0000000141F0BF50 48 8D 0C 40                             lea     rcx, [rax+rax*2]
.text:0000000141F0BF54 F3 41 0F 10 7C 8B 04                    movss   xmm7, dword ptr [r11+rcx*4+4]
.text:0000000141F0BF5B F3 43 0F 5C 7C 8B 04                    subss   xmm7, dword ptr [r11+r9*4+4]
.text:0000000141F0BF62 F3 41 0F 10 54 8B 08                    movss   xmm2, dword ptr [r11+rcx*4+8]
.text:0000000141F0BF69 F3 43 0F 5C 54 8B 08                    subss   xmm2, dword ptr [r11+r9*4+8]
.text:0000000141F0BF70 F3 41 0F 10 24 8B                       movss   xmm4, dword ptr [r11+rcx*4]
.text:0000000141F0BF76 F3 43 0F 5C 24 8B                       subss   xmm4, dword ptr [r11+r9*4]
.text:0000000141F0BF7C 0F 28 C7                                movaps  xmm0, xmm7
.text:0000000141F0BF7F F3 0F 59 FE                             mulss   xmm7, xmm6
.text:0000000141F0BF83 44 0F 28 CA                             movaps  xmm9, xmm2
.text:0000000141F0BF87 F3 0F 59 C3                             mulss   xmm0, xmm3
.text:0000000141F0BF8B F3 0F 59 D6                             mulss   xmm2, xmm6
.text:0000000141F0BF8F 44 0F 28 C4                             movaps  xmm8, xmm4
.text:0000000141F0BF93 F3 44 0F 59 C3                          mulss   xmm8, xmm3
.text:0000000141F0BF98 F3 0F 10 1D 98 CC 2C 05                 movss   xmm3, cs:__real@322bcc77
.text:0000000141F0BFA0 F3 44 0F 59 CD                          mulss   xmm9, xmm5
.text:0000000141F0BFA5 F3 44 0F 5C C2                          subss   xmm8, xmm2
.text:0000000141F0BFAA F3 0F 59 E5                             mulss   xmm4, xmm5
.text:0000000141F0BFAE F3 44 0F 5C C8                          subss   xmm9, xmm0
.text:0000000141F0BFB3 F3 0F 5C FC                             subss   xmm7, xmm4
.text:0000000141F0BFB7 41 0F 28 D0                             movaps  xmm2, xmm8
.text:0000000141F0BFBB F3 41 0F 59 D0                          mulss   xmm2, xmm8
.text:0000000141F0BFC0 41 0F 28 C1                             movaps  xmm0, xmm9
.text:0000000141F0BFC4 F3 41 0F 59 C1                          mulss   xmm0, xmm9
.text:0000000141F0BFC9 0F 28 CF                                movaps  xmm1, xmm7
.text:0000000141F0BFCC F3 0F 59 CF                             mulss   xmm1, xmm7
.text:0000000141F0BFD0 F3 0F 58 D0                             addss   xmm2, xmm0
.text:0000000141F0BFD4 F3 0F 58 D1                             addss   xmm2, xmm1
.text:0000000141F0BFD8 0F 2F D3                                comiss  xmm2, xmm3
.text:0000000141F0BFDB 72 26                                   jb      short loc_141F0C003
.text:0000000141F0BFDD F3 0F 10 05 DB 86 2B 05                 movss   xmm0, cs:__real@3f800000
.text:0000000141F0BFE5 0F 57 C9                                xorps   xmm1, xmm1
.text:0000000141F0BFE8 F3 0F 51 CA                             sqrtss  xmm1, xmm2
.text:0000000141F0BFEC 0F 2F CB                                comiss  xmm1, xmm3
.text:0000000141F0BFEF F3 0F 5E C1                             divss   xmm0, xmm1
.text:0000000141F0BFF3 F3 0F 59 F8                             mulss   xmm7, xmm0
.text:0000000141F0BFF7 F3 44 0F 59 C0                          mulss   xmm8, xmm0
.text:0000000141F0BFFC F3 44 0F 59 C8                          mulss   xmm9, xmm0
.text:0000000141F0C001 73 10                                   jnb     short loc_141F0C013
.text:0000000141F0C003
.text:0000000141F0C003                         loc_141F0C003:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+13B↑j
.text:0000000141F0C003 F2 0F 10 0D 05 87 2B 05                 movsd   xmm1, cs:__real@3ff0000000000000
.text:0000000141F0C00B 0F 57 C0                                xorps   xmm0, xmm0
.text:0000000141F0C00E 0F 57 D2                                xorps   xmm2, xmm2
.text:0000000141F0C011 EB 0B                                   jmp     short loc_141F0C01E
.text:0000000141F0C013                         ; ---------------------------------------------------------------------------
.text:0000000141F0C013
.text:0000000141F0C013                         loc_141F0C013:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+161↑j
.text:0000000141F0C013 0F 5A CF                                cvtps2pd xmm1, xmm7
.text:0000000141F0C016 41 0F 5A C0                             cvtps2pd xmm0, xmm8
.text:0000000141F0C01A 41 0F 5A D1                             cvtps2pd xmm2, xmm9
.text:0000000141F0C01E
.text:0000000141F0C01E                         loc_141F0C01E:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+171↑j
.text:0000000141F0C01E F2 41 0F 11 12                          movsd   qword ptr [r10], xmm2
.text:0000000141F0C023 49 8B C2                                mov     rax, r10
.text:0000000141F0C026 F2 41 0F 11 42 08                       movsd   qword ptr [r10+8], xmm0
.text:0000000141F0C02C F2 41 0F 11 4A 10                       movsd   qword ptr [r10+10h], xmm1
.text:0000000141F0C032 44 0F 28 44 24 10                       movaps  xmm8, [rsp+48h+var_38]
.text:0000000141F0C038 0F 28 7C 24 20                          movaps  xmm7, [rsp+48h+var_28]
.text:0000000141F0C03D 0F 28 74 24 30                          movaps  xmm6, [rsp+48h+var_18]
.text:0000000141F0C042 44 0F 28 0C 24                          movaps  xmm9, [rsp+48h+var_48]
.text:0000000141F0C047 48 83 C4 48                             add     rsp, 48h
.text:0000000141F0C04B C3                                      retn
.text:0000000141F0C04C                         ; ---------------------------------------------------------------------------
.text:0000000141F0C04C
.text:0000000141F0C04C                         loc_141F0C04C:                          ; CODE XREF: Chaos::FTriangleMeshImplicitObject::GetFaceNormal(int)+B↑j
.text:0000000141F0C04C                                                                 ; DATA XREF: .pdata:00000001497ADA8C↓o ...
.text:0000000141F0C04C 33 C0                                   xor     eax, eax
.text:0000000141F0C04E 48 89 02                                mov     [rdx], rax
.text:0000000141F0C051 48 89 42 08                             mov     [rdx+8], rax
.text:0000000141F0C055 48 B8 00 00 00 00 00 00                 mov     rax, 3FF0000000000000h
.text:0000000141F0C055 F0 3F
.text:0000000141F0C05F 48 89 42 10                             mov     [rdx+10h], rax
.text:0000000141F0C063 49 8B C2                                mov     rax, r10
.text:0000000141F0C066 48 83 C4 48                             add     rsp, 48h
.text:0000000141F0C06A C3                                      retn
.text:0000000141F0C06A                         ?GetFaceNormal@FTriangleMeshImplicitObject@Chaos@@QEBA?AV?$TVector@N$02@2@H@Z endp
.text:0000000141F0C06A
.text:0000000141F0C06A                         ; ---------------------------------------------------------------------------
```

</details>

**Pseudocode**

<details>
<summary>Show pseudocode</summary>

```cpp
double *__fastcall Chaos::FTriangleMeshImplicitObject::GetFaceNormal(__int64 a1, double *a2, int a3)
{
  __int64 v3; // r11
  __int64 v4; // rax
  __int64 v5; // rcx
  __int64 v6; // r9
  __int64 v7; // rax
  float v8; // xmm6_4
  float v9; // xmm5_4
  float v10; // xmm3_4
  __int64 v11; // rcx
  __int64 v12; // r8
  float v13; // xmm3_4
  float v14; // xmm5_4
  float v15; // xmm6_4
  float v16; // xmm2_4
  float v17; // xmm4_4
  float v18; // xmm0_4
  float v19; // xmm8_4
  float v20; // xmm9_4
  float v21; // xmm7_4
  float v22; // xmm2_4
  float v23; // xmm1_4
  float v24; // xmm7_4
  float v25; // xmm8_4
  float v26; // xmm9_4
  double v27; // xmm1_8
  double v28; // xmm0_8
  double v29; // xmm2_8

  if ( a3 == -1 )
  {
    *a2 = 0.0;
    a2[1] = 0.0;
    a2[2] = 1.0;
    return a2;
  }
  else
  {
    v3 = *(_QWORD *)(a1 + 0x48);                // FTriangleMeshImplicitObject::MParticles
    v4 = a3;
    if ( *(_BYTE *)(a1 + 0x80) != 0 )           // FTrimeshIndexBuffer::bRequiresLargeIndices
    {
      v5 = *(_QWORD *)(a1 + 0x60);              // FTrimeshIndexBuffer::LargeIdxBuffer
      v6 = 3LL * *(int *)(v5 + 12LL * a3);
      v7 = *(int *)(v5 + 12LL * a3 + 8);
      v8 = *(float *)(v3 + 12LL * *(int *)(v5 + 12LL * a3 + 4));
      v9 = *(float *)(v3 + 12LL * *(int *)(v5 + 12LL * a3 + 4) + 4);
      v10 = *(float *)(v3 + 12LL * *(int *)(v5 + 12LL * a3 + 4) + 8);
    }
    else
    {
      v11 = *(_QWORD *)(a1 + 0x70);             // FTrimeshIndexBuffer::SmallIdxBuffer
      v6 = 3LL * *(unsigned __int16 *)(v11 + 6LL * a3);
      v12 = 3LL * *(unsigned __int16 *)(v11 + 6LL * a3 + 2);
      v7 = *(unsigned __int16 *)(v11 + 6 * v4 + 4);
      v8 = *(float *)(v3 + 4 * v12);
      v9 = *(float *)(v3 + 4 * v12 + 4);
      v10 = *(float *)(v3 + 4 * v12 + 8);
    }
    v13 = v10 - *(float *)(v3 + 4 * v6 + 8);
    v14 = v9 - *(float *)(v3 + 4 * v6 + 4);
    v15 = v8 - *(float *)(v3 + 4 * v6);
    v16 = *(float *)(v3 + 12 * v7 + 8) - *(float *)(v3 + 4 * v6 + 8);
    v17 = *(float *)(v3 + 12 * v7) - *(float *)(v3 + 4 * v6);
    v18 = *(float *)(v3 + 12 * v7 + 4) - *(float *)(v3 + 4 * v6 + 4);
    v19 = (float)(v17 * v13) - (float)(v16 * v15);
    v20 = (float)(v16 * v14) - (float)(v18 * v13);
    v21 = (float)(v18 * v15) - (float)(v17 * v14);
    v22 = (float)((float)(v19 * v19) + (float)(v20 * v20)) + (float)(v21 * v21);
    if ( v22 >= 0.00000001f
      && (v23 = fsqrt(v22),
          v24 = v21 * (float)(1.0 / v23),
          v25 = v19 * (float)(1.0 / v23),
          v26 = v20 * (float)(1.0 / v23),
          v23 >= 0.00000001f) )
    {
      v27 = v24;
      v28 = v25;
      v29 = v26;
    }
    else
    {
      v27 = 1.0;
      v28 = 0.0;
      v29 = 0.0;
    }
    *a2 = v29;
    a2[1] = v28;
    a2[2] = v27;
    return a2;
  }
}
```

</details>

### Without a PDB

Since the PDB only changes the names, the pseudocode is 1:1 between the PDB and non-PDB versions, so only the assembly is shown here.

<details>
<summary>Show disassembly (no PDB)</summary>

```asm
.text:0000000141F0BEA0
.text:0000000141F0BEA0 ; =============== S U B R O U T I N E =======================================
.text:0000000141F0BEA0
.text:0000000141F0BEA0
.text:0000000141F0BEA0 ; double *__fastcall sub_141F0BEA0(__int64, double *, int)
.text:0000000141F0BEA0 sub_141F0BEA0   proc near               ; CODE XREF: sub_141ED54A0+94C↑p
.text:0000000141F0BEA0                                         ; sub_141ED5E50+94C↑p ...
.text:0000000141F0BEA0
.text:0000000141F0BEA0 var_48          = xmmword ptr -48h
.text:0000000141F0BEA0 var_38          = xmmword ptr -38h
.text:0000000141F0BEA0 var_28          = xmmword ptr -28h
.text:0000000141F0BEA0 var_18          = xmmword ptr -18h
.text:0000000141F0BEA0
.text:0000000141F0BEA0                 sub     rsp, 48h
.text:0000000141F0BEA4                 mov     r10, rdx
.text:0000000141F0BEA7                 cmp     r8d, 0FFFFFFFFh
.text:0000000141F0BEAB                 jz      loc_141F0C04C
.text:0000000141F0BEB1                 cmp     byte ptr [rcx+80h], 0
.text:0000000141F0BEB8                 mov     r11, [rcx+48h]
.text:0000000141F0BEBC                 movaps  [rsp+48h+var_18], xmm6
.text:0000000141F0BEC1                 movaps  [rsp+48h+var_28], xmm7
.text:0000000141F0BEC6                 movaps  [rsp+48h+var_38], xmm8
.text:0000000141F0BECC                 movaps  [rsp+48h+var_48], xmm9
.text:0000000141F0BED1                 movsxd  rax, r8d
.text:0000000141F0BED4                 jz      short loc_141F0BF0A
.text:0000000141F0BED6                 mov     rcx, [rcx+60h]
.text:0000000141F0BEDA                 lea     r8, [rax+rax*2]
.text:0000000141F0BEDE                 movsxd  rax, dword ptr [rcx+r8*4]
.text:0000000141F0BEE2                 lea     r9, [rax+rax*2]
.text:0000000141F0BEE6                 movsxd  rax, dword ptr [rcx+r8*4+4]
.text:0000000141F0BEEB                 lea     rdx, [rax+rax*2]
.text:0000000141F0BEEF                 movsxd  rax, dword ptr [rcx+r8*4+8]
.text:0000000141F0BEF4                 movss   xmm6, dword ptr [r11+rdx*4]
.text:0000000141F0BEFA                 movss   xmm5, dword ptr [r11+rdx*4+4]
.text:0000000141F0BF01                 movss   xmm3, dword ptr [r11+rdx*4+8]
.text:0000000141F0BF08                 jmp     short loc_141F0BF3C
.text:0000000141F0BF0A ; ---------------------------------------------------------------------------
.text:0000000141F0BF0A
.text:0000000141F0BF0A loc_141F0BF0A:                          ; CODE XREF: sub_141F0BEA0+34↑j
.text:0000000141F0BF0A                 mov     rcx, [rcx+70h]
.text:0000000141F0BF0E                 lea     rdx, [rax+rax*2]
.text:0000000141F0BF12                 movzx   eax, word ptr [rcx+rdx*2]
.text:0000000141F0BF16                 lea     r9, [rax+rax*2]
.text:0000000141F0BF1A                 movzx   eax, word ptr [rcx+rdx*2+2]
.text:0000000141F0BF1F                 lea     r8, [rax+rax*2]
.text:0000000141F0BF23                 movzx   eax, word ptr [rcx+rdx*2+4]
.text:0000000141F0BF28                 movss   xmm6, dword ptr [r11+r8*4]
.text:0000000141F0BF2E                 movss   xmm5, dword ptr [r11+r8*4+4]
.text:0000000141F0BF35                 movss   xmm3, dword ptr [r11+r8*4+8]
.text:0000000141F0BF3C
.text:0000000141F0BF3C loc_141F0BF3C:                          ; CODE XREF: sub_141F0BEA0+68↑j
.text:0000000141F0BF3C                 subss   xmm3, dword ptr [r11+r9*4+8]
.text:0000000141F0BF43                 subss   xmm5, dword ptr [r11+r9*4+4]
.text:0000000141F0BF4A                 subss   xmm6, dword ptr [r11+r9*4]
.text:0000000141F0BF50                 lea     rcx, [rax+rax*2]
.text:0000000141F0BF54                 movss   xmm7, dword ptr [r11+rcx*4+4]
.text:0000000141F0BF5B                 subss   xmm7, dword ptr [r11+r9*4+4]
.text:0000000141F0BF62                 movss   xmm2, dword ptr [r11+rcx*4+8]
.text:0000000141F0BF69                 subss   xmm2, dword ptr [r11+r9*4+8]
.text:0000000141F0BF70                 movss   xmm4, dword ptr [r11+rcx*4]
.text:0000000141F0BF76                 subss   xmm4, dword ptr [r11+r9*4]
.text:0000000141F0BF7C                 movaps  xmm0, xmm7
.text:0000000141F0BF7F                 mulss   xmm7, xmm6
.text:0000000141F0BF83                 movaps  xmm9, xmm2
.text:0000000141F0BF87                 mulss   xmm0, xmm3
.text:0000000141F0BF8B                 mulss   xmm2, xmm6
.text:0000000141F0BF8F                 movaps  xmm8, xmm4
.text:0000000141F0BF93                 mulss   xmm8, xmm3
.text:0000000141F0BF98                 movss   xmm3, dword ptr cs:ymmword_1471D8C38
.text:0000000141F0BFA0                 mulss   xmm9, xmm5
.text:0000000141F0BFA5                 subss   xmm8, xmm2
.text:0000000141F0BFAA                 mulss   xmm4, xmm5
.text:0000000141F0BFAE                 subss   xmm9, xmm0
.text:0000000141F0BFB3                 subss   xmm7, xmm4
.text:0000000141F0BFB7                 movaps  xmm2, xmm8
.text:0000000141F0BFBB                 mulss   xmm2, xmm8
.text:0000000141F0BFC0                 movaps  xmm0, xmm9
.text:0000000141F0BFC4                 mulss   xmm0, xmm9
.text:0000000141F0BFC9                 movaps  xmm1, xmm7
.text:0000000141F0BFCC                 mulss   xmm1, xmm7
.text:0000000141F0BFD0                 addss   xmm2, xmm0
.text:0000000141F0BFD4                 addss   xmm2, xmm1
.text:0000000141F0BFD8                 comiss  xmm2, xmm3
.text:0000000141F0BFDB                 jb      short loc_141F0C003
.text:0000000141F0BFDD                 movss   xmm0, dword ptr cs:ymmword_1471C46C0
.text:0000000141F0BFE5                 xorps   xmm1, xmm1
.text:0000000141F0BFE8                 sqrtss  xmm1, xmm2
.text:0000000141F0BFEC                 comiss  xmm1, xmm3
.text:0000000141F0BFEF                 divss   xmm0, xmm1
.text:0000000141F0BFF3                 mulss   xmm7, xmm0
.text:0000000141F0BFF7                 mulss   xmm8, xmm0
.text:0000000141F0BFFC                 mulss   xmm9, xmm0
.text:0000000141F0C001                 jnb     short loc_141F0C013
.text:0000000141F0C003
.text:0000000141F0C003 loc_141F0C003:                          ; CODE XREF: sub_141F0BEA0+13B↑j
.text:0000000141F0C003                 movsd   xmm1, qword ptr cs:ymmword_1471C46F8+18h
.text:0000000141F0C00B                 xorps   xmm0, xmm0
.text:0000000141F0C00E                 xorps   xmm2, xmm2
.text:0000000141F0C011                 jmp     short loc_141F0C01E
.text:0000000141F0C013 ; ---------------------------------------------------------------------------
.text:0000000141F0C013
.text:0000000141F0C013 loc_141F0C013:                          ; CODE XREF: sub_141F0BEA0+161↑j
.text:0000000141F0C013                 cvtps2pd xmm1, xmm7
.text:0000000141F0C016                 cvtps2pd xmm0, xmm8
.text:0000000141F0C01A                 cvtps2pd xmm2, xmm9
.text:0000000141F0C01E
.text:0000000141F0C01E loc_141F0C01E:                          ; CODE XREF: sub_141F0BEA0+171↑j
.text:0000000141F0C01E                 movsd   qword ptr [r10], xmm2
.text:0000000141F0C023                 mov     rax, r10
.text:0000000141F0C026                 movsd   qword ptr [r10+8], xmm0
.text:0000000141F0C02C                 movsd   qword ptr [r10+10h], xmm1
.text:0000000141F0C032                 movaps  xmm8, [rsp+48h+var_38]
.text:0000000141F0C038                 movaps  xmm7, [rsp+48h+var_28]
.text:0000000141F0C03D                 movaps  xmm6, [rsp+48h+var_18]
.text:0000000141F0C042                 movaps  xmm9, [rsp+48h+var_48]
.text:0000000141F0C047                 add     rsp, 48h
.text:0000000141F0C04B                 retn
.text:0000000141F0C04C ; ---------------------------------------------------------------------------
.text:0000000141F0C04C
.text:0000000141F0C04C loc_141F0C04C:                          ; CODE XREF: sub_141F0BEA0+B↑j
.text:0000000141F0C04C                 xor     eax, eax
.text:0000000141F0C04E                 mov     [rdx], rax
.text:0000000141F0C051                 mov     [rdx+8], rax
.text:0000000141F0C055                 mov     rax, 3FF0000000000000h
.text:0000000141F0C05F                 mov     [rdx+10h], rax
.text:0000000141F0C063                 mov     rax, r10
.text:0000000141F0C066                 add     rsp, 48h
.text:0000000141F0C06A                 retn
.text:0000000141F0C06A sub_141F0BEA0   endp
.text:0000000141F0C06A
.text:0000000141F0C06A ; ---------------------------------------------------------------------------
```

</details>
