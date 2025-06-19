#define NOMINMAX
#include <filesystem>
#include <FreeImagePlus.h>
#include "assetpipeline/loaders/TextureLoader.hxx"
#include <compressonator.h>

namespace playground::editor::assetpipeline::loaders::textureloader {
    auto LoadFromFile(
        std::filesystem::path path,
        bool isNormal
    ) -> assetloader::RawTextureData {

        CMP_InitFramework();
        CMP_InitializeBCLibrary();

        CMP_MipSet mipmapInput;
        memset(&mipmapInput, 0, sizeof(CMP_MipSet));
        auto cmp_status = CMP_LoadTexture(path.string().c_str(), &mipmapInput);
        if (cmp_status != CMP_OK) {
            std::printf("Error %d: Loading source file!\n", cmp_status);
            exit(1);
        }

        if (mipmapInput.m_nMipLevels <= 1)
        {
            CMP_INT requestLevel = 14;

            CMP_INT nMinSize = CMP_CalcMinMipSize(mipmapInput.m_nHeight, mipmapInput.m_nWidth, 10);
            CMP_GenerateMIPLevels(&mipmapInput, nMinSize);
        }

        KernelOptions kernel_options;
        memset(&kernel_options, 0, sizeof(KernelOptions));

        if (mipmapInput.m_format == CMP_FORMAT_RGB_888) {
            mipmapInput.m_nChannels = 3;
            kernel_options.format = CMP_FORMAT_BC5;
        }
        else if (mipmapInput.m_format == CMP_FORMAT_RGBA_8888) {
            mipmapInput.m_nChannels = 4;
            kernel_options.format = CMP_FORMAT_BC3;
        }
        else {
            std::printf("Unsupported channel count: %d\n", mipmapInput.m_nChannels);
            exit(1);
        }
        kernel_options.fquality = 0.5f;
        kernel_options.threads = 8;

        // Enable punch through alpha setting
        kernel_options.bc15.useAlphaThreshold = true;
        kernel_options.bc15.alphaThreshold = 128;

        if (isNormal) {
            kernel_options.bc15.useChannelWeights = false;
        } else {
            // Enable setting channel weights
            kernel_options.bc15.useChannelWeights = true;
            kernel_options.bc15.channelWeights[0] = 0.3086f;
            kernel_options.bc15.channelWeights[1] = 0.6094f;
            kernel_options.bc15.channelWeights[2] = 0.0820f;
        }

        CMP_MipSet mipmapOutput;
        memset(&mipmapOutput, 0, sizeof(CMP_MipSet));

        cmp_status = CMP_ProcessTexture(&mipmapInput, &mipmapOutput, kernel_options, [](auto progress, auto user1, auto user2) {
            std::cout << "Compressing texture: " << progress << "%" << std::endl;
            return true;
        });
        if (cmp_status != CMP_OK) {
            std::printf("Error %d: Compressing texture!\n", cmp_status);
            exit(1);
        }

        std::vector<std::vector<uint8_t>> compressedMips;

        for (int mipLevel = 0; mipLevel < mipmapOutput.m_nMipLevels; mipLevel++) {
            CMP_MipLevel* mip = &mipmapOutput.m_pMipLevelTable[mipLevel][0];
            if (mip && mip->m_pbData && mip->m_dwLinearSize > 0) {
                std::vector<uint8_t> mipData(mip->m_pbData, mip->m_pbData + mip->m_dwLinearSize);
                compressedMips.push_back(std::move(mipData));
            }
        }

        assetloader::RawTextureData asset;
        asset.MipMaps = compressedMips;
        asset.Width = mipmapInput.dwWidth;
        asset.Height = mipmapInput.dwHeight;
        asset.Channels = mipmapInput.m_nChannels;

        CMP_FreeMipSet(&mipmapInput);
        CMP_FreeMipSet(&mipmapOutput);

        return asset;
    }
}
