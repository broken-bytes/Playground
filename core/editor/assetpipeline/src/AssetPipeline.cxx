#include <fstream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <zip.h>
#include <rendering/Mesh.hxx>
#include <assetloader/AssetLoader.hxx>
#include <assetloader/RawMeshData.hxx>
#include "assetpipeline/assetpipeline.hxx"
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <stdexcept>
#include <vector>
#include <wrl.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include <iostream>
#include <sstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


using namespace playground::assetloader;

namespace playground::editor::assetpipeline {
    void XOR_Encrypt(std::vector<uint8_t>& data, uint8_t key = 0xAB) {
        for (auto& byte : data) {
            byte ^= key;
        }
    }

    inline Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderWithDxc(
        const std::string& shaderCode, const std::wstring& entryPoint, const std::wstring& target)
    {
        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
        Microsoft::WRL::ComPtr<IDxcResult> result;

        // Initialize DXC Compiler and Utility
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

        // Create a blob from the shader source string
        utils->CreateBlob(shaderCode.c_str(), (uint32_t)shaderCode.size(), DXC_CP_UTF8, &sourceBlob);

        // Create a DxcBuffer to pass to Compile()
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        sourceBuffer.Encoding = DXC_CP_UTF8;

        // Define compiler arguments
        const wchar_t* arguments[] = {
            L"-E", entryPoint.c_str(),
            L"-T", target.c_str(),
            L"-O3",
            L"-Zi"
        };

        // Compile the shader
        HRESULT hr = compiler->Compile(
            &sourceBuffer,
            arguments, _countof(arguments),
            nullptr,
            IID_PPV_ARGS(&result)
        );

        if (FAILED(hr)) {
            throw std::runtime_error("DXC Compilation failed.");
        }

        // Retrieve compiled shader blob
        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        if (FAILED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
            throw std::runtime_error("Failed to retrieve error messages.");
        }
        if (errors && errors->GetStringLength() > 0) {
            OutputDebugStringA(errors->GetStringPointer()); // Print error messages
        }

        Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
        return shaderBlob;
    }


    auto Init() -> void
    {
    }

    auto CreateDirectory(const std::string_view path) -> void {
        create_directory(std::filesystem::path(path));
    }

    auto CreateArchive(const std::filesystem::path& path) -> int8_t {
        zip_t* archive;
        int err;

        if ((archive = zip_open(path.string().c_str(), ZIP_CREATE, &err)) == nullptr) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            zip_error_fini(&error);

            return -1;
        }

        zip_close(archive);

        return 0;
    }

    auto SaveBufferToArchive(const std::filesystem::path& path, const std::string name, std::vector<uint8_t>& buffer) -> int8_t {
        zip_t* archive;
        int err;

        if ((archive = zip_open(path.string().c_str(), ZIP_CREATE, &err)) == nullptr) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            std::cerr << "zip_open failed (" << err << "): "
                << zip_error_strerror(&error) << "\n";
            zip_error_fini(&error);

            return -1;
        }

        zip_source_t* s = zip_source_buffer(archive, buffer.data(), buffer.size(), 0);
        if (!s) {
            std::cerr << "zip_source_buffer failed: " << zip_strerror(archive) << "\n";
            zip_discard(archive);
            return -1;
        }

        if (zip_file_add(archive, name.data(), s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) >= 0) {
        }
        else {
            zip_source_free(s);
            std::cerr << "zip_file_add failed for '" << name << "': "
                << zip_strerror(archive) << "\n";

            return -1;
        }
        
        if (zip_close(archive) < 0) {
            std::cerr << "zip_close failed: " << zip_strerror(archive) << "\n";
            return -1;
        }

        return 0;
    }

    auto CookModel(std::vector<assetloader::RawMeshData> meshData) -> std::vector<uint8_t> {
        std::ostringstream oss(std::ios::binary);
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(meshData);
        }

        // Convert the output to a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }

    auto CookMaterial(assetloader::RawMaterialData materialData) -> std::vector<uint8_t> {
        std::ostringstream oss;
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(materialData);
        }

        // Convert the serialized output to a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }


    auto CookTexture(assetloader::RawTextureData textureData) -> std::vector<uint8_t> {
        std::ostringstream oss(std::ios::binary);
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(textureData);
        }

        // Convert the serialised output into a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }

    auto CookShader(std::string code) -> std::vector<uint8_t> {
        RawShaderData shaderData;

        Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
        vertexShaderBlob = CompileShaderWithDxc(code, L"VSMain", L"vs_6_0");
        size_t len = vertexShaderBlob->GetBufferSize();
        DxcBuffer vertexShader = {};
        vertexShader.Ptr = vertexShaderBlob->GetBufferPointer();
        vertexShader.Size = vertexShaderBlob->GetBufferSize();
        vertexShader.Encoding = DXC_CP_ACP;
        shaderData.vertexShader = std::string((char*)vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());

        Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
        pixelShaderBlob = CompileShaderWithDxc(code, L"PSMain", L"ps_6_0");
        DxcBuffer pixelShader = {};
        pixelShader.Ptr = pixelShaderBlob->GetBufferPointer();
        pixelShader.Size = pixelShaderBlob->GetBufferSize();
        pixelShader.Encoding = DXC_CP_ACP;
        shaderData.pixelShader = std::string((char*)pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());

        Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

        auto shaders = std::vector { std::pair { vertexShaderBlob, vertexShader }, std::pair { pixelShaderBlob, pixelShader } };

        for (const auto& (shader) : shaders) {
            if (!shader.first) {
                throw std::runtime_error("Shader compilation failed.");
            }
            Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
            dxcUtils->CreateReflection(
                &shader.second,
                __uuidof(ID3D12ShaderReflection),
                (void**)&reflection
            );
            // Use reflection normally
            D3D12_SHADER_DESC shaderDesc;
            reflection->GetDesc(&shaderDesc);

            for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC bindDesc;
                reflection->GetResourceBindingDesc(i, &bindDesc);

                std::string name = std::string(bindDesc.Name, strlen(bindDesc.Name));
                std::string typeStr;
                switch (bindDesc.Type)
                {
                case D3D_SIT_TEXTURE:
                    if (shaderData.properties.textures.contains(name)) {
                        continue;
                    }
                    shaderData.properties.textures.insert({ name, (uint8_t)bindDesc.BindPoint });
                    break;
                default: break;
                }
            }

            for (UINT cbIndex = 0; cbIndex < shaderDesc.ConstantBuffers; ++cbIndex)
            {
                ID3D12ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(cbIndex);

                D3D12_SHADER_BUFFER_DESC cbDesc;
                cb->GetDesc(&cbDesc);                    // name, size, variable count

                if (strcmp(cbDesc.Name, "MaterialData")) {
                    continue; // Skip non-material constant buffers
                }

                // Walk variables inside the CB
                for (UINT v = 0; v < cbDesc.Variables; ++v)
                {
                    ID3D12ShaderReflectionVariable* var = cb->GetVariableByIndex(v);

                    D3D12_SHADER_TYPE_DESC tDesc;
                    var->GetType()->GetDesc(&tDesc);

                    D3D12_SHADER_VARIABLE_DESC vDesc;
                    var->GetDesc(&vDesc);

                    std::string name = std::string(vDesc.Name, strlen(vDesc.Name));
                    uint8_t offset = vDesc.StartOffset;
                    uint8_t columns = tDesc.Columns;

                    if (shaderData.properties.variables.contains(name)) {
                        continue;
                    }

                    RawShaderData::ShaderPropertyType propType;

                    switch (tDesc.Type) {
                    case D3D_SVT_FLOAT:
                        if (columns == 2) {
                            propType = RawShaderData::ShaderPropertyType::Vec2;
                        }
                        else if (columns == 3) {
                            propType = RawShaderData::ShaderPropertyType::Vec3;
                        }
                        else if (columns == 4) {
                            propType = RawShaderData::ShaderPropertyType::Vec4;
                        }
                        else {
                            propType = RawShaderData::ShaderPropertyType::Float;
                        }
                        break;
                    case D3D_SVT_INT:
                        if (columns == 2) {
                            propType = RawShaderData::ShaderPropertyType::IVec2;
                        }
                        else if (columns == 3) {
                            propType = RawShaderData::ShaderPropertyType::IVec3;
                        }
                        else if (columns == 4) {
                            propType = RawShaderData::ShaderPropertyType::IVec4;
                        }
                        else {
                            propType = RawShaderData::ShaderPropertyType::Int;
                        }
                        break;
                    case D3D_SVT_UINT:
                        propType = RawShaderData::ShaderPropertyType::UInt;
                        break;
                    case D3D_SVT_BOOL:
                        propType = RawShaderData::ShaderPropertyType::Bool;
                        break;
                    default:
                        continue;
                    }

                    shaderData.properties.variables.insert({ name, {.offset = offset, .type = propType } });
                }
            }
        }

        std::ostringstream oss;
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(shaderData);
        }

        // Convert the serialized output to a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }

    auto CookPhysicsMaterial(assetloader::RawPhysicsMaterialData materialData) -> std::vector<uint8_t> {
        std::ostringstream oss;
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(materialData);
        }

        // Convert the serialized output to a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }
}
