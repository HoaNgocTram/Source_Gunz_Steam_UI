#include "stdafx.h"
#include "AnimatedGifTexture.h"
#include "gif_load.h"
#include "MDebug.h"

// WebP support
#include <webp/decode.h>
#include <webp/demux.h>
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "libwebpdemux.lib")

static void emojilog(const char* fmt, ...) {
    static FILE* fp = nullptr;
    if (!fp) {
        fp = fopen("emojis.log", "a");
        if (!fp) return;
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fflush(fp);
}

AnimatedGifTexture::AnimatedGifTexture()
    : m_nWidth(0)
    , m_nHeight(0)
    , m_nTotalDurationMs(0)
    , m_pd3dDevice(nullptr)
    , m_dwStartTime(0)
{
}

AnimatedGifTexture::~AnimatedGifTexture()
{
    Destroy();
}

bool AnimatedGifTexture::CreateFromMemory(const uint8_t* pData, int nDataSize, LPDIRECT3DDEVICE9 pd3dDevice)
{
    if (!pData || nDataSize <= 0 || !pd3dDevice)
        return false;

    Destroy();

    m_pd3dDevice = pd3dDevice;

    // Keep raw data for device reset
    m_RawGifData.assign(pData, pData + nDataSize);

    AnimatedTextureFormat fmt = DetectFormat(pData, nDataSize);
    bool bSuccess = false;

    switch (fmt)
    {
    case ATF_GIF:
        bSuccess = CreateTexturesFromGifData();
        break;
    case ATF_WEBP:
        bSuccess = CreateTexturesFromWebPData();
        break;
    default:
        emojilog("AnimatedGifTexture: Unknown format (not GIF or WebP)\n");
        break;
    }

    if (!bSuccess)
    {
        Destroy();
        return false;
    }

    m_dwStartTime = timeGetTime();
    return true;
}

bool AnimatedGifTexture::CreateTexturesFromGifData()
{
    if (m_RawGifData.empty() || !m_pd3dDevice)
        return false;

    // Decode GIF
    GifFileData gif;
    if (!GifLoad(&gif, m_RawGifData.data(), (int)m_RawGifData.size()))
    {
        emojilog("AnimatedGifTexture: GIF decode failed\n");
        return false;
    }

    m_nWidth = gif.width;
    m_nHeight = gif.height;

    int cumulative = 0;

    for (int i = 0; i < gif.frameCount; i++)
    {
        // Create D3D texture for this frame
        LPDIRECT3DTEXTURE9 pTex = nullptr;
        //HRESULT hr = m_pd3dDevice->CreateTexture(
        //    m_nWidth, m_nHeight, 1,
        //    0,                    // no special usage
        //    D3DFMT_A8R8G8B8,     // 32-bit ARGB
        //    D3DPOOL_MANAGED,      // managed pool for simplicity
        //    &pTex, nullptr
        //);
        IDirect3DDevice9Ex* pEx = nullptr;
        m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pEx));
        bool bIsExMode = (pEx != nullptr);
        if (pEx) pEx->Release();

        HRESULT hr = m_pd3dDevice->CreateTexture(
            m_nWidth, m_nHeight, 1,
            bIsExMode ? D3DUSAGE_DYNAMIC : 0,
            D3DFMT_A8R8G8B8,
            bIsExMode ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
            &pTex, nullptr
        );


        if (FAILED(hr) || !pTex)
        {
            emojilog("AnimatedGifTexture: CreateTexture failed for frame %d\n", i);
            GifFree(&gif);
            return false;
        }

        // Lock texture and copy RGBA -> ARGB pixel data
        D3DLOCKED_RECT lr;
        hr = pTex->LockRect(0, &lr, nullptr, 0);
        if (FAILED(hr))
        {
            pTex->Release();
            emojilog("AnimatedGifTexture: LockRect failed for frame %d\n", i);
            GifFree(&gif);
            return false;
        }

        const uint8_t* src = gif.frames[i].pixels;
        uint8_t* dst = (uint8_t*)lr.pBits;

        for (int y = 0; y < m_nHeight; y++)
        {
            uint32_t* dstRow = (uint32_t*)(dst + y * lr.Pitch);
            const uint8_t* srcRow = src + y * m_nWidth * 4;

            for (int x = 0; x < m_nWidth; x++)
            {
                uint8_t r = srcRow[x * 4 + 0];
                uint8_t g = srcRow[x * 4 + 1];
                uint8_t b = srcRow[x * 4 + 2];
                uint8_t a = srcRow[x * 4 + 3];
                // D3DFMT_A8R8G8B8: ARGB layout
                dstRow[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        pTex->UnlockRect(0);

        // Build frame entry
        Frame frame;
        frame.pTexture = pTex;
        frame.delayMs = gif.frames[i].delayMs;
        cumulative += frame.delayMs;
        frame.cumulativeMs = cumulative;
        m_Frames.push_back(frame);
    }

    m_nTotalDurationMs = cumulative;

    GifFree(&gif);

    emojilog("AnimatedGifTexture: Loaded %d frames (%dx%d, total %dms)\n",
        (int)m_Frames.size(), m_nWidth, m_nHeight, m_nTotalDurationMs);

    return true;
}

bool AnimatedGifTexture::CreateTexturesFromWebPData()
{
    if (m_RawGifData.empty() || !m_pd3dDevice)
        return false;

    WebPData webpData;
    webpData.bytes = m_RawGifData.data();
    webpData.size = m_RawGifData.size();

    WebPDemuxer* demux = WebPDemux(&webpData);
    if (!demux)
    {
        emojilog("AnimatedGifTexture: WebP demux failed\n");
        return false;
    }

    m_nWidth = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    m_nHeight = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
    int frameCount = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);

    if (m_nWidth <= 0 || m_nHeight <= 0 || frameCount <= 0)
    {
        emojilog("AnimatedGifTexture: WebP invalid dimensions or no frames\n");
        WebPDemuxDelete(demux);
        return false;
    }

    // Canvas for compositing (RGBA)
    std::vector<uint8_t> canvas(m_nWidth * m_nHeight * 4, 0);
    int cumulative = 0;

    WebPIterator iter;
    if (!WebPDemuxGetFrame(demux, 1, &iter))
    {
        emojilog("AnimatedGifTexture: WebP cannot get first frame\n");
        WebPDemuxDelete(demux);
        return false;
    }

    do
    {
        // Decode this frame fragment
        int frameW = 0, frameH = 0;
        uint8_t* frameRGBA = WebPDecodeRGBA(iter.fragment.bytes, iter.fragment.size, &frameW, &frameH);
        if (!frameRGBA)
        {
            emojilog("AnimatedGifTexture: WebP frame decode failed\n");
            WebPDemuxReleaseIterator(&iter);
            WebPDemuxDelete(demux);
            return false;
        }

        // Composite onto canvas at (x_offset, y_offset)
        bool bNoBlend = (iter.blend_method == WEBP_MUX_NO_BLEND);

        for (int y = 0; y < frameH && (y + iter.y_offset) < m_nHeight; y++)
        {
            for (int x = 0; x < frameW && (x + iter.x_offset) < m_nWidth; x++)
            {
                int srcIdx = (y * frameW + x) * 4;
                int dstIdx = ((y + iter.y_offset) * m_nWidth + (x + iter.x_offset)) * 4;

                if (bNoBlend)
                {
                    // Direct overwrite — no blending
                    memcpy(&canvas[dstIdx], &frameRGBA[srcIdx], 4);
                }
                else
                {
                    uint8_t srcA = frameRGBA[srcIdx + 3];
                    if (srcA == 255)
                    {
                        memcpy(&canvas[dstIdx], &frameRGBA[srcIdx], 4);
                    }
                    else if (srcA > 0)
                    {
                        uint8_t dstA = canvas[dstIdx + 3];
                        for (int c = 0; c < 3; c++)
                        {
                            canvas[dstIdx + c] = (uint8_t)((frameRGBA[srcIdx + c] * srcA + canvas[dstIdx + c] * (255 - srcA)) / 255);
                        }
                        canvas[dstIdx + 3] = (uint8_t)(srcA + dstA * (255 - srcA) / 255);
                    }
                }
            }
        }

        WebPFree(frameRGBA);

        // Create D3D texture from canvas
        LPDIRECT3DTEXTURE9 pTex = nullptr;
        HRESULT hr = m_pd3dDevice->CreateTexture(
            m_nWidth, m_nHeight, 1, 0,
            D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
            &pTex, nullptr);

        if (FAILED(hr) || !pTex)
        {
            emojilog("AnimatedGifTexture: WebP CreateTexture failed\n");
            WebPDemuxReleaseIterator(&iter);
            WebPDemuxDelete(demux);
            return false;
        }

        D3DLOCKED_RECT lr;
        hr = pTex->LockRect(0, &lr, nullptr, 0);
        if (FAILED(hr))
        {
            pTex->Release();
            WebPDemuxReleaseIterator(&iter);
            WebPDemuxDelete(demux);
            return false;
        }

        for (int y = 0; y < m_nHeight; y++)
        {
            uint32_t* dstRow = (uint32_t*)((uint8_t*)lr.pBits + y * lr.Pitch);
            const uint8_t* srcRow = &canvas[y * m_nWidth * 4];
            for (int x = 0; x < m_nWidth; x++)
            {
                uint8_t r = srcRow[x * 4 + 0];
                uint8_t g = srcRow[x * 4 + 1];
                uint8_t b = srcRow[x * 4 + 2];
                uint8_t a = srcRow[x * 4 + 3];
                dstRow[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

        pTex->UnlockRect(0);

        Frame frame;
        frame.pTexture = pTex;
        frame.delayMs = iter.duration > 0 ? iter.duration : 100;
        cumulative += frame.delayMs;
        frame.cumulativeMs = cumulative;
        m_Frames.push_back(frame);

        // Handle dispose method
        if (iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND)
        {
            for (int y = 0; y < iter.height && (y + iter.y_offset) < m_nHeight; y++)
            {
                for (int x = 0; x < iter.width && (x + iter.x_offset) < m_nWidth; x++)
                {
                    int idx = ((y + iter.y_offset) * m_nWidth + (x + iter.x_offset)) * 4;
                    memset(&canvas[idx], 0, 4);
                }
            }
        }

    } while (WebPDemuxNextFrame(&iter));

    WebPDemuxReleaseIterator(&iter);
    WebPDemuxDelete(demux);

    m_nTotalDurationMs = cumulative;

    // Static WebP (1 frame) is fine — just 1 frame with infinite display
    if (m_Frames.size() == 1 && m_nTotalDurationMs <= 0)
        m_nTotalDurationMs = 1000;

    emojilog("AnimatedGifTexture: Loaded WebP %d frames (%dx%d, total %dms)\n",
        (int)m_Frames.size(), m_nWidth, m_nHeight, m_nTotalDurationMs);

    return true;
}

void AnimatedGifTexture::Destroy()
{
    for (auto& f : m_Frames)
    {
        if (f.pTexture)
        {
            f.pTexture->Release();
            f.pTexture = nullptr;
        }
    }
    m_Frames.clear();
    m_RawGifData.clear();
    m_nWidth = 0;
    m_nHeight = 0;
    m_nTotalDurationMs = 0;
    m_pd3dDevice = nullptr;
    m_dwStartTime = 0;
}

int AnimatedGifTexture::GetCurrentFrame() const
{
    if (m_Frames.empty() || m_nTotalDurationMs <= 0)
        return 0;

    DWORD elapsed = timeGetTime() - m_dwStartTime;
    int timeInLoop = (int)(elapsed % (DWORD)m_nTotalDurationMs);

    // Binary search for the right frame
    int lo = 0, hi = (int)m_Frames.size() - 1;
    while (lo < hi)
    {
        int mid = (lo + hi) / 2;
        if (m_Frames[mid].cumulativeMs <= timeInLoop)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

LPDIRECT3DTEXTURE9 AnimatedGifTexture::GetCurrentFrameTexture() const
{
    if (m_Frames.empty()) return nullptr;
    int idx = GetCurrentFrame();
    return m_Frames[idx].pTexture;
}

void AnimatedGifTexture::OnLostDevice()
{
    // D3DPOOL_MANAGED textures survive device lost,
    // but if using DEFAULT pool, release here
    // Currently using MANAGED, so nothing needed
    if (g_isDirect3D9ExEnabled)
    {
        // Với bản Ex và POOL_DEFAULT, ta bắt buộc phải Release texture ở đây
        Destroy();
    }
}

void AnimatedGifTexture::OnResetDevice()
{
    // If we need to recreate (e.g. switched to DEFAULT pool):
    // Destroy();
    // CreateTexturesFromGifData();
    // m_dwStartTime = timeGetTime();
    if (g_isDirect3D9ExEnabled) 
    {
        // 1. Giải phóng texture cũ đã bị hỏng do Reset
        Destroy(); 

        // 2. Nếu còn giữ dữ liệu gốc, nạp lại vào GPU
        if (!m_RawGifData.empty()) 
        {
            AnimatedTextureFormat fmt = DetectFormat(m_RawGifData.data(), (int)m_RawGifData.size());
            bool bSuccess = false;

            if (fmt == ATF_GIF) bSuccess = CreateTexturesFromGifData();
            else if (fmt == ATF_WEBP) bSuccess = CreateTexturesFromWebPData();

            if (bSuccess) {
                m_dwStartTime = timeGetTime();
                // emojilog("AnimatedGifTexture: Restored after ResetDevice\n");
            }
        }
    }
}

AnimatedTextureFormat AnimatedGifTexture::DetectFormat(const uint8_t* pData, int nSize)
{
    if (nSize >= 4 && pData[0] == 'G' && pData[1] == 'I' && pData[2] == 'F')
        return ATF_GIF;
    if (nSize >= 12 && pData[0] == 'R' && pData[1] == 'I' && pData[2] == 'F' && pData[3] == 'F'
        && pData[8] == 'W' && pData[9] == 'E' && pData[10] == 'B' && pData[11] == 'P')
        return ATF_WEBP;
    return ATF_UNKNOWN;
}
