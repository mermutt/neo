/*
    droplet.h

    Copyright (C) 2021 Stewart Reive

    neo is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    neo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with neo. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef DROPLET_H
#define DROPLET_H

#include <chrono>
#include <cstdint>

using namespace std::chrono;

class Cloud;

// A Droplet is a single vertical character string
class Droplet {
public:
    Droplet();
    Droplet(Cloud* cl, uint16_t col, uint16_t endLine, uint16_t cpIdx,
            uint16_t len, float cps, uint64_t ttlMs, bool epochBool);

    void Reset();
    void Activate(uint64_t curTimeMs);
    void Advance(uint64_t curTimeMs);
    void Draw(uint64_t curTimeMs);
    void SyncCurLine();  // Sync curLine to putLine after Draw()

    // Getters/Setters/Convenience
    bool IsAlive() const { return _isAlive; }
    uint16_t GetCol() const { return _boundCol; }
    void SetCharsPerSec(float cps) { _charsPerSec = cps; }
    uint16_t GetHeadPutLine() const { return _headPutLine; }
    uint16_t GetTailPutLine() const { return _tailPutLine; }
    uint16_t GetCharPoolIdx() const { return _charPoolIdx; }
    bool GetEpochBool() const { return _epochBool; }
    void SetSimulationData(uint16_t dataOffset, uint16_t topFreezeLine) {
        _dataOffset = dataOffset;
        _topFreezeLine = topFreezeLine;
    }
    uint16_t GetDataOffset() const { return _dataOffset; }
    uint16_t GetTopFreezeLine() const { return _topFreezeLine; }
    void SetCloud(Cloud* cloud) { _pCloud = cloud; }

    enum class CharLoc { // describes where a char is within a Droplet
        MIDDLE,
        TAIL,
        HEAD
    };

private:
    Cloud* _pCloud; // Cloud keeps track of attributes/characters
    bool _isAlive; // Is this Droplet still displaying something?
    bool _isHeadCrawling; // Is the head (bottom) still moving?
    bool _isTailCrawling; // Is the tail (top) still moving?
    bool _epochBool; // Which epoch this droplet was created in
    uint16_t _boundCol; // Which screen column this droplet renders to
    uint16_t _headPutLine; // Where we are advancing the head
    uint16_t _headCurLine; // Where the head currently is
    uint16_t _topFreezeLine; // Where the upper char of droplet was at Epoch end
    uint16_t _tailPutLine; // Where we are advancing the tail
    uint16_t _tailCurLine; // The last empty line in this column
    uint16_t _endLine; // The head will not advance past this line
    uint16_t _charPoolIdx; // Index into the "charPool"
    uint16_t _length; // How many chars is this droplet?
    uint32_t _dataOffset; // Relative offset of top char on screen
    float _charsPerSec; // How many chars will be drawn per second
    uint64_t _lastTimeMs = 0; // Last time we drew something (milliseconds)
    uint64_t _headStopTimeMs = 0; // Time when head stopped (milliseconds)
    uint64_t _timeToLingerMs = 0; // How long the droplet is stationary before destruction (milliseconds)
    float _fractionalChars = 0.0f; // Accumulated fractional character movement

    bool IsHeadBright(uint64_t curTimeMs) const;
};

#endif
