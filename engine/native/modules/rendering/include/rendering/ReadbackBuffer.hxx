#pragma once

namespace playground::rendering {
    class ReadbackBuffer {
    public:
        virtual ~ReadbackBuffer() = default;

        virtual void Read(void* result, size_t* numRead) = 0;
    };
}
