#pragma once

struct ChunkCoord {
    int x, z;

    ChunkCoord(int _x=0, int _z=0) : x(_x), z(_z) {}

    bool operator==(const ChunkCoord& other) {
        return other.x == x && other.z == z;
    }

    bool operator!=(const ChunkCoord& other) {
        return !(*this == other);
    }

    long long getHash() const {
        return ((long long)(x) << 32) | (z & 0xffffffff);
    }

    static ChunkCoord fromHash(long long hash) {
        int x = (int)(hash >> 32);
        int z = (int)(hash & 0xffffffff);
        return ChunkCoord(x, z);
    }
};