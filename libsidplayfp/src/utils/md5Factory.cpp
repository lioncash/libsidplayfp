/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2014-2015 Leandro Nini <drfiemost@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "utils/md5Factory.h"

#include "utils/iMd5.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef GCRYPT_WITH_MD5
#  include "utils/md5Gcrypt.h"
#else
#  include "utils/md5Internal.h"
#endif

namespace libsidplayfp
{

std::unique_ptr<iMd5> md5Factory::get()
{
#ifdef GCRYPT_WITH_MD5
    return std::make_unique<md5Gcrypt>()
#else
    return std::make_unique<md5Internal>();
#endif
}

}
