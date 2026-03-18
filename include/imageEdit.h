#pragma once

#include <algorithm>
#include <vector>
#include <cstring>
#include <cmath>

template <typename T>
T clamp(T value, T min_val, T max_val)
{
    return std::max(min_val, std::min(value, max_val));
}

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

const int TARGET_SIZE = 16;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Loads any image, resizes it to 16x16, and returns the buffer.
// Caller is responsible for calling free() on the returned pointer.
static unsigned char *load_and_resize(const char *path, int *out_channels)
{
    int w, h, c;
    unsigned char *original = stbi_load(path, &w, &h, &c, 0);
    if (!original)
        return nullptr;

    *out_channels = c;
    unsigned char *resized = (unsigned char *)malloc(TARGET_SIZE * TARGET_SIZE * c);

    // Nearest-neighbour downsample — preserves hard edges and transparency
    for (int y = 0; y < TARGET_SIZE; ++y)
    {
        for (int x = 0; x < TARGET_SIZE; ++x)
        {
            int src_x = x * w / TARGET_SIZE;
            int src_y = y * h / TARGET_SIZE;

            const unsigned char *src = &original[(src_y * w + src_x) * c];
            unsigned char *dst = &resized[(y * TARGET_SIZE + x) * c];

            std::memcpy(dst, src, c);
        }
    }

    stbi_image_free(original);
    return resized;
}

static bool save(const char *path, unsigned char *data, int channels)
{
    return stbi_write_png(path, TARGET_SIZE, TARGET_SIZE, channels, data, TARGET_SIZE * channels) != 0;
}

// ── Filter Functions ──────────────────────────────────────────────────────────

bool apply_multiply(const char *input, const char *output, float r, float g, float b)
{
    int channels;
    unsigned char *data = load_and_resize(input, &channels);
    if (!data)
        return false;

    for (int i = 0; i < TARGET_SIZE * TARGET_SIZE; ++i)
    {
        unsigned char *p = &data[i * channels];

        if(channels == 4 && p[3] == 0) // Skip fully transparent pixels
            continue;

        p[0] = (unsigned char)clamp(p[0] * r, 0.0f, 255.0f);
        p[1] = (unsigned char)clamp(p[1] * g, 0.0f, 255.0f);
        p[2] = (unsigned char)clamp(p[2] * b, 0.0f, 255.0f);
    }

    bool ok = save(output, data, channels);
    free(data);
    return ok;
}

bool apply_grayscale(const char *input, const char *output)
{
    int channels;
    unsigned char *data = load_and_resize(input, &channels);
    if (!data)
        return false;

    for (int i = 0; i < TARGET_SIZE * TARGET_SIZE; ++i)
    {
        unsigned char *p = &data[i * channels];
        unsigned char gray = (unsigned char)(0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]);
        p[0] = p[1] = p[2] = gray;
    }

    bool ok = save(output, data, channels);
    free(data);
    return ok;
}

bool apply_invert(const char *input, const char *output)
{
    int channels;
    unsigned char *data = load_and_resize(input, &channels);
    if (!data)
        return false;

    for (int i = 0; i < TARGET_SIZE * TARGET_SIZE; ++i)
    {
        unsigned char *p = &data[i * channels];
        p[0] = 255 - p[0];
        p[1] = 255 - p[1];
        p[2] = 255 - p[2];
    }

    bool ok = save(output, data, channels);
    free(data);
    return ok;
}

// ── Composition ───────────────────────────────────────────────────────────────

// Composites overlay onto base using alpha blending, writes result to output.
// Formula: C_out = C_src * alpha + C_dst * (1 - alpha)
bool apply_overlay(const char *base_input, const char *overlay_input, const char *output)
{
    int base_c, over_c;
    unsigned char *base = load_and_resize(base_input, &base_c);
    unsigned char *overlay = load_and_resize(overlay_input, &over_c);

    if (!base || !overlay)
    {
        free(base);
        free(overlay);
        return false;
    }

    unsigned char *result = (unsigned char *)malloc(TARGET_SIZE * TARGET_SIZE * base_c);
    std::memcpy(result, base, TARGET_SIZE * TARGET_SIZE * base_c);

    for (int i = 0; i < TARGET_SIZE * TARGET_SIZE; ++i)
    {
        int b_idx = i * base_c;
        int o_idx = i * over_c;

        float alpha = (over_c == 4) ? (overlay[o_idx + 3] / 255.0f) : 1.0f;
        float inv_alpha = 1.0f - alpha;

        for (int c = 0; c < 3; ++c)
            result[b_idx + c] = (unsigned char)(overlay[o_idx + c] * alpha + result[b_idx + c] * inv_alpha);

        if (base_c == 4)
            result[b_idx + 3] = base[b_idx + 3];
    }

    bool ok = save(output, result, base_c);
    free(result);
    free(base);
    free(overlay);
    return ok;
}