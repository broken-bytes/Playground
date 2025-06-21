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
        FreeImage_Initialise();

        auto* img = FreeImage_Load(FREE_IMAGE_FORMAT::FIF_PNG, path.string().c_str());

        CMP_MipSet MipSetIn;
        memset(&MipSetIn, 0, sizeof(CMP_MipSet));
        auto cmp_status = CMP_LoadTexture(path.string().c_str(), &MipSetIn);
        if (cmp_status != CMP_OK) {
            std::printf("Error %d: Loading source file!\n", cmp_status);
            exit(1);
        }

        if (MipSetIn.m_nMipLevels <= 1)
        {
            CMP_INT requestLevel = 10;

            CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, 10);

            CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
        }

        KernelOptions   kernel_options;
        memset(&kernel_options, 0, sizeof(KernelOptions));

        kernel_options.format = CMP_FORMAT_BC3;   // Set the format to process
        kernel_options.fquality = 0.7f;     // Set the quality of the result
        kernel_options.threads = 24;            // Auto setting

        if (kernel_options.format == CMP_FORMAT_BC3)
        {
            // Enable punch through alpha setting
            kernel_options.bc15.useAlphaThreshold = true;
            kernel_options.bc15.alphaThreshold = 128;

            // Enable setting channel weights
            kernel_options.bc15.useChannelWeights = true;
            kernel_options.bc15.channelWeights[0] = 0.3086f;
            kernel_options.bc15.channelWeights[1] = 0.6094f;
            kernel_options.bc15.channelWeights[2] = 0.0820f;
        }

        CMP_MipSet MipSetCmp;
        memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

        cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, nullptr);
        if (cmp_status != CMP_OK) {
            std::printf("Error %d: Loading source file!\n", cmp_status);
            exit(1);
        }

        assetloader::RawTextureData asset;

        for (int x = 0; x < MipSetCmp.m_nMipLevels; x++) {
            CMP_MipLevel* mip = MipSetCmp.m_pMipLevelTable[x];
            if (!mip || !mip->m_pbData || mip->m_dwLinearSize == 0) continue;

            std::vector<uint8_t> mipData(mip->m_pbData, mip->m_pbData + mip->m_dwLinearSize);
            asset.MipMaps.push_back(std::move(mipData));
        }

        asset.Width = MipSetCmp.m_nWidth;
        asset.Height = MipSetCmp.m_nHeight;
        asset.Channels = 4;

        CMP_FreeMipSet(&MipSetIn);
        CMP_FreeMipSet(&MipSetCmp);

        return asset;
    }
}
