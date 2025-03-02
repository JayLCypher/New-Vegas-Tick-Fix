#pragma once
#ifndef SAFEWRITE_H
#define SAFEWRITE_H

void __stdcall SafeWrite8(UInt32 addr, UInt32 data);
void __stdcall SafeWrite16(UInt32 addr, UInt32 data);
void __stdcall SafeWrite32(UInt32 addr, UInt32 data);
void __stdcall SafeWriteBuf(UInt32 addr, const void* data, UInt32 len);

// 5 bytes
void __stdcall WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt);
void __stdcall WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt);

// 6 bytes
void WriteRelJnz(UInt32 jumpSrc, UInt32 jumpTgt);
void WriteRelJle(UInt32 jumpSrc, UInt32 jumpTgt);

#endif