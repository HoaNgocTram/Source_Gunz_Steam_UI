/**
 * gif_load.h — Lightweight single-header GIF decoder
 * 
 * Decodes GIF87a/GIF89a files into RGBA pixel arrays with per-frame delays.
 * No external dependencies. Suitable for embedding in game engines.
 *
 * Usage:
 *   GifFileData gif;
 *   if (GifLoad(&gif, fileBuffer, fileSize)) {
 *       // gif.width, gif.height — canvas size
 *       // gif.frameCount — number of frames
 *       // gif.frames[i].pixels — RGBA data (width * height * 4 bytes)
 *       // gif.frames[i].delayMs — display duration in milliseconds
 *       GifFree(&gif);
 *   }
 *
 * Supports: transparency, interlacing, local/global color tables,
 *           disposal methods (0-3), frame delays.
 *
 * Based on GIF89a specification.
 */

#pragma once
#ifndef GIF_LOAD_H
#define GIF_LOAD_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

struct GifFrame
{
    uint8_t* pixels;    // RGBA, size = width * height * 4
    int delayMs;        // display time in milliseconds (0 = use default ~100ms)
};

struct GifFileData
{
    int width;
    int height;
    int frameCount;
    GifFrame* frames;
};

// Forward declarations
static bool GifLoad(GifFileData* out, const uint8_t* data, int dataSize);
static void GifFree(GifFileData* gif);

// ============================================================
//  Implementation
// ============================================================

namespace gif_detail
{

struct Reader
{
    const uint8_t* data;
    int size;
    int pos;

    uint8_t ReadByte()
    {
        if (pos >= size) return 0;
        return data[pos++];
    }

    uint16_t ReadU16()
    {
        uint8_t lo = ReadByte();
        uint8_t hi = ReadByte();
        return (uint16_t)(lo | (hi << 8));
    }

    void ReadBytes(uint8_t* dst, int count)
    {
        for (int i = 0; i < count; i++)
            dst[i] = ReadByte();
    }

    void Skip(int count)
    {
        pos += count;
        if (pos > size) pos = size;
    }
};

struct ColorTable
{
    uint8_t colors[256][3];
    int count;
};

struct GCE // Graphic Control Extension
{
    int disposalMethod; // 0=none, 1=leave, 2=bg, 3=restore
    bool hasTransparency;
    int transparentIndex;
    int delayCs; // delay in centiseconds
};

// LZW decompression
struct LZWDecoder
{
    static const int MAX_BITS = 12;
    static const int MAX_CODE = (1 << MAX_BITS);

    struct Entry
    {
        int16_t prefix;
        uint8_t suffix;
        int16_t length;
    };

    Entry table[MAX_CODE];
    int codeSize;
    int clearCode;
    int endCode;
    int nextCode;
    int curCodeSize;

    // Bit reader state
    Reader* reader;
    int blockRemain;
    uint32_t bitBuf;
    int bitCount;

    void Init(Reader* r, int minCodeSize)
    {
        reader = r;
        codeSize = minCodeSize;
        clearCode = 1 << codeSize;
        endCode = clearCode + 1;
        bitBuf = 0;
        bitCount = 0;
        blockRemain = 0;
        ResetTable();
    }

    void ResetTable()
    {
        nextCode = endCode + 1;
        curCodeSize = codeSize + 1;
        for (int i = 0; i < clearCode; i++)
        {
            table[i].prefix = -1;
            table[i].suffix = (uint8_t)i;
            table[i].length = 1;
        }
    }

    int ReadCode()
    {
        while (bitCount < curCodeSize)
        {
            if (blockRemain == 0)
            {
                blockRemain = reader->ReadByte();
                if (blockRemain == 0) return endCode;
            }
            bitBuf |= ((uint32_t)reader->ReadByte()) << bitCount;
            bitCount += 8;
            blockRemain--;
        }
        int code = bitBuf & ((1 << curCodeSize) - 1);
        bitBuf >>= curCodeSize;
        bitCount -= curCodeSize;
        return code;
    }

    // Output a code's pixel string into buffer, return count
    int OutputCode(int code, uint8_t* output, int maxLen)
    {
        if (code < 0 || code >= MAX_CODE) return 0;
        int len = (code < nextCode) ? table[code].length : 0;
        if (len == 0 || len > maxLen) return 0;

        int i = len - 1;
        int c = code;
        while (c >= 0 && i >= 0)
        {
            output[i--] = table[c].suffix;
            c = table[c].prefix;
        }
        return len;
    }

    void AddEntry(int prefix, uint8_t suffix)
    {
        if (nextCode < MAX_CODE)
        {
            table[nextCode].prefix = (int16_t)prefix;
            table[nextCode].suffix = suffix;
            table[nextCode].length = (prefix >= 0 ? table[prefix].length : 0) + 1;
            nextCode++;
            if (nextCode >= (1 << curCodeSize) && curCodeSize < MAX_BITS)
                curCodeSize++;
        }
    }

    // Skip remaining sub-blocks
    void SkipRemainder()
    {
        // Skip any remaining bits in current block
        while (blockRemain > 0)
        {
            reader->ReadByte();
            blockRemain--;
        }
        // Skip any additional sub-blocks
        int bs;
        do {
            bs = reader->ReadByte();
            reader->Skip(bs);
        } while (bs > 0 && reader->pos < reader->size);
    }

    int Decompress(uint8_t* output, int maxPixels)
    {
        int pixelsWritten = 0;
        int prevCode = -1;
        uint8_t tempBuf[MAX_CODE + 1];

        while (pixelsWritten < maxPixels)
        {
            int code = ReadCode();
            if (code == endCode) break;

            if (code == clearCode)
            {
                ResetTable();
                prevCode = -1;
                continue;
            }

            if (code < nextCode)
            {
                // Code exists in table
                int len = OutputCode(code, tempBuf, MAX_CODE);
                if (len == 0) break;
                int toCopy = (pixelsWritten + len > maxPixels) ? (maxPixels - pixelsWritten) : len;
                memcpy(output + pixelsWritten, tempBuf, toCopy);
                pixelsWritten += toCopy;

                if (prevCode >= 0 && nextCode < MAX_CODE)
                    AddEntry(prevCode, tempBuf[0]);
            }
            else if (code == nextCode && prevCode >= 0)
            {
                // Special case: code not yet in table
                int len = OutputCode(prevCode, tempBuf, MAX_CODE - 1);
                if (len == 0) break;
                tempBuf[len] = tempBuf[0];
                len++;
                int toCopy = (pixelsWritten + len > maxPixels) ? (maxPixels - pixelsWritten) : len;
                memcpy(output + pixelsWritten, tempBuf, toCopy);
                pixelsWritten += toCopy;

                if (nextCode < MAX_CODE)
                    AddEntry(prevCode, tempBuf[0]);
            }
            else
            {
                break; // Invalid code
            }

            prevCode = code;
        }

        SkipRemainder();
        return pixelsWritten;
    }
};

static const int INTERLACE_OFFSETS[] = { 0, 4, 2, 1 };
static const int INTERLACE_STEPS[] = { 8, 8, 4, 2 };

} // namespace gif_detail

static bool GifLoad(GifFileData* out, const uint8_t* data, int dataSize)
{
    if (!out || !data || dataSize < 13) return false;

    memset(out, 0, sizeof(GifFileData));

    gif_detail::Reader r;
    r.data = data;
    r.size = dataSize;
    r.pos = 0;

    // Header: "GIF87a" or "GIF89a"
    uint8_t header[6];
    r.ReadBytes(header, 6);
    if (memcmp(header, "GIF", 3) != 0) return false;

    // Logical Screen Descriptor
    int canvasW = r.ReadU16();
    int canvasH = r.ReadU16();
    uint8_t packed = r.ReadByte();
    r.ReadByte(); // bg color index
    r.ReadByte(); // pixel aspect ratio

    bool hasGCT = (packed & 0x80) != 0;
    int gctSize = 1 << ((packed & 0x07) + 1);

    // Global Color Table
    gif_detail::ColorTable gct;
    gct.count = 0;
    if (hasGCT)
    {
        gct.count = gctSize;
        for (int i = 0; i < gctSize; i++)
            r.ReadBytes(gct.colors[i], 3);
    }

    out->width = canvasW;
    out->height = canvasH;

    // Temporary storage
    std::vector<GifFrame> frames;

    // Canvas buffers
    int canvasBytes = canvasW * canvasH * 4;
    uint8_t* canvas = (uint8_t*)calloc(1, canvasBytes);     // current composited frame
    uint8_t* prevCanvas = (uint8_t*)calloc(1, canvasBytes);  // for disposal method 3
    if (!canvas || !prevCanvas)
    {
        free(canvas);
        free(prevCanvas);
        return false;
    }

    gif_detail::GCE gce;
    gce.disposalMethod = 0;
    gce.hasTransparency = false;
    gce.transparentIndex = -1;
    gce.delayCs = 0;

    bool gceValid = false;

    // Parse blocks
    while (r.pos < r.size)
    {
        uint8_t blockType = r.ReadByte();

        if (blockType == 0x3B) // Trailer
            break;

        if (blockType == 0x21) // Extension
        {
            uint8_t extLabel = r.ReadByte();

            if (extLabel == 0xF9) // Graphic Control Extension
            {
                r.ReadByte(); // block size (always 4)
                uint8_t gcPacked = r.ReadByte();
                gce.disposalMethod = (gcPacked >> 2) & 0x07;
                gce.hasTransparency = (gcPacked & 0x01) != 0;
                gce.delayCs = r.ReadU16();
                gce.transparentIndex = r.ReadByte();
                r.ReadByte(); // block terminator
                gceValid = true;
            }
            else
            {
                // Skip other extensions (comment, application, plain text)
                int bs;
                do {
                    bs = r.ReadByte();
                    r.Skip(bs);
                } while (bs > 0 && r.pos < r.size);
            }
            continue;
        }

        if (blockType == 0x2C) // Image Descriptor
        {
            int imgLeft = r.ReadU16();
            int imgTop = r.ReadU16();
            int imgW = r.ReadU16();
            int imgH = r.ReadU16();
            uint8_t imgPacked = r.ReadByte();

            bool hasLCT = (imgPacked & 0x80) != 0;
            bool interlaced = (imgPacked & 0x40) != 0;
            int lctSize = hasLCT ? (1 << ((imgPacked & 0x07) + 1)) : 0;

            gif_detail::ColorTable lct;
            lct.count = 0;
            if (hasLCT)
            {
                lct.count = lctSize;
                for (int i = 0; i < lctSize; i++)
                    r.ReadBytes(lct.colors[i], 3);
            }

            gif_detail::ColorTable* ct = hasLCT ? &lct : &gct;

            // Handle disposal BEFORE compositing new frame
            int disposal = gceValid ? gce.disposalMethod : 0;

            // Save canvas for disposal method 3 (restore to previous)
            if (disposal == 3)
                memcpy(prevCanvas, canvas, canvasBytes);

            // LZW decode
            int minCodeSize = r.ReadByte();
            if (minCodeSize < 2 || minCodeSize > 11)
            {
                // Skip bad frame
                int bs;
                do {
                    bs = r.ReadByte();
                    r.Skip(bs);
                } while (bs > 0 && r.pos < r.size);
                gceValid = false;
                continue;
            }

            int pixelCount = imgW * imgH;
            uint8_t* indices = (uint8_t*)malloc(pixelCount);
            if (!indices) break;

            gif_detail::LZWDecoder lzw;
            lzw.Init(&r, minCodeSize);
            int decoded = lzw.Decompress(indices, pixelCount);

            // Composite onto canvas
            for (int p = 0; p < decoded; p++)
            {
                int x, y;
                if (interlaced)
                {
                    // De-interlace
                    int row = 0;
                    int remaining = p / imgW;
                    for (int pass = 0; pass < 4; pass++)
                    {
                        int passRows = 0;
                        for (int r2 = gif_detail::INTERLACE_OFFSETS[pass]; r2 < imgH; r2 += gif_detail::INTERLACE_STEPS[pass])
                            passRows++;
                        if (remaining < passRows)
                        {
                            row = gif_detail::INTERLACE_OFFSETS[pass] + remaining * gif_detail::INTERLACE_STEPS[pass];
                            break;
                        }
                        remaining -= passRows;
                    }
                    x = (p % imgW) + imgLeft;
                    y = row + imgTop;
                }
                else
                {
                    x = (p % imgW) + imgLeft;
                    y = (p / imgW) + imgTop;
                }

                if (x < 0 || x >= canvasW || y < 0 || y >= canvasH) continue;

                uint8_t idx = indices[p];

                // Skip transparent pixels
                if (gceValid && gce.hasTransparency && idx == gce.transparentIndex)
                    continue;

                if (idx < ct->count)
                {
                    int off = (y * canvasW + x) * 4;
                    canvas[off + 0] = ct->colors[idx][0]; // R
                    canvas[off + 1] = ct->colors[idx][1]; // G
                    canvas[off + 2] = ct->colors[idx][2]; // B
                    canvas[off + 3] = 255;                 // A
                }
            }

            free(indices);

            // Create frame from current canvas state
            GifFrame frame;
            frame.pixels = (uint8_t*)malloc(canvasBytes);
            if (!frame.pixels) break;
            memcpy(frame.pixels, canvas, canvasBytes);

            // Delay: GIF uses centiseconds, we want milliseconds
            frame.delayMs = gceValid ? (gce.delayCs * 10) : 100;
            if (frame.delayMs <= 0) frame.delayMs = 100; // Default 100ms

            frames.push_back(frame);

            // Apply disposal after saving the frame
            if (disposal == 2)
            {
                // Restore to background: clear the sub-image area
                for (int py = imgTop; py < imgTop + imgH && py < canvasH; py++)
                {
                    for (int px = imgLeft; px < imgLeft + imgW && px < canvasW; px++)
                    {
                        int off = (py * canvasW + px) * 4;
                        canvas[off + 0] = 0;
                        canvas[off + 1] = 0;
                        canvas[off + 2] = 0;
                        canvas[off + 3] = 0;
                    }
                }
            }
            else if (disposal == 3)
            {
                // Restore to previous
                memcpy(canvas, prevCanvas, canvasBytes);
            }
            // disposal 0 or 1: leave canvas as-is

            gceValid = false;
            continue;
        }

        // Unknown block — skip
        break;
    }

    free(canvas);
    free(prevCanvas);

    if (frames.empty())
        return false;

    // Copy to output
    out->frameCount = (int)frames.size();
    out->frames = (GifFrame*)malloc(sizeof(GifFrame) * out->frameCount);
    if (!out->frames)
    {
        for (auto& f : frames) free(f.pixels);
        return false;
    }
    memcpy(out->frames, frames.data(), sizeof(GifFrame) * out->frameCount);

    return true;
}

static void GifFree(GifFileData* gif)
{
    if (!gif) return;
    if (gif->frames)
    {
        for (int i = 0; i < gif->frameCount; i++)
            free(gif->frames[i].pixels);
        free(gif->frames);
    }
    memset(gif, 0, sizeof(GifFileData));
}

#endif // GIF_LOAD_H
