/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright 2011-2017 Leandro Nini
 *  Copyright 2007-2010 Antti Lankila
 *  Copyright 2000 Simon White
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sidplayfp/SidTuneInfo.h>

uint_least16_t SidTuneInfo::loadAddr() const { return getLoadAddr(); }

uint_least16_t SidTuneInfo::initAddr() const { return getInitAddr(); }

uint_least16_t SidTuneInfo::playAddr() const { return getPlayAddr(); }

unsigned int SidTuneInfo::songs() const { return getSongs(); }

unsigned int SidTuneInfo::startSong() const { return getStartSong(); }

unsigned int SidTuneInfo::currentSong() const { return getCurrentSong(); }

uint_least16_t SidTuneInfo::sidChipBase(std::size_t i) const { return getSidChipBase(i); }

std::size_t SidTuneInfo::sidChips() const { return getSidChips(); }

int SidTuneInfo::songSpeed() const { return getSongSpeed(); }

uint_least8_t SidTuneInfo::relocStartPage() const { return getRelocStartPage(); }

uint_least8_t SidTuneInfo::relocPages() const { return getRelocPages(); }

SidTuneInfo::Model SidTuneInfo::sidModel(std::size_t i) const { return getSidModel(i); }

SidTuneInfo::Compatibility SidTuneInfo::compatibility() const { return getCompatibility(); }

std::size_t SidTuneInfo::numberOfInfoStrings() const { return getNumberOfInfoStrings(); }
const char* SidTuneInfo::infoString(std::size_t i) const { return getInfoString(i); }


std::size_t SidTuneInfo::numberOfCommentStrings() const{ return getNumberOfCommentStrings(); }
const char* SidTuneInfo::commentString(std::size_t i) const{ return getCommentString(i); }

uint_least32_t SidTuneInfo::dataFileLen() const { return getDataFileLen(); }

uint_least32_t SidTuneInfo::c64dataLen() const { return getC64dataLen(); }

SidTuneInfo::Clock SidTuneInfo::clockSpeed() const { return getClockSpeed(); }

const char* SidTuneInfo::formatString() const { return getFormatString(); }

bool SidTuneInfo::fixLoad() const { return getFixLoad(); }

const char* SidTuneInfo::path() const { return getPath(); }

const char* SidTuneInfo::dataFileName() const { return getDataFileName(); }

const char* SidTuneInfo::infoFileName() const { return getInfoFileName(); }
