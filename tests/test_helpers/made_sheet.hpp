#ifndef SKIP_OPENGL_TESTS
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "rendering/texture2d.hpp"

namespace made_sheet
{
    inline std::uint32_t crcOf(const std::vector<unsigned char> &bytes)
    {
        std::uint32_t crc = 0xFFFFFFFF;
        for (unsigned char byte : bytes)
        {
            crc ^= byte;
            for (int bit = 0; bit < 8; ++bit)
                crc = (crc >> 1) ^ (0xEDB88320 & (~(crc & 1) + 1));
        }

        return crc ^ 0xFFFFFFFF;
    }

    inline std::uint32_t adlerOf(const std::vector<unsigned char> &bytes)
    {
        std::uint32_t low = 1, high = 0;
        for (unsigned char byte : bytes)
        {
            low = (low + byte) % 65521;
            high = (high + low) % 65521;
        }

        return (high << 16) | low;
    }

    inline void beInt(std::vector<unsigned char> &into, std::uint32_t value)
    {
        for (int shift = 24; shift >= 0; shift -= 8)
            into.push_back(static_cast<unsigned char>((value >> shift) & 0xFF));
    }

    inline void chunk(
        std::vector<unsigned char> &into,
        const char *kind,
        const std::vector<unsigned char> &body)
    {
        beInt(into, static_cast<std::uint32_t>(body.size()));

        std::vector<unsigned char> named(kind, kind + 4);
        named.insert(named.end(), body.begin(), body.end());
        into.insert(into.end(), named.begin(), named.end());
        beInt(into, crcOf(named));
    }

    inline std::vector<unsigned char> stored(const std::vector<unsigned char> &raw)
    {
        std::vector<unsigned char> out{0x78, 0x01};
        for (std::size_t at = 0; at < raw.size(); at += 65535)
        {
            auto length = static_cast<std::uint16_t>(std::min<std::size_t>(65535, raw.size() - at));
            out.push_back(at + length >= raw.size() ? 1 : 0);
            out.push_back(static_cast<unsigned char>(length & 0xFF));
            out.push_back(static_cast<unsigned char>(length >> 8));
            out.push_back(static_cast<unsigned char>(~length & 0xFF));
            out.push_back(static_cast<unsigned char>((~length >> 8) & 0xFF));
            out.insert(
                out.end(),
                raw.begin() + static_cast<std::ptrdiff_t>(at),
                raw.begin() + static_cast<std::ptrdiff_t>(at + length));
        }

        beInt(out, adlerOf(raw));
        return out;
    }
}

inline std::string aSheetFileOf(int across, int down, int cellSize = 16)
{
    int width = across * cellSize, height = down * cellSize;

    std::vector<unsigned char> raw;
    for (int y = 0; y < height; ++y)
    {
        raw.push_back(0);
        for (int x = 0; x < width; ++x)
        {
            auto cell = static_cast<unsigned char>((y / cellSize) * across + x / cellSize);
            bool inside = x % cellSize > 2 && y % cellSize > 2;

            raw.push_back(static_cast<unsigned char>(cell * 20));
            raw.push_back(static_cast<unsigned char>(255 - cell * 20));
            raw.push_back(128);
            raw.push_back(inside ? 255 : 0);
        }
    }

    std::vector<unsigned char> header;
    made_sheet::beInt(header, static_cast<std::uint32_t>(width));
    made_sheet::beInt(header, static_cast<std::uint32_t>(height));
    header.insert(header.end(), {8, 6, 0, 0, 0});

    std::vector<unsigned char> png{0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    made_sheet::chunk(png, "IHDR", header);
    made_sheet::chunk(png, "IDAT", made_sheet::stored(raw));
    made_sheet::chunk(png, "IEND", {});

    std::filesystem::path path = std::filesystem::temp_directory_path() /
                                 ("platformer_sheet_" + std::to_string(across) + "x" +
                                  std::to_string(down) + "x" + std::to_string(cellSize) + ".png");

    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char *>(png.data()), static_cast<std::streamsize>(png.size()));

    return path.string();
}

inline Texture2D aSheetOf(int across, int down, int cellSize = 16)
{
    return Texture2D(aSheetFileOf(across, down, cellSize));
}
#endif
