#define WIN32_LEAN_AND_MEAN
#define POINTER_64 __ptr64 // Thêm dòng này để định nghĩa con trỏ 64-bit

#include <windows.h>

// Định nghĩa thủ công PVOID64 nếu nó chưa tồn tại
#ifndef PVOID64
typedef void* POINTER_64 PVOID64;
#endif

#include <d3dx9shader.h>
#include <cstdio>

static void PrintUsage()
{
    std::puts("Usage: ShaderBuilder <input.vsh> <output.vso>");
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        PrintUsage();
        return 1;
    }

    const char* input = argv[1];
    const char* output = argv[2];

    ID3DXBuffer* code = NULL;
    ID3DXBuffer* errors = NULL;

    HRESULT hr = D3DXAssembleShaderFromFileA(input, NULL, NULL, 0, &code, &errors);
    if (FAILED(hr) || !code)
    {
        if (errors)
            std::fprintf(stderr, "%s\n", (const char*)errors->GetBufferPointer());
        else
            std::fprintf(stderr, "D3DXAssembleShaderFromFileA failed: 0x%08lx\n", hr);

        if (errors) errors->Release();
        return 2;
    }

    FILE* fp = NULL;
    if (fopen_s(&fp, output, "wb") != 0 || !fp)
    {
        std::fprintf(stderr, "Cannot open output file: %s\n", output);
        code->Release();
        if (errors) errors->Release();
        return 3;
    }

    const void* ptr = code->GetBufferPointer();
    const size_t size = code->GetBufferSize();
    const size_t written = std::fwrite(ptr, 1, size, fp);
    std::fclose(fp);

    code->Release();
    if (errors) errors->Release();

    if (written != size)
    {
        std::fprintf(stderr, "Short write while saving %s\n", output);
        return 4;
    }

    std::printf("Built %s -> %s (%zu bytes)\n", input, output, size);
    return 0;
}
