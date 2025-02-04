#include <filesystem>
#include <FreeImagePlus.h>
#include "assetpipeline/loaders/TextureLoader.hxx"

namespace playground::editor::assetpipeline::loaders::textureloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawTextureData {
        FreeImage_Initialise();
        fipImage image;

        auto type = FreeImage_GetFileType(path.string().c_str());
        auto result = FreeImage_Load(type, path.string().c_str(), type == FIF_PNG ? PNG_IGNOREGAMMA : 0);
        int width = FreeImage_GetWidth(result);
        int height = FreeImage_GetHeight(result);
        auto bbp = FreeImage_GetBPP(result);

        FreeImage_FlipVertical(result);
        RGBQUAD pix;
        FreeImage_GetPixelColor(result, 60, 0, &pix);

        int channels = bbp / 8;

        assetloader::RawTextureData asset;
        asset.Width = width;
        asset.Height = height;
        asset.Channels = channels;
        asset.Pixels = std::vector<uint8_t>(width * height * channels);

        memcpy(asset.Pixels.data(), (uint8_t*)FreeImage_GetBits(result), (width) * (height)*channels);

        FreeImage_Unload(result);
        FreeImage_DeInitialise();

        return asset;
    }
}
