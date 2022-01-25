
#pragma once

#include "Core/Core.h"

struct Buffer
{
    byte* Data;
    uint32_t Size;

    Buffer()
        : Data(nullptr), Size(0)
    {
    }

    Buffer(byte* data, uint32_t size)
        : Data(data), Size(size)
    {
    }

    static Buffer Copy(void* data, uint32_t size)
    {
        Buffer buffer;
        buffer.Allocate(size);
        memcpy(buffer.Data, data, size);
        return buffer;
    }

    void Allocate(uint32_t size)
    {
        delete[] Data;
        Data = nullptr;

        if (size == 0)
            return;

        Data = new byte[size];
        Size = size;
    }

    template<typename T>
    T& Read(uint32_t offset = 0)
    {
        return *(T*)((byte*)Data + offset);
    }

    byte* ReadBytes(uint32_t size, uint32_t offset)
    {
        JN_ASSERT(offset + size <= Size, "Buffer overflow!");
        byte* buffer = new byte[size];
        memcpy(buffer, (byte*)Data + offset, size);
        return buffer;
    }

    void ZeroInitialize()
    {
        if (Data)
            memset(Data, 0, Size);
    }

    void Write(void* data, uint32_t size, uint32_t offset = 0)
    {
        memcpy(Data + offset, data, size);
    }

    operator bool() const
    {
        return Data;
    }

    byte& operator[](int index)
    {
        return Data[index];
    }

    byte operator[](int index) const
    {
        return Data[index];
    }

    template<typename T>
    T* As()
    {
        return (T*)Data;
    }

    inline uint32_t GetSize() const { return Size; }
};
