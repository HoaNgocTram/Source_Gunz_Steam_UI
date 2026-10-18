#pragma once
#ifndef ANIMATED_GIF_TEXTURE_H
#define ANIMATED_GIF_TEXTURE_H

/**
 * AnimatedGifTexture — Manages animated GIF as D3D9 textures
 *
 * Each frame of the GIF becomes a separate D3D texture.
 * The class handles frame timing and returns the correct texture
 * for the current time via GetCurrentFrame().
 *
 * Usage:
 *   AnimatedGifTexture gif;
 *   gif.CreateFromFile("emojis/diamond.gif", pd3dDevice);
 *   // In render loop:
 *   LPDIRECT3DTEXTURE9 tex = gif.GetCurrentFrameTexture();
 */

#include <d3d9.h>
#include <vector>
#include <cstdint>
#include <windows.h>

 // Format enum for multi-format support
enum AnimatedTextureFormat
{
    ATF_UNKNOWN = 0,
    ATF_GIF,
    ATF_WEBP,
};

class AnimatedGifTexture
{
public:
    AnimatedGifTexture();
    ~AnimatedGifTexture();

    // Load GIF from raw memory buffer (already read from MZFile)
    bool CreateFromMemory(const uint8_t* pData, int nDataSize, LPDIRECT3DDEVICE9 pd3dDevice);

    // Destroy all textures and free memory
    void Destroy();

    // Get the texture for the current frame based on timeGetTime()
    LPDIRECT3DTEXTURE9 GetCurrentFrameTexture() const;

    // Get current frame index
    int GetCurrentFrame() const;

    // Frame count
    int GetFrameCount() const { return (int)m_Frames.size(); }

    // Canvas dimensions
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }

    // Is valid / loaded
    bool IsValid() const { return !m_Frames.empty(); }

    // Total animation duration in ms
    int GetTotalDurationMs() const { return m_nTotalDurationMs; }

    // D3D9 device lost / reset handling
    void OnLostDevice();
    void OnResetDevice();

private:
    struct Frame
    {
        LPDIRECT3DTEXTURE9 pTexture;
        int delayMs;          // how long this frame displays
        int cumulativeMs;     // cumulative time up to end of this frame
    };

    std::vector<Frame> m_Frames;
    int m_nWidth;
    int m_nHeight;
    int m_nTotalDurationMs;
    LPDIRECT3DDEVICE9 m_pd3dDevice;
    DWORD m_dwStartTime;      // timeGetTime() when animation started

    // Raw GIF data kept for device reset
    std::vector<uint8_t> m_RawGifData;

    bool CreateTexturesFromGifData();
    bool CreateTexturesFromWebPData();
    static AnimatedTextureFormat DetectFormat(const uint8_t* pData, int nSize);
};

#endif // ANIMATED_GIF_TEXTURE_H
