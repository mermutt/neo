/*
    droplet.cpp - Implements the Droplet class

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

#include "droplet.h"
#include "cloud.h"

#include "log.h"
extern DebugLog debug;

Droplet::Droplet() {
    Reset();
}

Droplet::Droplet(Cloud* cl, uint16_t col, uint16_t endLine, uint16_t cpIdx,
                 uint16_t len, float cps, uint64_t ttlMs, bool epochBool) {
    Reset();
    _pCloud = cl;
    _boundCol = col;
    _endLine = endLine;
    _charPoolIdx = cpIdx;
    _length = len;
    _charsPerSec = cps;
    _timeToLingerMs = ttlMs;
    _fractionalChars = 0.0f;
    _epochBool = epochBool;
}

void Droplet::Reset() {
    _pCloud = nullptr;
    _isAlive = false;
    _isHeadCrawling = false;
    _isTailCrawling = false;
    _boundCol = 0xFFFF;
    _headPutLine = 0;
    _headCurLine = 0;
    _tailPutLine = 0xFFFF;
    _tailCurLine = 0;
    _endLine = 0xFFFF;
    _charPoolIdx = 0xFFFF;
    _length = 0xFFFF;
    _charsPerSec = 0.0f;
    _lastTimeMs = 0;
    _headStopTimeMs = 0;
    _timeToLingerMs = 0;
    _fractionalChars = 0.0f;
    _epochBool = false;
    _dataOffset = 0xFFFFFFFF;
    _topFreezeLine = 0xFFFF;
}

void Droplet::Activate(uint64_t curTimeMs) {
    _isAlive = true;
    _isHeadCrawling = true;
    _isTailCrawling = true;
    _lastTimeMs = curTimeMs;
}

void Droplet::Advance(uint64_t curTimeMs) {
    // Calculate elapsed time in milliseconds
    uint64_t elapsedMs = 0;
    if (curTimeMs > _lastTimeMs) {
        elapsedMs = curTimeMs - _lastTimeMs;
    }
    const float elapsedSec = static_cast<float>(elapsedMs) / 1000.0f;
    _fractionalChars += _charsPerSec * elapsedSec;
    uint16_t charsAdvanced = static_cast<uint16_t>(_fractionalChars);
    if (!charsAdvanced)
        return;
    _fractionalChars -= charsAdvanced;

    // Save old curLine values for threshold detection and Draw() optimization
    uint16_t oldTailCurLine = _tailCurLine;
    uint16_t oldHeadCurLine = _headCurLine;

    // Advance the head
    if (_isHeadCrawling) {
        _headPutLine += charsAdvanced;
        _headPutLine = min(_headPutLine, _endLine);

        // If head reaches the _endLine, stop the head and maybe the tail too
        if (_headPutLine == _endLine) {
            _isHeadCrawling = false;
            if (_headStopTimeMs == 0) {
                _headStopTimeMs = curTimeMs;
                if (_timeToLingerMs > 0) {
                    _isTailCrawling = false;
                }
            }
        }
    }

    // Advance the tail
    if (_isTailCrawling && (_headPutLine >= _length || _headPutLine >= _endLine)) {
        if (_tailPutLine != 0xFFFF) {
            _tailPutLine += charsAdvanced;
        } else {
            _tailPutLine = charsAdvanced;
        }
        _tailPutLine = min(_tailPutLine, _endLine);

        // If the tail advances far enough down the screen, allow other droplets to spawn
        const uint16_t threshLine = _pCloud->GetLines() / 4;
        if (oldTailCurLine <= threshLine && _tailPutLine > threshLine)
            _pCloud->SetColumnSpawn(_boundCol, true);
    }

    // Restart the tail after lingering
    if (!_isTailCrawling && _headStopTimeMs > 0 && curTimeMs >= _headStopTimeMs + _timeToLingerMs) {
        _isTailCrawling = true;
    }
    // Once tail reaches the head, kill this droplet
    if (_tailPutLine == _headPutLine) {
        _isAlive = false;
    }
    _lastTimeMs = curTimeMs; // Required or else nothing will ever get drawn...
}

void Droplet::SyncCurLine() {
    // Sync curLine to putLine - called after Draw() for deterministic behavior
    _headCurLine = _headPutLine;
    // Only sync tail curLine if tail has started moving (putLine is valid)
    // This ensures Draw() clearing loop works correctly
    if (_tailPutLine != 0xFFFF)
        _tailCurLine = _tailPutLine;
}

void Droplet::Draw(uint64_t curTimeMs) {
    uint16_t startLine = 0;
    if (_tailPutLine != 0xFFFF) {
        // Delete the very end of tail
        for (uint16_t line = _tailCurLine; line <= _tailPutLine; line++) {
            mvaddch(line, _boundCol, ' ');
        }
        // Note: _tailCurLine is now updated in Advance() for deterministic behavior
        startLine = _tailPutLine + 1;
    }
    // if (_boundCol == 0) {
    //     debug.log("  _boundCol: " + std::to_string(_boundCol));
    //     debug.log("    startLine: " + std::to_string(startLine) + ", _headPutLine: " + std::to_string(_headPutLine) + ", _dataOffset: " + std::to_string(_dataOffset) + ", _topFreezeLine: " +  std::to_string(_topFreezeLine));
    // }

    for (uint16_t line = startLine; line <= _headPutLine; line++) {

        uint16_t offset = line >= _topFreezeLine ? _dataOffset + line - _topFreezeLine : UINT16_MAX;
        const wchar_t val = _pCloud->GetChar(line, _charPoolIdx, offset);

        CharLoc cl = CharLoc::MIDDLE;
        if (_tailPutLine != 0xFFFF && line == _tailPutLine + 1)
            cl = CharLoc::TAIL;
        if (line == _headPutLine && IsHeadBright(curTimeMs))
            cl = CharLoc::HEAD;

        // No need to draw chars between tail and _headCurLine
        if (cl == CharLoc::MIDDLE && line < _headCurLine && line != _endLine &&
            _pCloud->GetShadingMode() != Cloud::ShadingMode::DISTANCE_FROM_HEAD)
            continue;

        Cloud::CharAttr attr;
        _pCloud->GetAttr(line, _boundCol, val, cl, &attr, curTimeMs, _headPutLine, _length);

        // if (_boundCol == 114) {
        //     debug.log("  _boundCol: " + std::to_string(_boundCol) +
        //               "  offset: " + std::to_string(offset) +
        //               "  line: " + std::to_string(line) +
        //               "  val: " + std::string(1, val)
        //               );
        // }

        attr_t attr2 = attr.isBold ? A_BOLD : A_NORMAL;
        cchar_t wc = {};
        wc.attr = attr2;
        wc.chars[0] = val;

        if (_pCloud->GetColorMode() != ColorMode::MONO) {
            attron(COLOR_PAIR(attr.colorPair));
            mvadd_wch(line, _boundCol, &wc);
            attroff(COLOR_PAIR(attr.colorPair));
        } else {
            mvadd_wch(line, _boundCol, &wc);
        }
    }
    // Note: _headCurLine is now updated in Advance() for deterministic behavior
}

bool Droplet::IsHeadBright(uint64_t curTimeMs) const {
    if (_isHeadCrawling)
        return true;
    else if (_headStopTimeMs > 0 && curTimeMs <= _headStopTimeMs + 100)
        return true;

    return false;
}


